/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		kexport.cpp
	Author:		Eric Bryant

	Interface for exporting files
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "kexport.h"
#include "base/kstring.h"
#include "system/systemxml.h"

//
// Constructor
//
ExportSettings::ExportSettings()
	: exportType( EXPORT_GEOMETRY )
	, selectionOnly( false )
	, computeNormals( true )
	, computeTangents( true )
	, computeColors( false )
	, computeSecondaryTextureCoordinates( false )
	, includeTextures( true )
	, embeddedTextures( false )
	, textureNameOnly( false )
	, exportAnimationOnly( false )
	, includeAnimation( false )
	, stripifyGeometry( false )
	, animStartFrame( 0 )
	, animEndFrame( 0 )
	, computeNumLODs( 0 )
	, exportInWorldSpace( true )
{
}

ExportSettings::ExportSettings( const char * szSettingsFile )
	: exportType( EXPORT_GEOMETRY )
	, selectionOnly( false )
	, computeNormals( true )
	, computeTangents( true )
	, computeColors( false )
	, computeSecondaryTextureCoordinates( false )
	, includeTextures( true )
	, embeddedTextures( false )
	, textureNameOnly( false )
	, exportAnimationOnly( false )
	, includeAnimation( false )
	, stripifyGeometry( false )
	, animStartFrame( 0 )
	, animEndFrame( 0 )
	, computeNumLODs( 0 )
	, exportInWorldSpace( true )
{
	loadSettings( szSettingsFile );
}

//
// loadSettings
//
bool ExportSettings::loadSettings( const char * szSettingsFile )
{
	SystemXML settingsFile( szSettingsFile );

	if ( !settingsFile.isValid() )
		return false;

	// Initialize the export settings.  They have the following XML Schema:
	// <kexport>
	//		<general>
	//		</general>
	//		<geometry>
	//		</geometry>
	//		<bsp>
	//		</bsp>
	//		<biped>
	//		</biped>
	//		<animation>
	//		</animation>
	// </kexport>

	XML_Node kexport = settingsFile.getNode( "kexport" );

		// GENERAL
		XML_Node general = kexport.getNode( "general" );
			exportType = (ExportType)general.getAttributeInteger( "type" );
			selectionOnly = general.getAttributeBoolean( "selectiononly" );
			previewOnly = general.getAttributeBoolean( "previewonly" );

		// GEOMETRY
		XML_Node geometry = kexport.getNode( "geometry" );
			XML_Node textures = geometry.getNode( "textures" );
				includeTextures = textures.getAttributeBoolean( "include" );
				embeddedTextures = textures.getAttributeBoolean( "embed" );
				textureNameOnly = textures.getAttributeBoolean( "nameonly" );
			XML_Node vertex = geometry.getNode( "vertex" );
				computeNormals = vertex.getAttributeBoolean( "normals" );
				computeTangents = vertex.getAttributeBoolean( "tangents" );
				computeColors = vertex.getAttributeBoolean( "colors" );
				computeSecondaryTextureCoordinates = vertex.getAttributeBoolean( "texcoord2" );
			XML_Node animation = geometry.getNode( "animation" );
				includeAnimation = animation.getAttributeBoolean( "include" );
				exportAnimationOnly = animation.getAttributeBoolean( "exportanimonly" );
				animStartFrame = animation.getAttributeInteger( "startframe" );
				animEndFrame = animation.getAttributeInteger( "endframe" );
			XML_Node extras = geometry.getNode( "extras" );
				stripifyGeometry = extras.getAttributeBoolean( "stripify" );
				computeNumLODs = extras.getAttributeInteger( "lods" );
				exportInWorldSpace = extras.getAttributeBoolean( "worldspace" );

	return true;
}

//
// saveSettings
//
bool ExportSettings::saveSettings( const char * szSettingsFile )
{
	// Create the root kexport node
	XML_Node kexport( "kexport" );
		XML_Node general( kexport, "general" );
			general.addAttribute( "type", (long)exportType );
			general.addAttribute( "selectiononly", selectionOnly );
			general.addAttribute( "previewonly", false ); // This is always false

		XML_Node geometry( kexport, "geometry" );
			XML_Node textures( geometry, "textures" );
				textures.addAttribute( "include", includeTextures );
				textures.addAttribute( "embed", embeddedTextures );
				textures.addAttribute( "nameonly", textureNameOnly );
			XML_Node vertex( geometry, "vertex" );
				vertex.addAttribute( "normals", computeNormals );
				vertex.addAttribute( "tangents", computeTangents );
				vertex.addAttribute( "colors", computeColors );
				vertex.addAttribute( "texcoord2", computeSecondaryTextureCoordinates );
			XML_Node animation( geometry, "animation" );
				animation.addAttribute( "include", includeAnimation );
				animation.addAttribute( "exportanimonly", exportAnimationOnly );
				animation.addAttribute( "startframe", (long)animStartFrame );
				animation.addAttribute( "endframe", (long)animEndFrame );
			XML_Node extras( geometry, "extras" );
				extras.addAttribute( "stripify", stripifyGeometry );
				extras.addAttribute( "lods", (long)computeNumLODs );
				extras.addAttribute( "worldspace", exportInWorldSpace );

	// Create a new XML File
	SystemXML saveFile( kexport );

	// Serialize it
	return saveFile.saveXML( szSettingsFile );
}