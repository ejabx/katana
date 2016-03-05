/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scriptregistry.h
	Author:		Eric Bryant

	Objects whose responsible for registering proxy objects (with methods) 
	with the scripting environment.
*/

#ifndef _SCRIPTREGISTRY_H
#define _SCRIPTREGISTRY_H

namespace Katana
{

///
/// ScriptRegistry
///
class ScriptRegistry
{
public:
	/// Register's a script object with the scripting environment
	void registerObject( const char * szName );

public:
	/// Sets the scripting environment [for internal use only]
	void setScriptEnv( void * env )				{ m_env = env; }

private:
	/// The scripting environment
	void * m_env;
};

///
/// RegistryProxy
/// Use this template function to register a proxy
///
template <typename T>
void RegisterProxy( ScriptRegistry * registry )
{
	// Allow the object to register it's functions
	T::OnCreate( registry );

	// Make the object scriptable
	registry->registerObject( T::ms_kRTTI.GetName() );
};

///
/// UnregistryProxy
/// Use this template function to unregister a proxy
///
template <typename T>
void UnregisterProxy( ScriptRegistry * registry )
{
	T::OnDestroy( registry );
};

}; // Katana

#endif // _SCRIPTREGISTRY_H