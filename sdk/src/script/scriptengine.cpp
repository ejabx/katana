/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scriptengine.cpp
	Author:		Eric Bryant

	Manager execution of LUA scripts within the game system
*/

#include <windows.h>
#include <luabind/luabind.hpp>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "scriptengine.h"
#include "luahelper.h"
#include "luaconsole.h"
#include "base/log.h"

// Make sure client includes LUA libaries
#ifdef _DEBUG
	#pragma comment(lib, "lua_d.lib")
	#pragma comment(lib, "lualib_d.lib")
	#pragma comment(lib, "luabind_d.lib" )
#else
	#pragma comment(lib, "lua.lib")
	#pragma comment(lib, "lualib.lib")
	#pragma comment(lib, "luabind.lib" )
#endif

//
// Extern Functions
//
bool Base_Register( lua_State * env );
bool Math_Register( lua_State * env );

//
// Static Variables
//
shared_ptr< std::vector<RegistrationFunction> > ScriptEngine::m_RegistrationFunctions;

//
// Extern Variables
//
lua_State * Katana::GLOBAL_LUA_STATE = 0;


//
// Local Functions
//
int __DefaultErrorHandler( lua_State * _L );
int __DefaultPrint( lua_State * _L );
int __DefaultPanic( lua_State * _L );

//
// Constructor
//
ScriptEngine::ScriptEngine()
{
}

//
// Destructor
//
ScriptEngine::~ScriptEngine()
{
	Terminate();
}

//
// Initialize
//
bool ScriptEngine::Initialize()
{
	// Lua's default environment
	GLOBAL_LUA_STATE = lua_open();
	if ( GLOBAL_LUA_STATE == NULL )
		return false;

	// Lua's base libraries
	luaopen_base( GLOBAL_LUA_STATE );
	luaopen_math( GLOBAL_LUA_STATE );
	luaopen_table( GLOBAL_LUA_STATE );
	luaopen_io( GLOBAL_LUA_STATE );
	luaopen_string( GLOBAL_LUA_STATE );

	// Initialize luabind
	luabind::open( GLOBAL_LUA_STATE );

	// Register the error handlers
	lua_register( GLOBAL_LUA_STATE, "_ERRORMESSAGE", __DefaultErrorHandler );
	lua_register( GLOBAL_LUA_STATE, "_ALERT", __DefaultErrorHandler );
	lua_register( GLOBAL_LUA_STATE, "print", __DefaultPrint );

	// Log
	KLOG( "Script Initialization" );

	// Register all the functions
	if ( m_RegistrationFunctions )
	{
		for( std::vector<RegistrationFunction>::iterator iter = m_RegistrationFunctions->begin();
			 iter != m_RegistrationFunctions->end();
			 iter++ )
		{
			(*iter)( GLOBAL_LUA_STATE );
		}
	}

	// Register the Math Functions
	Base_Register( GLOBAL_LUA_STATE );
	Math_Register( GLOBAL_LUA_STATE );

	return true;
}

//
// Terminate
//
bool ScriptEngine::Terminate()
{
	// Close the envionment
	lua_close( GLOBAL_LUA_STATE );
	GLOBAL_LUA_STATE = NULL;

	return true;
}

//
// execBuffer
//
bool ScriptEngine::execBuffer( std::string buffer )
{
	// Execute the buffer
	lua_dostring( GLOBAL_LUA_STATE, buffer.c_str() );

	return true;
}

//
// execFile
//
bool ScriptEngine::execFile( const char * file )
{
	// Check for empty filename
	if ( !file || !(*file) ) return false;

	// Execute the level file
	int nResult = lua_dofile( GLOBAL_LUA_STATE, file );
	if ( nResult != 0 )
	{
		switch ( nResult )
		{
		case LUA_ERRRUN:	KLOG( "LUA_ERRRUN - error while running the chunk.\n" ); break;
		case LUA_ERRSYNTAX:	KLOG( "LUA_ERRSYNTAX - syntax error during pre-compilation.\n" ); break;
		case LUA_ERRMEM:	KLOG( "LUA_ERRMEM - memory allocation error. For such errors, Lua does not call _ERRORMESSAGE (see Section 4.7).\n" ); break; 
		case LUA_ERRERR:	KLOG( "LUA_ERRERR - error while running _ERRORMESSAGE. For such errors, Lua does not call _ERRORMESSAGE again, to avoid loops.\n" ); break; 
		case LUA_ERRFILE:	KLOG( "LUA_ERRFILE - error opening the file (only for lua_dofile). In this case, you may want to check errno, call strerror, or call perror to tell the user what went wrong.\n" ); break; 
		}
		return false;
	}

	return true;
}

//
// execFunction
//
bool ScriptEngine::execFunction( const char * function, int numArgs, ... )
{
	int args = 0;
	int results = 0;

	// Push the function name onto the stack and retrieve a reference to the function
	lua_pushstring( GLOBAL_LUA_STATE, function );
	lua_gettable( GLOBAL_LUA_STATE, LUA_GLOBALSINDEX );

	// Iterate through the parameters and push the sequentially onto the stack
	va_list list;
	va_start( list, numArgs ); // Initialize the variable arguments

	while ( args < numArgs )
	{
		// Grab the first optional string parameter
		const char * str = va_arg( list, const char * );

		// Push the string (or number) onto the stack
		if ( isdigit( *str ) )
			lua_pushnumber( GLOBAL_LUA_STATE, atof( str ) );
		else
			lua_pushstring( GLOBAL_LUA_STATE, str );

		// Increment the arguments
		args++;
	}

	va_end( list );		// Reset the variable arguments

	// Call the function
	lua_call( GLOBAL_LUA_STATE, args, results );

	return true;
}

//
// getScriptEnvironment
//
lua_State * ScriptEngine::getScriptEnvironment()
{
	return GLOBAL_LUA_STATE;
}

//
// addRegisterFunction
//
bool ScriptEngine::addRegisterFunction( RegistrationFunction pfunc )
{
	if ( !m_RegistrationFunctions )
		m_RegistrationFunctions = shared_ptr< std::vector<RegistrationFunction> >( new std::vector<RegistrationFunction> );
	m_RegistrationFunctions->push_back( pfunc );

	return true;
}


//
// DefaultErrorHandler
//
static int __DefaultErrorHandler( lua_State * _L )
{
	const char* message = lua_tostring( _L, 1 );
	KERROR( "[Lua]: [%s]", message );

#ifdef _MSC_VER
	OutputDebugString( "Lua error: [" );
	OutputDebugString( message );
	OutputDebugString( "]\n" );
#endif
	return 0;
}

//
// DefaultPrint
//
static int __DefaultPrint( lua_State * _L )
{
	if ( lua_isstring( _L, -1 ) )
		KPRINT( lua_tostring( _L, -1 ) );

	return 0;
}