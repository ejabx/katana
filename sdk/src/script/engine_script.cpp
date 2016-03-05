/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		engine_script.cpp
	Author:		Eric Bryant

	Registers all classes in engine library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "luabind_policy.h"
#include "scriptengine.h"
#include "system/systemtimer.h"
#include "engine/console.h"
#include "engine/gameengine.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include "render/vertexbuffer.h"
#include "render/render.h"
#include "scene/visible.h"
#include "scene/visnode.h"
#include "scene/vismesh.h"
#include "scene/scenecontext.h"
#include "scene/scenegraph.h"
#include "scene/controller.h"
#include "scene/mousecontroller.h"
#include "scene/keyboardcontroller.h"
#include "scene/scriptcontroller.h"
#include "input/inputsystem.h"

//
// External References
//
extern shared_ptr<InputSystem>		katana_input;
extern shared_ptr<Console>			katana_console;
extern shared_ptr<GameEngine>		katana_game;

// --------------------------------------------------------------------
// Local Functions
// --------------------------------------------------------------------

shared_ptr<InputSystem>	getInput()		{ return katana_input; }
shared_ptr<Console>		getConsole()	{ return katana_console; }
shared_ptr<GameEngine>	getEngine()		{ return katana_game; }

// --------------------------------------------------------------------
// Registration
// --------------------------------------------------------------------

//
// Controller Registration
//
bool Controller::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
	[
		class_< Controller, shared_ptr<Controller> >( "Controller" )
	];

	return true;
}

//
// GameEngine Registration
//
bool GameEngine::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
	[
		class_< GameEngine, shared_ptr<GameEngine> >( "GameEngine" )
			.def( "setPause",			&setPause )
			.def( "getPause",			&getPause )
			.def( "scaleTime",			&scaleTime )
			.def( "getScaleTime",		&getScaleTime )
			.def( "stepTime",			&stepTime )
			.def( "getCurrentCamera",	&getCurentCamera )
			.def( "getCurrentRender",	&getCurrentRender )
		,

		def( "getEngine", getEngine )
	];

	return true;
}

//
// InputSystem Registration
//
bool InputSystem::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
	[
		class_< InputSystem, shared_ptr<InputSystem> >( "InputSystem" )
			.def( "enableDevice", &enableDevice )
			.def( "disableDevice", &disableDevice )
			.def( "isDeviceEnabled", &isDeviceEnabled )
			.enum_( "InputDevice" )
			[
				value( "KEYBOARD", INPUT_KEYBOARD ),
				value( "MOUSE", INPUT_MOUSE ),
				value( "JOYSTICK", INPUT_JOYSTICK ),
				value( "JOYSTICK_A", INPUT_JOYSTICK_A ),
				value( "JOYSTICK_B", INPUT_JOYSTICK_B )
			]
		,

		def( "getInput", getInput )
	];

	return true;
}

//
// KeyboardController Registration
//
bool KeyboardController::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	Controller::OnRegister( env );

	module( env )
	[
		class_< KeyboardController, Controller, shared_ptr<KeyboardController> >( "KeyboardController" )
			.def( constructor<>() )
	];

	return true;
}

//
// MouseController Registration
//
bool MouseController::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	Controller::OnRegister( env );

	module( env )
	[
		class_< MouseController, Controller, shared_ptr<MouseController> >( "MouseController" )
			.def( constructor< shared_ptr<Visible> >(), shared_ptr_policy( _1 ) )
			.def( constructor< shared_ptr<Visible>, float, float>(), shared_ptr_policy( _1 ) )
			.def( "setSensitivity", &MouseController::setSensitivity )
	];

	return true;
}

//
// ScriptController Registration
//
bool ScriptController::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	Controller::OnRegister( env );

	module( env )
	[
		class_< ScriptController, Controller, shared_ptr<ScriptController> >( "ScriptController" )
			.def( constructor< const char * >() )
	];

	return true;
}

//
// Console Registration
//
bool Console::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
	[
		class_< Console, shared_ptr<Console> >( "Console" )
			.def( "openConsole", &openConsole )
			.def( "closeConsole", &closeConsole )
		,
		def( "getConsole", &getConsole )
	];

	return true;
}