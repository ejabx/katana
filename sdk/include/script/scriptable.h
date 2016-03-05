/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scriptable.h
	Author:		Eric Bryant

	Scripting interface. Any object which exposed itself to the scripting
	environment must have a proxy which supports this interface. It provdies
	the glue between the scripting engine and the C++ code.

	TODO:	There should be two kinds of scriptable classes, one that's aggregated,
			and another that doesn't require an aggregate. An example would be
			inputproxy (non-aggregated) versus visnodeproxy (aggregate). As a result,
			non-aggregated proxies need to derive from RefCount for compilation,
			when in fact it should derive from another scriptable class.
*/

#ifndef _SCRIPTABLE_H
#define _SCRIPTABLE_H

namespace Katana
{

///
/// Scriptable
///
template <typename T, typename AGGREGATE = T>
class Scriptable : public RTTI
{
public:

	/// Stores the reference to an existing aggregate object
	void setRef( AGGREGATE * aggregate )					{ m_ref = aggregate; }

	/// Returns an autoptr to the internal reference
	AutoPtr<AGGREGATE> getRef()								{ return m_ref; }

private:

	/// Type definitions
	typedef int (*ScriptGlobal)(lua_State*);
	typedef int (T::*ScriptMethod)(lua_State*);
	typedef std::map<const char *, ScriptMethod> ScriptMethodMap;
	typedef boost::function2< int, T*, lua_State* > ScriptFunctor;

protected:
	/// Initializes the scripting interface to this object. Call during this object's
	/// OnRegister() event. This will also iterate through the methods and register them.
	static void InitScriptObject( ScriptEnv env );

	/// Unbinds the scripting object with the script environment
	static void UnlinkScriptObject( ScriptEnv env );

protected:
	/// Register a global method under this class (using ":")
	static void addGlobal( const char * szMethod, ScriptGlobal funct );

	/// Register a method under this class (using ".")
	static void addMethod( const char * szMethod, ScriptMethod funct );

	/// Register a setter function under this class (using "=")
	static void addSetter( const char * szSetter, ScriptMethod funct );

private:

	/// Private scripting helper functions
	static int gettable( lua_State* lua );
	static int settable( lua_State* lua );
	static int gc( lua_State* lua );
	static int thunk( lua_State* lua );
	static int getnamespacetable( lua_State * lua );
	static int getmethodtable( lua_State * _L );
	static int getmethodtable( lua_State * _L, const RTTI_TYPE * rtti );
	static int getsettertable( lua_State * _L );

protected:
	/// The internal reference to the corresponding C++ object which
	/// this scripting object corresponding to. For example, if this was
	/// VisibleProxy, this would be a reference to the internal Visible
	/// object.
	AutoPtr<AGGREGATE>	m_ref;

	/// This tag identifies the script object's interface to the scripting environment
	static int	m_tag;

