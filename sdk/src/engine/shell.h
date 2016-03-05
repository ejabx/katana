/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		shell.h
	Author:		Eric Bryant

	Main interface into the Katana Engine. 
	Provides the functionality to start, shutdown, and hook into the Katana Game Engine.
	Application creation, renderer creation, and game engnine initialization is all handled
	automatically by the shell.
*/

#ifndef _SHELL_H
#define _SHELL_H

namespace Katana
{
	/// Initialize the Katana Engine. Creates the application window, creates a renderer,
	/// and initializes the game engine.
	bool Katana_Init();

	/// Call this function to execute the game loop. This function will not return until
	/// the game has exited.
	bool Katana_Run();

	/// Terminate the game engine. Call to destroy the game environment.
	bool Katana_Shutdown();

}; // Katana

///
/// External variables which the client can use to access the internals of the katana game engine.
///
extern shared_ptr<Katana::Application>		katana_app;			/// Main Application (which is generally the windowed app)
extern shared_ptr<Katana::GameEngine>		katana_game;		/// Katana Game Engine
extern shared_ptr<Katana::Render>			katana_render;		/// Device renderer
extern shared_ptr<Katana::ScriptEngine>		katana_script;		/// Scripting engine
extern shared_ptr<Katana::InputSystem>		katana_input;		/// Input system
extern shared_ptr<Katana::DebugOutput>		katana_debug;		/// Display debug output
extern shared_ptr<Katana::SceneGraph>		katana_scene;		/// The Scene Graph
extern shared_ptr<Katana::GameSettings>		katana_settings;	/// Global game engine settings
extern shared_ptr<Katana::Console>			katana_console;		/// The katana engine debugging console
extern shared_ptr<Katana::PhysicsSystem>	katana_physics;		/// The physics system
extern shared_ptr<Katana::TextDisplay>		katana_text;		/// The text display system

#endif _SHELL_H