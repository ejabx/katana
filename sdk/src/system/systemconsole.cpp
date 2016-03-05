/*
	Katana Engine
	Copyright © 2003 Eric Bryant, Inc.

	File:		systemconsole.cpp
	Author:		Eric Bryant

	System Console
*/

#include <windows.h>
#include <io.h>
#include <iostream>
#include <fcntl.h>

#include "katana_core_includes.h"
#include "engine/application.h"
#include "script/scriptengine.h"
#include "script/luaconsole.h"
#include "systemconsole.h"

//
// Forward Declarations
//
extern "C" {
	WINBASEAPI HWND APIENTRY GetConsoleWindow( VOID );
}

//
// Externally defined structures
// 
extern shared_ptr<Application>	katana_app;
extern shared_ptr<ScriptEngine>	katana_script;

//
// Global Variables
//
bool g_bStopConsole				= false;
bool g_bOwnConsole				= false;
const int MAX_CONSOLE_BUFFER	= 2048;
CONSOLE_SCREEN_BUFFER_INFO		g_csbiInfo;
HANDLE g_hOut;

//
// Local Functions
//
DWORD WINAPI ConsoleProcedure( LPVOID lpVoid );

//
// Constructor
//
SystemConsole::SystemConsole() :
	m_consoleThread( 0 )
{
}

//
// Destructor
//
SystemConsole::~SystemConsole()
{
	// Allow the console thread to exit normally. This
	// Automatically frees the console (if we created it)
	g_bStopConsole = true;
}

//
// addString
//
void SystemConsole::addString( const char * szMessage )
{
	// Restore the text colors to the original
	if ( g_hOut ) SetConsoleTextAttribute( g_hOut, g_csbiInfo.wAttributes);

	// Output to standard output
	std::cout << szMessage;
}

//
// showConsole
//
bool SystemConsole::showConsole()
{
	// Lazily create the console window if it doesn't exist
	if ( !m_consoleThread )
		if ( !CreateThread( NULL, 0, ConsoleProcedure, NULL, NULL, (LPDWORD)&m_consoleThread ) )
			return false;
	
	// Retrieve the console window and display it
	HWND hConsoleWnd = GetConsoleWindow();
	if ( hConsoleWnd && g_bOwnConsole ) 
		return ShowWindow( hConsoleWnd, TRUE ) ? true : false;

	return false;
}

//
// hideConsole
//
bool SystemConsole::hideConsole()
{
	// Retrieve the console window and hide it
	HWND hConsoleWnd = GetConsoleWindow();
	if ( hConsoleWnd && g_bOwnConsole ) 
		return ShowWindow( hConsoleWnd, FALSE ) ? true : false;

	return false;
}

// -------------------------------------------------------------

//
// ConsoleProcedure
//
DWORD WINAPI ConsoleProcedure( LPVOID lpVoid )
{
	// Check if the console already exists. This may be the case
	// because the application was created as a console application.
	// If it doesn't exist we'll create one.
	if ( !GetConsoleWindow() )
	{
		AllocConsole();
		g_bOwnConsole = true;
	}

	// Get the standard io streams and reroute them to our console
	g_hOut = GetStdHandle( STD_OUTPUT_HANDLE );
	HANDLE hIn = GetStdHandle( STD_INPUT_HANDLE );
	int hCrtOut = _open_osfhandle( (long)g_hOut, _O_TEXT );
	int hCrtIn = _open_osfhandle( (long)hIn, _O_TEXT );
	FILE * hfout = _fdopen( hCrtOut, "w" );
	FILE * hfin = _fdopen( hCrtIn, "r" );
	*stdout = *hfout;
	*stdin = *hfin;

	// Eliminate buffering of the console
	char buf[2];
	setvbuf( hfout, buf, _IONBF, 1 );

	// Save the current console settings. 
	GetConsoleScreenBufferInfo( g_hOut, &g_csbiInfo );

	// Reset the title
	SetConsoleTitle( "K A T A N A   E N G I N E" );

	// Continue until the variable is set to true. This is the
	// case when the SystemConsole destructor is called
	while ( !g_bStopConsole )
	{
		// Set the text to bright cyan
		SetConsoleTextAttribute( g_hOut, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY );

		// Route the command processing to the lua console
		LuaConsole::ProcessCommand();

		// Check for exiting the game engine
		if ( strnicmp( LuaConsole::LAST_SCRIPT_COMMAND.c_str(), "exit", 4 ) == 0 ) katana_app->Terminate();
	}
/*
	// Our input buffer
	char inputBuffer[MAX_CONSOLE_BUFFER];

	// Continue until the variable is set to true. This is the
	// case when the SystemConsole destructor is called
	while ( !g_bStopConsole )
	{
		// Set the text to bright cyan
		SetConsoleTextAttribute( g_hOut, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY );

		// Prompt the user
		std::cout << "> ";
		std::cin.getline( inputBuffer, MAX_CONSOLE_BUFFER );

		// Restore the text colors to the original
		SetConsoleTextAttribute( g_hOut, g_csbiInfo.wAttributes);

		// Check for exitting the game engine
		if ( strcmp( inputBuffer, "exit" ) == 0 )
			katana_app->Terminate();
		else
			katana_script->execBuffer( inputBuffer );
	}
*/
	// If we own the console, destroy it
	if ( g_bOwnConsole ) FreeConsole();

	return 0;
}