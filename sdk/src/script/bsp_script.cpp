/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		bsp_script.cpp
	Author:		Eric Bryant

	Registers all classes in bsp library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "luabind_policy.h"
#include "scriptengine.h"
#include "scene/visible.h"
#include "scene/visnode.h"
#include "scene/bspscene.h"

// --------------------------------------------------------------------
// Registration
// --------------------------------------------------------------------

//
// BSPScene Registration
//
bool BSPScene::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	VisNode::OnRegister( env );

	module( env )
		[
			class_< BSPScene, VisNode, shared_ptr<BSPScene> >( "BSPScene" )
			.def( constructor<>() )
			.def( constructor< shared_ptr<Geometry> >(), shared_ptr_policy( _1 ) )
			.def( constructor< shared_ptr<Geometry>, unsigned int, unsigned int >(), shared_ptr_policy( _1 ) )
		];

	return true;
}