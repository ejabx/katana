/*
	Katana Engine
	Copyright © 2001-2003 Eric Bryant, Inc.

	File:		gamesettings.h
	Author:		Eric Bryant

	Structure which holds the initalization settings of the terrain engine construction
*/

#include <vector>
#include "../base/refcount.h"
#include "../base/kstring.h"
#include "../system/systemxml.h"
#include "terrainsettings.h"
using namespace Katana;

//
// Local Functions
//
TerrainSettings::BlendType convertStringToBlendType( const kstring & str );

//
// Constructor
//
TerrainSettings::TerrainSettings()
{
}

TerrainSettings::TerrainSettings( const char * szSettingsFile ) :
	settingsFile( szSettingsFile )
{
	loadSettings( szSettingsFile );
}

TerrainSettings::TerrainSettings( const char * szHeightMapFile, unsigned int wWidth, unsigned int wHeight  ) :
	worldWidth( wWidth ), worldHeight( wHeight )
{
	heightMapFileName = szHeightMapFile;
}

//
// loadSettings
//
bool TerrainSettings::loadSettings( const char * szSettingsFile )
{
	// Load the settings from the XML File
	m_settingsFile = new SystemXML( szSettingsFile );

	if ( !m_settingsFile.isValid() )
		return false;

	// Initialize the terrain settings
	XML_Node terrain = m_settingsFile->getNode( "terrain" );
		XML_Node map = terrain.getNode( "map" );
			XML_Node source = map.getNode( "source" );
				heightMapFileName = source.getAttributeString( "file" );
			XML_Node size = map.getNode( "size" );
				worldWidth = size.getAttributeInteger( "width" );
				worldHeight = size.getAttributeInteger( "height" );
				worldDepth = size.getAttributeInteger( "depth" );
			XML_Node lod = map.getNode( "lod" );
				lodType = GEOMIPMAP; // TODO: This is the only lod type supported
				blendType = convertStringToBlendType( lod.getAttributeString( "blend" ) );
				maxScreenError  = lod.getAttributeFloat( "error" );
			XML_Node pvs = map.getNode( "pvs" );
				pvsFileName = pvs.getAttributeString( "file" );
		XML_Node textures = terrain.getNode( "textures" );
			maxTextureLayers = textures.getAttributeInteger( "layers" );

			int index; kstring nodeid;
			for( index = 1, nodeid = "layer-1";
				 index <= maxTextureLayers;
				 index++, nodeid.format( "layer-%d", index ) )
			{
				XML_Node layer = textures.getNode( nodeid );
				textureLayers.push_back( TerrainSettings::TextureLayer( layer.getAttributeString( "file" ),
																		layer.getAttributeString( "bump" ),
																		layer.getAttributeInteger( "repeat" ) ) );
			}

	return true;
}

//
//
//
TerrainSettings::BlendType convertStringToBlendType( const kstring & str )
{
	if ( str == "UNIFIED" )
		return TerrainSettings::BLEND_UNIFIED;
	if ( str == "SPLIT" )
		return TerrainSettings::BLEND_SPLIT;
	if ( str == "BLEND_SW" )
		return TerrainSettings::BLEND_SW;
	if ( str == "BLEND_HW" )
		return TerrainSettings::BLEND_HW;

	return TerrainSettings::BLEND_UNIFIED;
}