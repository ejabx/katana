/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		luaerr.cpp
	Author:		Eric Bryant

	Helper function to return lua errors
*/

#include "katana_core_includes.h"
#include "luahelper.h"
#include "luaerr.h"

//
// Extern Variables
//
int			Katana::MAX_LUA_ERROR_STRING	= 2048;

//
// LuaErr
//
int Katana::LuaErr( const char * format, ... )
{
	// Compose the variable arguments into a single string
	va_list l_va;
	char * buffer = new char[ Katana::MAX_LUA_ERROR_STRING ];

	va_start(l_va, format);
	vsprintf(buffer, format, l_va);
	va_end(l_va);

	// Push the error string
	lua_pushstring( Katana::GLOBAL_LUA_STATE, buffer );
	
	// Have lua handle the error
	lua_error( Katana::GLOBAL_LUA_STATE );

	// Destroy the temporary buffer
	delete [] buffer;

	return 0; // Must return 0 or lua will complain
}