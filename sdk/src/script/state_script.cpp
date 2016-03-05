/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		state_script.cpp
	Author:		Eric Bryant

	Registers all classes in render/state library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "scriptengine.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include "render/vertexbuffer.h"
#include "render/texture.h"
#include "render/light.h"
#include "render/renderstate.h"
#include "render/multitexturestate.h"
#include "render/shader.h"
#include "render/lightstate.h"
#include "render/cullstate.h"
#include "render/blendstate.h"

// --------------------------------------------------------------------
// Registration
// --------------------------------------------------------------------

//
// RenderState Registration
//
bool RenderState::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
	[
		class_< RenderState, shared_ptr<RenderState> >( "RenderState" )
	];

	return true;
}

//
// MultitextureState Registeration
//
bool MultitextureState::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	RenderState::OnRegister( env );

	module( env )
	[
		class_< MultitextureState, RenderState, shared_ptr<MultitextureState> >( "MultitextureState" )
			.def( constructor<>() )
			.def( constructor< shared_ptr<Texture>,shared_ptr<Texture>,shared_ptr<Texture>,shared_ptr<Texture> >() )
			.def( "setSourceTexture", &setSourceTexture )
			.def( "getSourceTexture", &getSourceTexture )
			.enum_("BlendType")
			[
				value( "NONE",			NONE ),
				value( "REPLACE",		REPLACE ),
				value( "MODULATE",		MODULATE ),
				value( "DECAL",			DECAL ),
				value( "ADD",			ADD ),
				value( "ADD_SIGNED",	ADD_SIGNED ),
				value( "DETAIL",		DETAIL ),
				value( "ALPHA",			ALPHA )
			]
			.def( "setBlendOperation", &setBlendOperation )
	];

	return true;
}

//
// LightState Registration
//
bool LightState::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	RenderState::OnRegister( env );

	module( env )
	[
		class_< LightState, RenderState, shared_ptr<LightState> >( "LightState" )
			.def( constructor<>() )
	];

	return true;
}

//
// CullState Registration
//
bool CullState::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	RenderState::OnRegister( env );

	module( env )
		[
			class_< CullState, RenderState, shared_ptr<CullState> >( "CullState" )
			.def( constructor<>() )
			.def( constructor<PolygonWinding, CullFace>() )
		];

	return true;
}

//
// BlendState Registration
//
bool BlendState::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	RenderState::OnRegister( env );

	module( env )
		[
			class_< BlendState, RenderState, shared_ptr<BlendState> >( "BlendState" )
			.def( constructor<>() )
			.def( constructor<bool>() )
			.def( constructor<BlendOp, BlendOp>() )
			.enum_("BlendType")
			[
				value( "ZERO",				BLEND_ZERO ),
				value( "ONE",				BLEND_ONE ),
				value( "SRCCOLOR",			BLEND_SRCCOLOR ),
				value( "INVSRCCOLOR",		BLEND_INVSRCCOLOR ),
				value( "SRCALPHA",			BLEND_SRCALPHA ),
				value( "INVSRCALPHA",		BLEND_INVSRCALPHA ),
				value( "DESTALPHA",			BLEND_DESTALPHA ),
				value( "INVDESTALPHA",		BLEND_INVDESTALPHA ),
				value( "DESTCOLOR",			BLEND_DESTCOLOR ),
				value( "INVDESTCOLOR",		BLEND_INVDESTCOLOR ),
				value( "SRCALPHASAT",		BLEND_SRCALPHASAT ),
				value( "BOTHSRCALPHA",		BLEND_BOTHSRCALPHA ),
				value( "BOTHINVSRCALPHA",	BLEND_BOTHINVSRCALPHA )
			]
		];

	return true;
}