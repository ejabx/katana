/*
	Katana Engine
	Copyright © 2004 Eric Bryant, Inc.

	File:		animation_script.cpp
	Author:		Eric Bryant

	Registers all classes in animation library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "luabind_policy.h"
#include "animation/animation.h"
#include "animation/keyframe.h"
#include "animation/animationtrack.h"

// --------------------------------------------------------------------
// Registration
// --------------------------------------------------------------------

//
// Keyframe Registration
//
bool Keyframe::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
		[
			class_< Keyframe, shared_ptr<Keyframe> >( "Key" )
			.def( constructor<float>() )
			.def( constructor<float, const Point3 &>() )
			.def( constructor<float, const Quaternion &>() )
			.def( constructor<float, const Point3 &, const Quaternion &>() )
			.def_readwrite( "time", &Keyframe::m_time )
			.def_readwrite( "translation", &Keyframe::m_translation )
			.def_readwrite( "rotation", &Keyframe::m_rotation )
		];

	return true;
}

//
// Animation Registration
//
bool Animation::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
		[
			class_< Animation, shared_ptr<Animation> >( "Animation" )
			.def( constructor<>() )
			.def( constructor< shared_ptr<AnimationTrack> >() )
			.def( constructor< shared_ptr<Animation> >() )
			.def( "addTrack",			Animation::addTrack )
			.def( "clearTracks",		Animation::clearTracks )
			.def( "getTrack",			Animation::getTrack )
			.def( "setScaleTime",		Animation::setScaleTime )
			.def( "getScaleTime",		Animation::getScaleTime )
			.def( "setLooping",			Animation::setLooping )
			.def( "getLooping",			Animation::getLooping )
			.def( "getAnimationLength",	Animation::getAnimationLength )
			.def( "setAnimationTime",	Animation::setAnimationTime )
			.def( "getAnimationTime",	Animation::getAnimationTime )
		];

	return true;
}

//
// AnimationTrack Registration
//
bool AnimationTrack::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
		[
			class_< AnimationTrack, shared_ptr<AnimationTrack> >( "AnimationTrack" )
			.def( constructor<>() )
			.def( "addKey",		AnimationTrack::addKeyframe )
			.def( "clearKeys",	AnimationTrack::clearKeyframes )
			.def( "getKey",		AnimationTrack::getKeyframe )
		];

	return true;
}