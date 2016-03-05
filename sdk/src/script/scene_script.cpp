/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scene_script.cpp
	Author:		Eric Bryant

	Registers all classes in scene library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>
#include <luabind/iterator_policy.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "luabind_policy.h"
#include "scriptengine.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include "render/vertexbuffer.h"
#include "render/renderstate.h"
#include "physics/collidable.h"
#include "scene/visible.h"
#include "scene/visnode.h"
#include "scene/vismesh.h"
#include "scene/scenecontext.h"
#include "scene/scenegraph.h"
#include "scene/camera.h"

//
// External References
//
extern shared_ptr<SceneGraph>		katana_scene;

// --------------------------------------------------------------------
// Local Functions
// --------------------------------------------------------------------

shared_ptr<Visible>		castVisible( shared_ptr<Streamable> object )		{ return boost::dynamic_pointer_cast<Visible>( object ); }
shared_ptr<VisMesh>		castVisMesh( shared_ptr<Streamable> object )		{ return boost::dynamic_pointer_cast<VisMesh>( object ); }
shared_ptr<VisNode>		castVisNode( shared_ptr<Streamable> object )		{ return boost::dynamic_pointer_cast<VisNode>( object ); }
shared_ptr<SceneGraph>	getScene()											{ return katana_scene; }

// --------------------------------------------------------------------
// Registration
// --------------------------------------------------------------------

//
// Extern Functions
//
bool Base_Register( lua_State * env );

//
// SceneGraph Registration
//
bool SceneGraph::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
		[
			class_< SceneStatistics >( "SceneStatistics" )
			.def_readonly( "objectsRendered",			&SceneStatistics::objectsRenderedLastFrame )
			.def_readonly( "trianglesRendered",			&SceneStatistics::trianglesRenderedLastFrame )
			.def_readonly( "totalObjectsRendered",		&SceneStatistics::totalObjectsRendered )
			.def_readonly( "totalTrianglesRendered",	&SceneStatistics::totalTrianglesRendered )
			.def_readonly( "fps",						&SceneStatistics::framesPerSecond )
			.def_readonly( "tps",						&SceneStatistics::trianglesPerSecond )
			,
			class_< SceneGraph, shared_ptr<SceneGraph> >( "SceneGraph" )
			.def( "addNode",							&addNode, shared_ptr_policy( _1 ) )
			.def( "removeNode", 						&removeNode, shared_ptr_policy( _1 ) )
			.def( "removeAllNodes", 					&removeAllNodes )
			.def( "addController",						&addController, shared_ptr_policy( _1 ) )
			.def( "removeController",					&removeController, shared_ptr_policy( _1 ) )
			.def( "removeAllControllers",				&removeAllControllers )
			.def( "getRoot",							&getRoot )
			.def( "setDefaultShader",					&setDefaultShader, shared_ptr_policy( _1 ) )
			.property( "stats",							&SceneGraph::getStatistics )
			,

			def( "getScene", getScene )
		];

	return true;
}

//
// Visible Registration
//
bool Visible::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	// Register Dependicies
	Base_Register( env );
	Collidable::OnRegister( env );

	module( env )
	[
		class_< Visible, Collidable, shared_ptr<Visible> >( "Visible" )
			.def( "setParent",			&setParent )
			.def( "setVisible",			&setVisible )
			.def( "setDirty",			&setDirty )
			.def( "setTranslation",		&setTranslation )
			.def( "setRotation",		&setRotation )
			.def( "setTransform",		&setTransform )
			.def( "setScale", 			&setScale )
			.def( "setBound", 			&setBound )
			.def( "isVisible",			&isVisible )
			.def( "isDirty",			&isDirty )
			.def( "getParent",			&getParent )
			.def( "getTranslation",		&getTranslation )
			.def( "getRotation",		&getRotation )
			.def( "getScale",			&getScale )
			.def( "getLocalBound", 		&getLocalBound )
			.def( "getWorldBound", 		&getWorldBound )
			.def( "setMaterial",		&setMaterial, shared_ptr_policy( _1 ) )
			.def( "setLight", 			&setLight, shared_ptr_policy( _1 ) )
			.def( "getLight", 			&getLight )
			.def( "setAnimation",		&setAnimation, shared_ptr_policy( _1 ) )
			.def( "setCastsShadows",	&setCastsShadows )
			.def( "getCastsShadows",	&getCastsShadows )
			.def( "setBillboard",		&setBillboard )
			.def( "getBillboard",		&getBillboard )
		,
		// TODO: This is temporary, we should find a way to upcast objects from lua
		def( "castVisible", &castVisible ),
		def( "castVisMesh", &castVisMesh ),
		def( "castVisNode", &castVisNode )
	];

	return true;
}

//
// VisNode Registration
//
bool VisNode::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	// Register Dependicies
	Visible::OnRegister( env );

	module( env )
	[
		class_< VisNode, Visible, shared_ptr<VisNode> >( "VisNode" )
			.def( constructor<>() )
			.def( "attachChild",		&attachChild, shared_ptr_policy( _1 ) )
			.def( "detachChild",		&detachChild, shared_ptr_policy( _1 ) )
			.def( "detachAllChildren",	&detachAllChildren )
			.def( "getChildrenCount",	&getChildrenCount )
			.def( "getChildren",		&getChildren, return_stl_iterator )
			.def( "addState",			&addState, shared_ptr_policy( _1 ) )
			.def( "removeState",		&removeState, shared_ptr_policy( _1 ) )
			.def( "removeAllStates",	&removeAllStates )
	];

	return true;
}

//
// VisMesh Registration
//
bool VisMesh::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	// Register Dependicies
	Visible::OnRegister( env );

	module( env )
	[
		class_< VisMesh, Visible, shared_ptr<VisMesh> >( "VisMesh" )
			.def( constructor<>() )
			.def( constructor< shared_ptr<Geometry> >() )
			.def( constructor<const VisMesh &>() )
			.def( "createGeometry", &createGeometry )
			.def( "getGeometry",	&getGeometry )
			.def( "setGeometry",	&setGeometry )
	];

	return true;
}

//
// Camera Registration
//
bool Camera::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	// Register Dependicies
	Visible::OnRegister( env );

	module( env )
	[
		class_< Camera, Visible, shared_ptr<Camera> >( "Camera" )
			.def( constructor<>() )
			.def( constructor<float,float,float,float>() )
			.def( "getUp",			&getUp )
			.def( "getRight",		&getRight )
			.def( "getAt",			&getAt )
			.def( "getPos",			&getPos )
			.def( "getProjection",	&getProjection )
	];

	return true;
}