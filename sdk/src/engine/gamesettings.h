/*
	Katana Engine
	Copyright © 2001-2003 Eric Bryant, Inc.

	File:		gamesettings.h
	Author:		Eric Bryant

	Structure which holds the initalization settings of the game engine.
*/

#ifndef _GAMESETTINGS_H
#define _GAMESETTINGS_H

namespace Katana
{

//
// Forward declaration
//
class SystemXML;

///
/// GameSettings
///
class GameSettings
{
public:
	/// Constructor with initialized the variables to reasonable defaults:
	/// (640x480x32) resolution and windowed
	GameSettings();

	/// Constructor with loads the game settings from disk. If the operation fails,
	/// it will default to: (640x480x32) resolution and windowed
	GameSettings( const char * szSettingsFile );

	/// Loads the default settings
	bool loadSettings( const char * szSettingsFile );

	/// Save the default settings
	bool saveSettings( const char * szSettingsFile );

public:	
	string					deviceName;				/// Name of the current rendering device
	bool					useReferenceDriver;		/// Use the reference hardware driver
	unsigned int			resolutionWidth;		/// Startup screen resolution width
	unsigned int			resolutionHeight;		/// Startup screen resolution height
	unsigned int			resolutionBitDepth;		/// Startup screen resolution bit depth
	bool					fullscreen;				/// Starting application is rendering full screen
	string					startupScript;			/// Main engine script. It's automatically executed during startup.
	vector<string>			startupScriptHistory;	/// Up to the last 10 selected script files
	bool					displayStartupDialog;	/// Boolean flag which determines whether to display the startup dialog
	string					fontTextureName;		/// Default texture which contains font information
	string					fontCharacterSizes;		/// Data file which stores the font sizes for each character

protected:
	shared_ptr<SystemXML> m_settingsFile;		/// Internal settings file used to load and save the engine configuration
};

} // Katana

#endif // _GAMESETTINGS_H