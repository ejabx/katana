/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		dx9texturemanager.h
	Author:		Eric Bryant

	Responsible for loading and keeping textures in video/system memory
*/

#ifndef _DX9TEXTUREMANAGER_H
#define _DX9TEXTUREMANAGER_H

//
// Forward Declarations
//
struct IDirect3DBaseTexture8;
struct IDirect3DDevice8;

namespace Katana
{

// Forward Declarations
class Texture;

///
/// DX9TextureManager
///
class DX9TextureManager
{
public:
	/// Constructor
	DX9TextureManager();

	/// Destructor
	~DX9TextureManager();

	/// Initializes the Texture Manager
	bool Initialize( IDirect3DDevice9 * pD3DDevice );

	/// Terminates the Texture Manager
	bool Terminate();

	/// Retrieves the DX8 Texture associated with the TextureID in Tex
	bool BindTexture( Texture * pTexture, IDirect3DBaseTexture9 ** ppTexture );

private:

	/// Increased the space within the texture dictionary for the specified amount
	bool IncreaseTextureDictionary( unsigned int uiSize );

	/// Creates a DX9TEXTURE, stores the image data, and pushes it onto m_pTextureList
	bool CreateTexture( Texture * pTexture );

	/// Create a Base Texture
	IDirect3DBaseTexture9 * CreateBaseTexture( Texture * pTexture );

	/// Create a Cube Texture
	IDirect3DBaseTexture9 * CreateCubeTexture( Texture * pTexture );

	/// Clear Texture List and Release DX9BASETEXTURES
	void ClearAllTextures();

	/// Resamples a texture
	bool ResampleTexture( Texture * pTexture, unsigned int & uiNewWidth, unsigned int & uiNewHeight );

private:
	/// We preallocate space in the texutre dictionary using this value.
	/// Also, if we run out of space in the texture dictionary, we'll increase it by
	/// this amount
	enum { PREALLOCATION_TEXTURE_DICTIONARY_SIZE = 500 };

	/// Mapping of Texture Bind IDS to their concrete DX9 Textures
	/// Note they are sequencial
	vector< IDirect3DBaseTexture9 * >	m_TextureDictionary;

	/// Pointer to the current DX8 Device (for texture creation)
	ComPtr<IDirect3DDevice9>			m_pD3DDevice;

	/// Current Texture Index (used to compute the next bind index)
	unsigned int						m_uiTextureCount;

	// Various Device Capabilities
	unsigned int			m_uiMaximumTextureWidth;		/// Maximum Texture Width
	unsigned int			m_uiMaximumTextureHeight;		/// Maximum Texture Height
	bool					m_bTextureConstraintToSquare;	/// Is the Texture constrained to square dimensions?
	bool					m_bTextureConstraintToPow2;		/// Is the Texture constrained to pow2 dimensions
};

} // Katana

#endif // _DX9TEXTUREMANAGER_H