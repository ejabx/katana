/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scriptengine.h
	Author:		Eric Bryant

	Manager execution of scripts within the game system
*/

#ifndef _SCRIPTENGINE_H
#define _SCRIPTENGINE_H

namespace Katana
{

//
// Forward Declarations
//
class GameEngine;

//
// Definitions
//
typedef bool (*RegistrationFunction)(lua_State*);

///
/// ScriptEngine
///
class ScriptEngine
{
public:
	/// Constructor
	ScriptEngine();

	/// Destructor
	virtual ~ScriptEngine();

	/// Initialize the scripting environment
	bool Initialize();

	/// Terminates the scripting environment
	bool Terminate();

public:

	/// Executes the contexts of a buffer
	bool execBuffer( std::string buffer );

	/// Executes the contents of a file
	bool execFile( const char * file );

	/// Executes a function (assuming it has already been loaded). The arguments must
	/// be all strings, however, they will be passed to lua as numbers if they are infact digits.
	bool execFunction( const char * function, int numArgs, ... );

	/// Returns a reference to the scripting environment
	lua_State* getScriptEnvironment();

public:

	/// Adds the registration function to the scripting engine. Clients do not call
	/// this method but use KDECLARE_SCRIPT/KIMPLEMENT_SCRIPT macros.
	static bool addRegisterFunction( RegistrationFunction pfunc );

private:
	/// Vector of registration functions. During initialize, these methods will
	/// be called to allow the classes to register their scripting interfaces.
	static shared_ptr< std::vector<RegistrationFunction> > m_RegistrationFunctions;
};

///
/// KDECLARE_SCRIPT
///
/// Place this function within your class to enable scripting.
/// It is the class writer's responsibility to implement this
/// function which register the methods/properties to expose to the
/// scripting environment.
///
#define KDECLARE_SCRIPT								\
public:												\
	static bool OnRegister( lua_State * env );


///
/// KIMPLEMENT_SCRIPT
/// Place this macro in the class's header file to automatically
/// register the above scripting registration function. It will be
/// called automatically during startup.
#define KIMPLEMENT_SCRIPT( class )						\
	static bool class##RegisterScriptingRetVal = ScriptEngine::addRegisterFunction( &class::OnRegister );

}; // Katana

#endif // _SCRIPTENGINE_H