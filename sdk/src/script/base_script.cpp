/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		base_script.cpp
	Author:		Eric Bryant

	Registers all classes in base library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>

#include "luahelper.h"
#include "luabind_policy.h"
#include "katana_base_includes.h"
#include "scriptengine.h"

// --------------------------------------------------------------------
// Scriptable Functions
// --------------------------------------------------------------------

const char * getStreamableType( Streamable * object )	{ return object->GetRTTI()->GetName(); }

// --------------------------------------------------------------------
// Base_Register
// --------------------------------------------------------------------
bool Base_Register( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	// Stream Registration
	module( env )
	[
		class_< Streamable, shared_ptr<Streamable> >( "Streamable" )
		,
		def( "getStreamableType", &getStreamableType )
		,
		class_<kistream>( "kistream" )
			.def( constructor<const char *>() )
			.def( "getObject", (shared_ptr<Streamable>(kistream::*)(unsigned int))&kistream::getObject )
			.def( "getObject", (shared_ptr<Streamable>(kistream::*)(const char *))&kistream::getObject )
			.def( "endStream", &kistream::endStream )
		,
		class_<kostream>( "kostream" )
			.def( constructor<const char *>() )
			.def( "insertObject", &kostream::insertObject, shared_ptr_policy( _1 ) )
			.def( "endStream", &kostream::endStream )		
	];

	return true;
}