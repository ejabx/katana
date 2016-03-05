/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8texturemanager.h
	Author:		Eric Bryant

	Responsible for loading and keeping textures in video/system memory
*/

#ifndef _DX8TEXTUREMANAGER_H
#define _DX8TEXTUREMANAGER_H

//
// Forward Declarations
//
struct IDirect3DBaseTexture8;
struct IDirect3DDevice8;

namespace Katana
{

//
// Forward Declarations
//
class Texture;

///
/// DX8TextureManager
///
class DX8TextureManager
{
public:
	/// Constructor
	DX8TextureManager();

	/// Destructor
	~DX8TextureManager();

	/// Retrieves the number of loaded textures
	unsigned int GetTextureCount() const						{ return m_uiTextureCount; }

	/// Initializes the Texture Manager
	bool Initialize(IDirect3DDevice8 * pD3DDevice);

	/// Terminates the Texture Manager
	bool Terminate();

	/// Retrieves the DX8 Texture associated with the TextureID in Tex
	bool BindTexture(Texture * pTexture, IDirect3DBaseTexture8 ** ppTexture);

private:

	/// Creates a DX8TEXTURE, stores the image data, and pushes it onto m_pTextureList
	bool CreateTexture(Texture * pTexture);

	/// Create a Base Texture
	bool CreateBaseTexture(Texture * pTexture);

	/// Create a Cube Texture
	bool CreateCubeTexture(Texture * pTexture);

	/// Create a Volumetric Texture
	bool CreateVolumeTexture(Texture * pTexture);

	/// Clear Texture List and Release DX8BASETEXTURES
	void ClearAllTextures();

	/// Converts the cube face index to D3D's CubeMap enumeration
	void ConvertCubeFaceToDXFace(unsigned int uiIndex, int & cubeFace);

	/// Resamples a texture
	bool ResampleTexture(Texture * pTexture, unsigned int & uiNewWidth, unsigned int & uiNewHeight);

private:
	/// Maximum allowable textures
	enum { MAX_TEXTURES = 500 };

	/// Mapping of texture ids to DX8 Textures
	ComPtr<IDirect3DBaseTexture8>	m_pTextureList[MAX_TEXTURES];

	/// Pointer to the current DX8 Device (for texture creation)
	ComPtr<IDirect3DDevice8>		m_pD3DDevice;

	/// Current Texture Index (used to compute the next index)
	unsigned int			m_uiTextureCount;

	// Various Device Capabilities
	unsigned int			m_uiMaximumTextureWidth;		/// Maximum Texture Width
	unsigned int			m_uiMaximumTextureHeight;		/// Maximum Texture Height
	bool					m_bTextureConstraintToSquare;	/// Is the Texture constrained to square dimensions?
	bool					m_bTextureConstraintToPow2;		/// Is the Texture constrained to pow2 dimensions
};
}

#endif // _DX8TEXTUREMANAGER_H