	/// The scripting environment
	static ScriptEnv m_env;
};


/// Place this in the class declaration to define a scriptable object
#define KDECLARE_SCRIPT								\
public:												\
	static int OnRegister( lua_State * lua );		\
	static int OnUnregister( lua_State * lua );		\

/// Place this in the source of the scriptable object
#define KIMPLEMENT_SCRIPT( class )						\
	int class::m_tag = 0;								\
	ScriptEnv class::m_env = 0;							\
	

/// Use this macro to define a global namespace method for the class
/// (it doesn't need an instance to execute, like a static class function)
#define KDECLARE_SCRIPT_GLOBAL( funct ) \
		static int __##funct( lua_State * lua ) \

/// Use this macro to define a scriptable method in your class
#define KDECLARE_SCRIPT_METHOD( funct ) \
		int __##funct( lua_State * lua ) \

/// Use this macro to define a scriptable property in your class
#define KDECLARE_SCRIPT_PROPERTY( funct ) \
		int __##funct( lua_State * lua ) \

/// Use this macro to implement a scriptable global method
#define KIMPLEMENT_SCRIPT_GLOBAL( class, funct ) \
		int class::__##funct( lua_State * lua )  \

/// Use this macro to implement a scriptable property
#define KIMPLEMENT_SCRIPT_PROPERTY( class, funct ) \
		int class::__##funct( lua_State * lua )  \

/// Use this macro to implement a scriptable method
#define KIMPLEMENT_SCRIPT_METHOD( class, funct ) \
		int class::__##funct( lua_State * lua )  \




//
// InitScriptObject
//
template <typename T, typename AGGREGATE>
void Scriptable<T,AGGREGATE>::InitScriptObject( ScriptEnv env )
{
	// Store the scripting environment
	m_env = env;

	if ( m_tag == 0 )
	{
		lua_State * _L = static_cast<lua_State *>(env);

		// Create the metatable
		lua_newtable( _L );
		int metatable = lua_gettop( _L );

		// Setup gc and add to metatable
		lua_pushliteral( _L, "__gc" );
		lua_pushcclosure( _L, gc, 0 );		// gc, bound with nothing
		lua_settable( _L, metatable );

		// Setup gettable and add to metatable
		lua_pushliteral( _L, "__gettable" );
		lua_pushcclosure( _L, gettable, 0 );
		lua_settable( _L, metatable );							// put __gettable in metatable

		// Setup settable and add to metatable
		lua_pushliteral( _L, "__settable" );
		lua_pushcclosure( _L, settable, 0 );
		lua_settable(_L, metatable );							// put __settable in metatable

		// Put a new "methods" table in our metatable
		lua_pushliteral( _L, "methods" );
		lua_newtable( _L );
		lua_settable( _L, metatable );

		// Put a new "setters" table in our metatable
		lua_pushliteral( _L, "setters" );
		lua_newtable( _L );
		lua_settable( _L, metatable );

		// Put the classtable in the registry, referenced by RTTI
		lua_pushlightuserdata( _L, const_cast<RTTI_TYPE*>( &AGGREGATE::ms_kRTTI ) );	// aggregates's RTTI is the name of our class table
		lua_pushvalue( _L, metatable );
		lua_settable( _L, LUA_REGISTRYINDEX );

		// Create a namespace table
		// The namespace is stored in the LUA_GLOBALSINDEX so that there can be static methods and members
		lua_pushstring( _L, AGGREGATE::ms_kRTTI.GetName() );
		lua_newtable( _L );
		lua_settable( _L, LUA_GLOBALSINDEX );

		// Unwind the stack to where it started at
		lua_pop( _L, 1 );

		// Set the internal tag to the metatable
		m_tag = metatable;

		// Log
		KLOG2("Register \"%s\"", T::ms_kRTTI.GetName() );
	}
}

//
// UnlinkScriptObject
//
template <typename T, typename AGGREGATE>
void Scriptable<T,AGGREGATE>::UnlinkScriptObject( ScriptEnv env )
{
	lua_State * _L = static_cast<lua_State *>(m_env);

	// Set our proxy's table to nil
	// It and its subtables will be garbage collected by Lua
	// Proxies will assert if the class table cannot be found
	lua_pushlightuserdata( _L, const_cast<RTTI_TYPE*>( &AGGREGATE::ms_kRTTI ) );
	lua_pushnil( _L );
	lua_settable( _L, LUA_REGISTRYINDEX );

	// Nil out the namespace's table
	lua_pushstring( _L, AGGREGATE::ms_kRTTI.GetName() );
	lua_pushnil( _L );
	lua_settable( _L, LUA_GLOBALSINDEX );
}

//
// addGlobal
//
template <typename T, typename AGGREGATE>
void Scriptable<T,AGGREGATE>::addGlobal( const char * szMethod, ScriptGlobal funct )
{
	lua_State * _L = static_cast<lua_State *>(m_env);
	getnamespacetable( _L );

	lua_pushstring( _L, szMethod );
	if ( !funct )
		lua_pushnil( _L );	// NULL pfn means clear out the value
	else
		lua_pushcfunction( _L, static_cast<lua_CFunction>(funct) );

	lua_settable( _L, -3 );
	lua_pop( _L, 1 );	// remove nNamespace
}

//
// addMethod
//
template <typename T, typename AGGREGATE>
void Scriptable<T,AGGREGATE>::addMethod( const char * szMethod, ScriptMethod funct )
{
	lua_State * _L = static_cast<lua_State *>(m_env);
	int methods = getmethodtable( _L );

	// Convert the szLuaName to a lowercase std::string
	std::string key(szMethod);
	std::transform( key.begin(), key.end(), key.begin(), tolower );
	lua_pushstring( _L, key.c_str() );

	// Add the method to the proxy's methods table, replacing any existing method
	ScriptFunctor * functor = new ScriptFunctor( funct );
	lua_pushlightuserdata( _L, functor );
	lua_settable( _L, methods );

	lua_pop( _L, 1 );
}

//
// addSetter
//
template <typename T, typename AGGREGATE>
void Scriptable<T,AGGREGATE>::addSetter( const char * szSetter, ScriptMethod funct )
{
	lua_State * _L = static_cast<lua_State *>(m_env);
	int setters = getsettertable( _L );

	// Convert the szLuaName to a lowercase std::string
	std::string key(szSetter);
	std::transform( key.begin(), key.end(), key.begin(), tolower );
	lua_pushstring( _L, key.c_str() );

	// Add the method to the proxy's methods table, replacing any existing method
	ScriptFunctor * functor = new ScriptFunctor( funct );
	lua_pushlightuserdata( _L, functor );
	lua_settable( _L, setters );

	lua_pop( _L, 1 );
}

//
// getnamespacetable
//
template <typename T, typename AGGREGATE>
int Scriptable<T,AGGREGATE>::getnamespacetable( lua_State * _L )
{
	lua_pushstring( _L, AGGREGATE::ms_kRTTI.GetName() );
	lua_gettable( _L, LUA_GLOBALSINDEX );

	if ( lua_isnil( _L, -1 ) )
	{
		luaL_error( _L, "namespace '%s' does not exist." );
		return 0;
	}

	return lua_gettop( _L );
}

//
// getmethodtable
//
template <typename T, typename AGGREGATE>
int Scriptable<T,AGGREGATE>::getmethodtable( lua_State * _L )
{
	return getmethodtable( _L, &AGGREGATE::ms_kRTTI );
}

template <typename T, typename AGGREGATE>
int Scriptable<T,AGGREGATE>::getmethodtable( lua_State * _L, const RTTI_TYPE * rtti )
{
	// Extract the methods table from this RTTI's metatable
	lua_pushlightuserdata( _L, const_cast<RTTI_TYPE*>(rtti) );
	lua_gettable( _L, LUA_REGISTRYINDEX );	

	if ( !lua_istable(_L,-1) )
	{
		luaL_error( _L, "couldn't find metatable for class %s", T::ms_kRTTI.GetName() );
		return 0;
	}

	// Get the methods table
	lua_pushliteral( _L, "methods" );
	lua_gettable( _L, -2 );
	lua_remove( _L, -2 );

	if ( !lua_istable(_L,-1) )
	{
		luaL_error( _L, "couldn't find methods table for class %s", T::ms_kRTTI.GetName() );
		return 0;
	}

	// Return the index
	return lua_gettop(_L);
}

//
// getsettertable
//
template <typename T, typename AGGREGATE>
int Scriptable<T,AGGREGATE>::getsettertable( lua_State * _L )
{
	// Extract the methods table from this RTTI's metatable
	lua_pushlightuserdata( _L, const_cast<RTTI_TYPE*>(&AGGREGATE::ms_kRTTI) );
	lua_gettable( _L, LUA_REGISTRYINDEX );	

	if ( !lua_istable(_L,-1) )
	{
		luaL_error( _L, "couldn't find metatable for class %s", T::ms_kRTTI.GetName() );
		return 0;
	}

	// Get the methods table
	lua_pushliteral( _L, "setters" );
	lua_gettable( _L, -2 );
	lua_remove( _L, -2 );

	if ( !lua_istable(_L,-1) )
	{
		luaL_error( _L, "couldn't find methods table for class %s", T::ms_kRTTI.GetName() );
		return 0;
	}

	// Return the index
	return lua_gettop(_L);
}

//
// gettable
//
template <typename T, typename AGGREGATE>
int Scriptable<T,AGGREGATE>::gettable( lua_State* _L )
{
	// stack has userdata and index

	// Get our proxy
	T ** handle = static_cast<T**>( lua_touserdata( _L, 1 ) );
	T * object = *handle;
	if ( !object )
		return luaL_error( _L, "__gettable:  invalid userdata\n" );

	// Get the index, which will be our LuaFunctionMap key
	const char* szKeyOrig = lua_tostring( _L, 2 );
	if ( !szKeyOrig )
		return luaL_error( _L, "__gettable:  invalid index\n" );

	// Convert the key argument into a lowercase std::string
	std::string strKey( szKeyOrig );
	std::transform( strKey.begin(), strKey.end(), strKey.begin(), tolower );

	// Get our methods table
	void* functor = NULL;
	const RTTI_TYPE* pRTTI = &AGGREGATE::ms_kRTTI;
	while ( !functor )
	{
		// Get the methods table (leaves on top of stack, returns that index)
		int methods = getmethodtable( _L, pRTTI );
		if ( !methods )
			return 0;

		// Find the method
		lua_pushstring( _L, strKey.c_str() );
		lua_gettable( _L, methods );
		functor = lua_touserdata( _L, -1 );

		// If the root is null, we've reached the bottom of the hierachy
		// the method was not found and the call unsuccessful
		if ( !pRTTI )
			return luaL_error( _L, "__gettable:  invalid method\n" );

		// Get the next RTTI
		pRTTI = pRTTI->GetBaseRTTI();
		lua_pop( _L, 2 );
	}

	// Return our thunk with a closure binding the the functor
	lua_pushlightuserdata( _L, static_cast<void*>(functor) );
	lua_pushlightuserdata( _L, static_cast<void*>(handle) );
	lua_pushcclosure( _L, thunk, 2 );

	return 1;
}

//
// settable
//
template <typename T, typename AGGREGATE>
int Scriptable<T,AGGREGATE>::settable( lua_State* _L )
{
	// stack has userdata and index

	// Get our proxy
	T ** handle = static_cast<T**>( lua_touserdata( _L, 1 ) );
	T * object = *handle;
	if ( !object )
		return luaL_error( _L, "__settable:  invalid userdata\n" );

	// Get the index, which will be our LuaFunctionMap key
	const char* szKeyOrig = lua_tostring( _L, 2 );
	if ( !szKeyOrig )
		return luaL_error( _L, "__settable:  invalid index\n" );

	// Convert the key argument into a lowercase std::string
	std::string strKey( szKeyOrig );
	std::transform( strKey.begin(), strKey.end(), strKey.begin(), tolower );

	// Get our methods table
	void* vfunctor = NULL;
	const RTTI_TYPE* pRTTI = &AGGREGATE::ms_kRTTI;
	while ( !vfunctor )
	{
		// If the root is null, we've reached the bottom of the hierachy
		// the method was not found and the call unsuccessful
		if ( !pRTTI )
			return luaL_error( _L, "__settable:  invalid method\n" );

		// Get the methods table (leaves on top of stack, returns that index)
		int setter = getsettertable( _L );
		if ( !setter )
			return 0;

		// Find the method
		lua_pushstring( _L, strKey.c_str() );
		lua_gettable( _L, setter );
		vfunctor = lua_touserdata( _L, -1 );

		// Get the next RTTI
		pRTTI = pRTTI->GetBaseRTTI();
		lua_pop( _L, 2 );
	}

	// Thunking doesn't work, we need to explicitly call the function
	ScriptFunctor * functor = static_cast<ScriptFunctor*>(vfunctor);
	return (*functor)( object, _L );
}

//
// gc
//
template <typename T, typename AGGREGATE>
int Scriptable<T,AGGREGATE>::gc( lua_State* _L )
{
	// Get our proxy
	T ** handle = static_cast<T**>( lua_touserdata( _L, 1 ) );
	T * object = *handle;
	if ( !object )
		return luaL_error( _L, "gc:  invalid userdata\n" );

	// delete the proxy
	delete object;

	return 0;
}

//
// thunk
//
template <typename T, typename AGGREGATE>
int Scriptable<T,AGGREGATE>::thunk( lua_State* _L )
{
	// Get the functor
	void * p = lua_touserdata( _L, lua_upvalueindex(1) );
	if ( !p )
		return 0;
	ScriptFunctor * functor = static_cast<ScriptFunctor*>(p);

	// Get "this" pointer
	T ** handle = static_cast<T**>( lua_touserdata( _L, lua_upvalueindex(2) ) );
	T * pThis = *handle;

	// Remove the first value (functor) to give a clean stack to our method
	lua_remove( _L, 1 );

	// Invoke the functor
	return (*functor)( pThis, _L );
}

}; // Katana

#endif // _SCRIPTABLE_H