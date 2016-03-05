/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scene_script.cpp
	Author:		Eric Bryant

	Registers all classes in render library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>
#include <luabind/iterator_policy.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "render/rendertypes.h"
#include "render/texture.h"
#include "render/light.h"
#include "render/render.h"
#include "scriptengine.h"

//
// Render Registration
//
bool Render::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
	[
		class_< Render, shared_ptr<Render> >( "Render" )
			.def( "setBackgroundColor",			&SetBackgroundColor )
	];

	return true;
}

//
// Texture Registration
//
bool Texture::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
	[
		class_< Texture, shared_ptr<Texture> >( "Texture" )
			.def( constructor<>() )
			.def( constructor<float,float,float,float>() )
			.def( constructor<const char *>() )
			.def( constructor<const char *, bool>() )
			.def( constructor<const char *, FilterMode, RepeatMode>() )
			.def( "getData", &getData )
			.def( "unload", &Unload )
			.def( "isLoaded", &isLoaded )
			.def( "setStayResident", &setStayResident )
			.def( "getStayResident", &getStayResident )
			.def( "getWidth", &getWidth )
			.def( "getHeight", &getHeight )
			.def( "getBitDepth", &getBitDepth )
			.def( "getMipMaps", &getMipMaps )
			.def( "getCubeMap", &getCubeMap )
			.def( "resample", &Resample )
			.enum_("filterMode")
			[
				value( "NEAREST", NEAREST ),
				value( "LINEAR", LINEAR ),
				value( "TRILINEAR", TRILINEAR ),
				value( "NEAREST_MIPMAP", NEAREST_MIPMAP ),
				value( "LINEAR_MIPMAP", LINEAR_MIPMAP )
			]
			.enum_("repeatMode")
			[
				value( "CLAMP_ST", CLAMP_ST ),
				value( "WRAP_ST", WRAP_ST ),
				value( "CLAMP_S_WRAP_T", CLAMP_S_WRAP_T ),
				value( "WRAP_S_CLAMP_T", WRAP_S_CLAMP_T )
			]
			.enum_("pixelFormat")
			[
				value( "NONE", NONE ),
				value( "UNKNOWN", UNKNOWN ),
				value( "RGB", RGB ),
				value( "RGBA", RGBA ),
				value( "BGR_EXT", BGR_EXT ),
				value( "GREYSCALE", GREYSCALE )
			]
	];

	return true;
}

//
// Light Registration
//
bool Light::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
	[
		class_< Light, shared_ptr<Light> >( "Light" )
			.def( constructor<>() )
			.def( "setPosition", &Light::setPosition )
			.def( "getPosition", &Light::getPosition )
			.def( "setRange", &Light::setRange )
			.def( "getRange", &Light::getRange )
			.def( "setAmbient", &Light::setAmbient )
			.def( "getAmbient", &Light::getAmbient )
			.def( "setDiffuse", &Light::setDiffuse )
			.def( "getDiffuse", &Light::getDiffuse )
			.def( "setSpecular", &Light::setSpecular )
			.def( "getSpecular", &Light::getSpecular )
		,
		class_< AmbientLight, Light, shared_ptr<AmbientLight> >( "AmbientLight" )
			.def( constructor<>() )
		,
		class_< DirectionLight, Light, shared_ptr<DirectionLight> >( "DirectionLight" )
			.def( constructor<>() )
			.def( "setDirection", &DirectionLight::setDirection )
			.def( "getDirection", &DirectionLight::getDirection )
		,
		class_< PointLight, Light, shared_ptr<PointLight> >( "PointLight" )
			.def( constructor<>() )
			.def( "setAttn0", &PointLight::setConstantAttenuation )
			.def( "getAttn0", &PointLight::getConstantAttenuation )
			.def( "setAttn1", &PointLight::setLinearAttenuation )
			.def( "getAttn1", &PointLight::getLinearAttenuation )
			.def( "setAttn2", &PointLight::setQuadraticAttenuation )
			.def( "getAttn2", &PointLight::getQuadraticAttenuation )
		,
		class_< SpotLight, PointLight, shared_ptr<SpotLight> >( "SpotLight" )
			.def( constructor<>() )
			.def( "setDirection", &SpotLight::setDirection )
			.def( "getDirection", &SpotLight::getDirection )
			.def( "setInnerCone", &SpotLight::setInnerCone )
			.def( "getInnerCone", &SpotLight::getInnerCone )
			.def( "setOuterCone", &SpotLight::setOuterCone )
			.def( "getOuterCone", &SpotLight::getOuterCone )
			.def( "setFalloff", &SpotLight::setFalloff )
			.def( "getFalloff", &SpotLight::getFalloff )
	];

	return true;
}