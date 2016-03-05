/*
	Katana Engine
	Copyright © 2001-2003 Eric Bryant, Inc.

	File:		gamesettings.h
	Author:		Eric Bryant

	Structure which holds the initalization settings of the terrain engine construction
*/

#ifndef _TERRAINSETTINGS_H
#define _TERRAINSETTINGS_H

namespace Katana
{

//
// Forward declaration
//
class SystemXML;

///
/// TerrainSettings
///
class TerrainSettings
{
public:
	enum LODType
	{
		GEOMIPMAP,
	};
	enum BlendType
	{
		BLEND_UNIFIED,
		BLEND_SPLIT,
		BLEND_SW,
		BLEND_HW,
	};
	struct TextureLayer
	{
		TextureLayer( const char * base, const char * bump, int repeat ) :
			baseTexture( base ), bumpTexture( bump ), nRepeat( repeat ) {}

		std::string	baseTexture;
		std::string	bumpTexture;
		int			nRepeat;
	};
public:
	/// Constructor
	TerrainSettings();

	/// Constructor which takes a settings file
	TerrainSettings( const char * szSettingsFile );

	/// Constructor with initialized the variables to reasonable defaults
	TerrainSettings( const char * szHeightMapFile, unsigned int wWidth, unsigned int wHeight );

	/// Loads the default settings
	bool loadSettings( const char * szSettingsFile );

public:	
	std::string					settingsFile;		/// Name of terrain settings file used to construct this class
	std::string					heightMapFileName;	/// Name of the height map file (greyscale texture) to use for terrain initialization.
													/// The texture's size must be 2^n+1, e.g., 257x257, 513x513, ...
	kstring						pvsFileName;		/// Potential visibility set file to use for terrain culling
	unsigned int				worldWidth;			/// The absolute width (X) of the world in world units
	unsigned int				worldHeight;		/// The absolute height (Y) of the world in world units
	unsigned int				worldDepth;			/// The absolute depth (Z) of the world in world units
	LODType						lodType;			/// Lod algorithm to use for the terrain (currently only geomipmapping is supported)
	BlendType					blendType;			/// Determines how to blend between different LOD versions of the terrain
	float						maxScreenError;		/// The tolerance for the screen error when determining terrain lod
	unsigned int				maxTextureLayers;	/// Maximum number of texture passes
	std::vector<TextureLayer>	textureLayers;		/// Texture pass layers

protected:
	shared_ptr<SystemXML> m_settingsFile;	/// Internal settings file used to load and save the terrain configuration
};

} // Katana

#endif // _TERRAINSETTINGS_H