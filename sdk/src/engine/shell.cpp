/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		shell.cpp
	Author:		Eric Bryant

	Provides the interface to hook into the Katana Game Engine
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "base/comptr.h"
#include "system/systemtimer.h"
#include "render/rendertypes.h"
#include "render/render.h"
#include "input/inputsystem.h"
#include "physics/physicssystem.h"
#include "scene/scenecontext.h"
#include "scene/scenegraph.h"
#include "system/systeminfo.h"
#include "system/systemdialog.h"
#include "application.h"
#include "debugoutput.h"
#include "gameengine.h"
#include "gamesettings.h"
#include "console.h"
#include "textdisplay.h"
#include "shell.h"

//
// Constants
//
const char * DEFAULT_KATANA_SCRIPT			= "katana.ks";
const char * DEFAULT_KATANA_CONFIGURATION	= "katana.xml";

//
// External variables
// 
shared_ptr<Application>		katana_app;
shared_ptr<GameEngine>		katana_game;
shared_ptr<Render>			katana_render;
shared_ptr<ScriptEngine>	katana_script;
shared_ptr<InputSystem>		katana_input;
shared_ptr<DebugOutput>		katana_debug;
shared_ptr<SceneGraph>		katana_scene;
shared_ptr<GameSettings>	katana_settings;
shared_ptr<Console>			katana_console;
shared_ptr<PhysicsSystem>	katana_physics;
shared_ptr<TextDisplay>		katana_text;

//
// Katana_Init
//
bool Katana::Katana_Init()
{
	RenderInfo Info;

	// Create the console
	katana_console.reset( new Console() );

	// Load the default katana settings
	katana_settings.reset( new GameSettings( DEFAULT_KATANA_CONFIGURATION ) );

	// The above console is created before this line because the log is logged to the console
	KLOG("Katana Engine Initialization...");

	// Gather the system information
	SystemInfo systemInfo;
	systemInfo.queryAllSystemInfo();

	// Only display the startup dialog if this setting is true
	if ( katana_settings->displayStartupDialog )
	{
		// System dialog which queries the user on application resolution and startup script
		KatanaStartupDialog dialog( systemInfo );

		// Did the user press cancel. If so, quit the application
		if ( !dialog.showDialog() )
			return false;
	}
	else
	{
		// Because the screen resolution dialog wasn't displayed, choose the first
		// device as the render device
		katana_settings->deviceName = systemInfo.getRenderDevices()[0].c_str();
	}

	// Create the Main Application
	if ( !katana_app ) katana_app.reset( Application::Create() );

	// Startup the Application
	katana_app->Startup();

	// Create the Renderer
	if ( !katana_render ) katana_render.reset( Render::CreateRenderer( Info ) );

	// Setup the Render Parameters
	WindowHandle hWnd; int x, y, cx, cy; bool full;
	katana_app->GetWindowInfo(hWnd, x, y, cx, cy, full);

	Info.hMainWindow = hWnd;
	Info.uiTargetWidth = katana_settings->resolutionWidth;
	Info.uiTargetHeight = katana_settings->resolutionHeight;
	Info.bFullscreen = katana_settings->fullscreen;
	Info.uiTargetColorDepth = katana_settings->resolutionBitDepth;
	Info.bUseReferenceDriver = false; // TODO: Support in Render Dialog UI
	KLOG("Katana Window Creation (%dx%dx%d)", Info.uiTargetWidth, Info.uiTargetHeight, Info.uiTargetColorDepth);

	// Initialize the Renderer
	if ( !katana_render->Initialize( Info ) )
		return false;

	// Create the Game Engine
	if ( !katana_game ) katana_game.reset( new GameEngine );
	
	// Store the render within the game engine
	katana_game->m_currentRenderer = katana_render;

	// Create the Text Display
	if ( !katana_text ) katana_text.reset( new TextDisplay );

	// Setup the initial font for the text display
	// TODO: This information should come from the katana.xml initialization file
	katana_text->setFont( katana_settings->fontTextureName, katana_settings->fontCharacterSizes );

	// Store the text display within the game engine
	katana_game->m_textdisplay = katana_text;

	// Create the Input Engine
	if ( !katana_input ) katana_input.reset( new InputSystem );

	// Initialize the Input System
	if ( !katana_input->Initialize( hWnd ) )
	{
		KLOG2( "!WARNING: One or more of the input devices did not initialize properly" );
	}

	// Store the input system within the game engine
	katana_game->m_input = katana_input;

	// Create the Physics System
	if ( !katana_physics ) katana_physics.reset( new PhysicsSystem );

	// Initialize the Physics System
	if ( !katana_physics->initialize() )
	{
		KLOG2( "!WARNING: Physics System failed to initialize" );
	}

	// Store the physics system within the game engine
	katana_game->m_physics = katana_physics;

	// Create the Script Engine
	if ( !katana_script ) katana_script.reset( new ScriptEngine );

	// Initialize the Script Engine
	if ( !katana_script->Initialize() )
		return false;

	// Store the scripting engine within the game engine
	katana_game->m_scripting = katana_script;

	// Create the debug output device
	if ( !katana_debug ) katana_debug.reset( new DebugOutput );

	// Store the debug output within the game engine
	katana_game->m_debug = katana_debug;

	// Initialize the Game Engine
	if ( !katana_game->OnStartup() )
		return false;

	// Get a reference to the scene graph
	if ( !katana_scene ) katana_scene = katana_game->getScene();

	// Store the Render and Game within the Application
	katana_app->m_spGameEngine = katana_game;
	katana_app->m_spRenderer = katana_render;

	// The Katana Library has been initialized properly; execute the default script katana.ks
	// to setup the scripting environment (such as scripting constants).
	KLOG("Default script: '%s'", DEFAULT_KATANA_SCRIPT);
	if ( !katana_script->execFile( DEFAULT_KATANA_SCRIPT ) )
	{
		KLOG2("!WARNING: Unable to execute script katana.ks. Scripting Engine was not initialized properly.");
	}
	KLOG("Executing script: '%s'", katana_settings->startupScript.c_str());
	if ( !katana_settings->startupScript.empty() && !katana_script->execFile( katana_settings->startupScript.c_str() ) )
	{
		KLOG2("!WARNING: Unable to execute the default script '%s'.", katana_settings->startupScript.c_str());
	}

	// Considering we startup successfully, let's resave the game settings
	if ( !katana_settings->saveSettings( DEFAULT_KATANA_CONFIGURATION ) )
	{
		KLOG2("!WARNING: Unable to save the default settings to '%s'.", DEFAULT_KATANA_CONFIGURATION );
	}

	return true;
}

//
// Katana_Run
//
bool Katana::Katana_Run()
{
	return katana_app->Run();
}

//
// Katana_Shutdown
//
bool Katana::Katana_Shutdown()
{
	KLOG("Katana Engine Termination");

	katana_game.reset();
	katana_app.reset();
	katana_render.reset();
	katana_script.reset();
	katana_scene.reset();
	katana_input.reset();
	katana_debug.reset();
	katana_settings.reset();
	katana_console.reset();

	return true;
}