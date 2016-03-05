/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		application.h
	Author:		Eric Bryant

	Platform-independent application framework which handles "window"
	creation and system events (mouse, keyboard, etc.)	
*/

#ifndef _APPLICATION_H
#define _APPLICATION_H


namespace Katana
{

//
// Forward Declarations
//
class Render;
class Console;
class GameEngine;

///
/// Application
/// Handles the window creation and drives the game engine
class Application
{
public:
	/// Startup the Application
	virtual bool Startup() = 0;

	/// Terminate the Application
	virtual bool Terminate() = 0;

	/// Run the main game loop
	virtual bool Run() = 0;

	/// Reposition the main window
	virtual bool SetPosition(int x, int y, int cx, int cy) = 0;

	/// Set Fullscreen
	virtual bool SetFullscreen(bool bEnable) = 0;

	/// Hide/Show Cursor
	virtual bool ShowCursor(bool bEnable) = 0;

	/// Retrieve the Window Info
	virtual bool GetWindowInfo(WindowHandle & hHandle) = 0;
	virtual bool GetWindowInfo(WindowHandle & hHandle, int & x, int & y, int & cx, int & cy, bool & full) = 0;

	/// Creates a platform-dependent application class
	static Application * Create();

public:

	/// The graphics renderer
	shared_ptr<Render>			m_spRenderer;

	/// The game engine, the application is responsible for updating per game tick
	shared_ptr<GameEngine>		m_spGameEngine;
};

}; // Katana

#endif // _APPLICATION_H