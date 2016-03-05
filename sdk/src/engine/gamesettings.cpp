/*
	Katana Engine
	Copyright © 2001-2003 Eric Bryant, Inc.

	File:		gamesettings.cpp
	Author:		Eric Bryant

	Structure which holds the initalization settings of the game engine.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "gamesettings.h"
#include "base/kstring.h"
#include "system/systemxml.h"

//
// Constructor
//
GameSettings::GameSettings() 
	: resolutionWidth( 640 )
	, resolutionHeight( 480 )
	, resolutionBitDepth( 16 )
	, fullscreen( false )
	, displayStartupDialog( true )
	, fontTextureName( "./textures/font_arial_10pts.tga" )
{
}

GameSettings::GameSettings( const char * szSettingsFile )
	: resolutionWidth( 640 )
	, resolutionHeight( 480 )
	, resolutionBitDepth( 16 )
	, fullscreen( false )
	, displayStartupDialog( true )
	, fontTextureName( "./textures/font_arial_10pts.tga" )
{
	loadSettings( szSettingsFile );
}

//
// loadSettings
//
bool GameSettings::loadSettings( const char * szSettingsFile )
{
	// Load the settings from the XML File
	m_settingsFile.reset( new SystemXML( szSettingsFile ) );

	if ( !m_settingsFile )
		return false;

	int history = 0;
	int index; kstring nodeid;

	// Initialize the game settings. They have the following XML Schema:
	// <Katana>
	//		<Display></Display>
	//		<Engine></Engine>
	//		<Script></Script>
	// </Katana>

	XML_Node katana = m_settingsFile->getNode( "katana" );

		// DISPLAY
		XML_Node display = katana.getNode( "display" );
			XML_Node adapter = display.getNode( "adapter" );
				deviceName = adapter.getAttributeString( "device" );
			XML_Node reference = display.getNode( "reference_driver" );
				useReferenceDriver = reference.getAttributeBoolean( "enable" );
			XML_Node resolution = display.getNode( "resolution" );
				resolutionWidth = resolution.getAttributeInteger( "width" );
				resolutionHeight = resolution.getAttributeInteger( "height" );
				resolutionBitDepth = resolution.getAttributeInteger( "bitdepth" );
			XML_Node full = display.getNode( "fullscreen" );
				fullscreen = full.getAttributeBoolean( "enable" );

		// ENGINE
		XML_Node engine = katana.getNode( "engine" );
			XML_Node font = engine.getNode( "font" );
				fontTextureName = font.getAttributeString( "texture" );
				fontCharacterSizes = font.getAttributeString( "fontwidths" );

		// SCRIPT
		XML_Node script = katana.getNode( "script" );
			startupScript = script.getAttributeString( "default" );
			history = script.getAttributeInteger( "history" );

			for( index = 1, nodeid = "history-1"; 
				 index <= history; 
				 index++, nodeid.format( "history-%d", index ) )
			{
				string historyValue = script.getNode( nodeid ).getValue();
				if ( historyValue.c_str()[0] != 0 )
					startupScriptHistory.push_back( historyValue.c_str() );
			}

	return true;
}

//
// saveSettings
//
bool GameSettings::saveSettings( const char * szSettingsFile )
{
	unsigned int index; kstring nodeid;

	// Create the root Katana Node
	XML_Node katana( "katana" );
		XML_Node display( katana, "display" );
			XML_Node adapter( display, "adapter" );
				adapter.addAttribute( "device", deviceName );
			XML_Node reference ( display, "reference_driver" );
				reference.addAttribute( "enable", useReferenceDriver );
			XML_Node resolution( display, "resolution" );
				resolution.addAttribute( "width", (long)resolutionWidth );
				resolution.addAttribute( "height", (long)resolutionHeight );
				resolution.addAttribute( "bitdepth", (long)resolutionBitDepth );
			XML_Node full( display, "fullscreen" );
				full.addAttribute( "enable", fullscreen );

		XML_Node engine( katana, "engine" );
			XML_Node font( engine, "font" );
				font.addAttribute( "texture", fontTextureName );
				font.addAttribute( "fontwidths", fontCharacterSizes );

		XML_Node script( katana, "script" );
			script.addAttribute( "default", startupScript );
			script.addAttribute( "history", (long)startupScriptHistory.size() );

			for( index = 0, nodeid = "history-1";
				 index < startupScriptHistory.size();
				 index++, nodeid.format( "history-%d", index+1 ) )
			{
				XML_Node history( script, nodeid );
					history.setValue( startupScriptHistory[index] );
			}

	// Create a new XML File
	shared_ptr<SystemXML> saveFile( new SystemXML( katana ) );

	// Serialize the XML File
	return saveFile->saveXML( szSettingsFile );
}