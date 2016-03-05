/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scriptregistry.cpp
	Author:		Eric Bryant

	Objects whose responsible for registering proxy objects (with methods) 
	with the scripting environment.
*/

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "scriptregistry.h"
using namespace Katana;

//
// registerObject
//
void ScriptRegistry::registerObject( const char * szName )
{
	if ( m_env )
	{
		// Cast the lua state
		lua_State * _L = (lua_State *)m_env;
	}
}