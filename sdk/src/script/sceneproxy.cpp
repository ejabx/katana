/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		sceneproxy.cpp
	Author:		Eric Bryant

	Scriptable proxy object for the Scene class
*/

#include <map>
#include <algorithm>
#include <functional>
#include <boost/function.hpp>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "../base/kbase.h"
#include "../base/rtti.h"
#include "../base/refcount.h"
#include "../base/log.h"
#include "../base/refcount.h"
#include "../base/karray.h"
#include "../base/streamable.h"
#include "../base/listener.h"
#include "../base/kstring.h"
#include "../base/color.h"
#include "../math/point.h"
#include "../math/matrix.h"
#include "../math/quaternion.h"
#include "../math/bound.h"
#include "../scene/visible.h"
#include "../scene/visnode.h"
#include "../scene/scenecontext.h"
#include "../scene/scenegraph.h"
#include "../scene/controller.h"
#include "../scene/mousecontroller.h"
#include "../scene/keyboardcontroller.h"
#include "../scene/scriptcontroller.h"
#include "luaargs.h"
#include "luaerr.h"
#include "scriptable.h"
#include "scriptableproxy.h"
#include "sceneproxy.h"
using namespace Katana;

//
// RTTI Definition
// 
KIMPLEMENT_ROOT_RTTI_ALIAS( SceneProxy, Scene );
KIMPLEMENT_SCRIPT( SceneProxy );

//
// Constants
//
const int KEYBOARD_CONTROLLER	= 1;
const int MOUSE_CONTROLLER		= 2;
const int JOYSTICK_CONTROLLER	= 3;
const int SCRIPT_CONTROLLER		= 4;

//
// Externally defined structures
// 
extern AutoPtr<SceneGraph> katana_scene;

//
// Local Variables
//
Controller * g_keyboardController	= 0;
Controller * g_mouseController		= 0;
Controller * g_scriptController		= 0;

//
// OnCreate
//
int SceneProxy::OnRegister( lua_State * _L )
{
	// Call base class's register to initialize the class with the scripting environent
	// Override the namespace name with "Scene" instead of "SceneProxy"
	InitScriptObject( _L );

	// Add the global methods for creation and loading katana objecs.
	addGlobal( "AddNode", __AddNode );
	addGlobal( "RemoveNode",  __RemoveNode );
	addGlobal( "AddController", __AddController );
	addGlobal( "RemoveController",  __RemoveController );

	return 0;
}

//
// OnDestroy
//
int SceneProxy::OnUnregister( lua_State * _L )
{
	// Call the base class's unregister to unlink the class with the scripting envrionment
	UnlinkScriptObject( _L );

	return 0;
}

// ----------------------------------------------------

//
// AddNode
//
KIMPLEMENT_SCRIPT_GLOBAL( SceneProxy, AddNode )
{
	// Check whether the scene has been initialized yet
	if ( !katana_scene.isValid() ) return LUA_ERROR( "Scene uninitialized" );

	// Encapsulate the arguments
	LuaArgs args( lua );

	// We need at least one argument
	if ( args.getNumInArgs() == 0 ) return LUA_ERROR( "Need a visible object as a parameter" );

	// Retrieve the object
	Streamable * ref = args.getObject();

	// Cast the object into a visible object to add to the scene
	Visible * visObject = KDynamicCast<Visible>( ref );
	if ( visObject )
		katana_scene->addNode( visObject );

	return LUA_SUCCESS;
}

//
// RemoveNode
//
KIMPLEMENT_SCRIPT_GLOBAL( SceneProxy, RemoveNode )
{
	// Check whether the scene has been initialized yet
	if ( !katana_scene.isValid() ) return LUA_ERROR( "Scene uninitialized" );

	return LUA_ERROR( "Not implemented" );
}

//
// AddController
//

KIMPLEMENT_SCRIPT_GLOBAL( SceneProxy, AddController )
{
	// Check whether the scene has been initialized yet
	if ( !katana_scene.isValid() ) return LUA_ERROR( "Scene uninitialized" );

	// Encapsulate the arguments
	LuaArgs args( lua );

	// We need at least one argument
	if ( args.getNumInArgs() == 0 ) return LUA_ERROR( "Need a controller index as a parameter. See katana.ks for constants." );

	// Grab the value of the controller index
	int controllerIndex = args.getInt();

	// If the controller index is invalid, exit
	if ( controllerIndex < 0 ) return LUA_ERROR( "Invalid controller index" );

	// Otherwise, we'll create a new controller based on the index
	Controller * controller = 0;
	switch ( controllerIndex )
	{
		case KEYBOARD_CONTROLLER:
		{
			g_keyboardController = controller = new KeyboardController;
			break;
		}
		case MOUSE_CONTROLLER:
		{
			// The mouse controller needs an origin to rotate around,
			// we need to grab an object's coordinate frame for this purpose.
			Streamable * object = args.getObject();
			if ( object )
			{
				Visible * vis = KDynamicCast<Visible>( object );
				if ( vis )
				{
					g_mouseController = controller = new MouseController( vis );

					// We'll also set the translation and rotation sensitivity from script (if possible)
					float transSensitivity = args.getFloat();
					float rotSensitivity = args.getFloat();
					
					if ( transSensitivity < 0 ) transSensitivity = 0.05f;
					if ( rotSensitivity < 0 ) rotSensitivity = 1;

					static_cast<MouseController*>(controller)->setSensitivity( rotSensitivity, transSensitivity );
				}
			}
			break;
		}
		case SCRIPT_CONTROLLER:
		{
			// The scripting controller needs a function to call per game tick
			kstring & str = args.getString();
			if ( str.isValid() )
			{
				g_scriptController = controller = new ScriptController( str );
			}
		}
	}

	// Error checking
	if ( !controller )
		return LUA_ERROR( "Unable to create controller with the proper parameters" );

	// Add the controller to the scene
	katana_scene->addController( controller );

	return LUA_SUCCESS;
}

//
// RemoveController
//

KIMPLEMENT_SCRIPT_GLOBAL( SceneProxy, RemoveController )
{
	// Check whether the scene has been initialized yet
	if ( !katana_scene.isValid() ) return LUA_ERROR( "Scene uninitialized" );

	// Encapsulate the arguments
	LuaArgs args( lua );

	// We need at least one argument
	if ( args.getNumInArgs() == 0 ) return LUA_ERROR( "Need a controller index as a parameter. See katana.ks for constants." );

	// Grab the value of the controller index
	int controllerIndex = args.getInt();

	// If the controller index is invalid, exit
	if ( controllerIndex < 0 ) return LUA_ERROR( "Invalid controller index" );

	// Otherwise, we'll create a new controller based on the index
	Controller * controller = 0;
	switch ( controllerIndex )
	{
		case KEYBOARD_CONTROLLER:
		{
			katana_scene->removeController( g_keyboardController );
			break;
		}
		case MOUSE_CONTROLLER:
		{
			katana_scene->removeController( g_mouseController );
			break;
		}
	}

	return LUA_SUCCESS;
}