/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		effects_script.cpp
	Author:		Eric Bryant

	Registers all special effect classes (sprites, etc.).
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>
#include <luabind/iterator_policy.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "luabind_policy.h"
#include "scriptengine.h"
#include "scene/visible.h"
#include "scene/sprite.h"

// --------------------------------------------------------------------
// Registration
// --------------------------------------------------------------------


//
// Sprite Registration
//
bool Sprite::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	// Register Dependicies
	Visible::OnRegister( env );

	module( env )
		[
			class_< Sprite, Visible, shared_ptr<Sprite> >( "Sprite" )
			.def( constructor<>() )
			.def( constructor< shared_ptr<Texture> >() )
			.def( constructor< shared_ptr<Texture>, float, float >() )
			.def( "setBillboard",	&setBillboard )
			.def( "getBillboard",	&getBillboard )
			.def( "setTexture",		&setTexture, shared_ptr_policy( _1 ) )
			.def( "getTexture",		&getTexture )
		];

	return true;
}