/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		shader_script.cpp
	Author:		Eric Bryant

	Registers all classes in shaders library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "luabind_policy.h"
#include "scriptengine.h"
#include "render/rendertypes.h"
#include "render/shader.h"
#include "render/hardwarelitshader.h"
#include "render/programableshader.h"

// --------------------------------------------------------------------
// Registration
// --------------------------------------------------------------------

//
// Shader Registration
//
bool Shader::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
	[
		class_< Shader, shared_ptr<Shader> >( "Shader" )
			.def( "addPreRenderState", &addPreRenderState, shared_ptr_policy( _1 ) )
			.def( "addPostRenderState", &addPostRenderState, shared_ptr_policy( _1) )
			.def( "clearPreRenderStates", &clearPreRenderStates )
			.def( "clearPostRenderStates", &clearPostRenderStates )
	];

	return true;
}

//
// HardwareLitShader Registration
//
bool HardwareLitShader::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	Shader::OnRegister( env );

	module( env )
	[
		class_< HardwareLitShader, Shader, shared_ptr<HardwareLitShader> >( "HardwareLitShader" )
			.def( constructor<>() )
			.def( "enableTextures", &enableTextures )
			.def( "enableBlending", &enableBlending )
			.def( "enableLighting", &enableLighting )
	];

	return true;
}

//
// ProgramableShader Registration
//
bool ProgramableShader::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	Shader::OnRegister( env );

	module( env )
	[
		class_< ProgramableShader, Shader, shared_ptr<ProgramableShader> >( "ProgramableShader" )
			.def( constructor<>() )
			.def( constructor<const char *>() )
	];

	return true;
}