/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		terrain_script.cpp
	Author:		Eric Bryant

	Registers all classes in scene library.
*/

#pragma warning(disable:4267)
#include <string>
#include <vector>
#include <map>
#include <list>

extern "C" 
{
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
}

#include "luahelper.h"
#include "luabind/luabind.hpp"
#include "luabind/smart_policy.hpp"
#include "luabind/iterator_policy.hpp"
#include "../base/kbase.h"
#include "../base/refcount.h"
#include "../base/rtti.h"
#include "../base/karray.h"
#include "../base/kstream.h"
#include "../base/kstring.h"
#include "../base/streamable.h"
#include "../base/color.h"
#include "scriptengine.h"
#include "../math/point.h"
#include "../math/bound.h"
#include "../math/matrix.h"
#include "../math/quaternion.h"
#include "../render/geometry.h"
#include "../render/vertexbuffer.h"
#include "../render/heightfield.h"
#include "../render/renderstate.h"
#include "../scene/visible.h"
#include "../scene/visnode.h"
#include "../scene/scenecontext.h"
#include "../scene/terrain.h"
#include "../scene/terrainsettings.h"
#include "../system/systemfile.h"
using namespace Katana;

// --------------------------------------------------------------------
// Registration
// --------------------------------------------------------------------

//
// Heightfield Registration
//
bool Heightfield::OnRegister( ScriptEnv env )
{
	REGISTER_SCRIPTING_GUARD();

	class_<Heightfield, Geometry>( lua, "Heightfield" )
		.def( constructor<>() )
		.def( constructor<const char *,int,int>() )
		.def( "loadFromTexture",	&loadFromTexture )
		.def( "getHeightAt",		&getHeightAt )
		.def( "getSlopeAt",			&getSlopeAt )
		.def( "convertToTriangles", &convertToTriangles )
		.def( "isValid",			&isValid )
	;

	return true;
}

//
// Terrain Registration
//
bool Terrain::OnRegister( ScriptEnv env )
{
	REGISTER_SCRIPTING_GUARD();

	class_<TerrainSettings>( lua, "TerrainSettings" )
		.def( constructor<>() )
		.def( constructor<const char *>() )
		.def( constructor<const char *, unsigned int, unsigned int>() )
		.def_readwrite( "heightMapFile",	&TerrainSettings::heightMapFileName )
		.def_readwrite( "worldWidth",		&TerrainSettings::worldWidth )
		.def_readwrite( "worldHeight",		&TerrainSettings::worldHeight )
	;

	class_<Terrain, VisNode>( lua, "Terrain" )
		.def( constructor<>() )
		.def( constructor<TerrainSettings &>() )
		.def( "construct",					&Terrain::construct )
		.def( "isInitialized",				&Terrain::isInitialized )
		.def( "getActivePatchCount",		&Terrain::getActivePatchCount )
		.def( "getTriangleCount",			&Terrain::getTriangleCount )
		.def( "getTerrainPatches",			&Terrain::getTerrainPatches, return_stl_iterator )
	;

	return true;
}