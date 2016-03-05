/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8texturemanager.cpp
	Author:		Eric Bryant

	Responsible for loading and keeping textures in video/system memory
*/

#include <d3d8.h>
#include <d3dx8.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "base/comptr.h"
#include "rendertypes.h"
#include "texture.h"
#include "dx8texturemanager.h"

//
// Constructor
//
DX8TextureManager::DX8TextureManager() :
	m_uiTextureCount(0),
	m_uiMaximumTextureWidth(0),
	m_uiMaximumTextureHeight(0),
	m_bTextureConstraintToSquare(false),
	m_bTextureConstraintToPow2(false)
{
	// Clear TextureList Memory
	memset(m_pTextureList, 0, sizeof(IDirect3DTexture8 *) * MAX_TEXTURES);
}

//
// Destructor
//
DX8TextureManager::~DX8TextureManager()
{
	Terminate();
}

//
// Initialize
//
bool DX8TextureManager::Initialize(IDirect3DDevice8 * pD3DDevice)
{
	// Assertions
	m_pD3DDevice = pD3DDevice;

	// Grab Device Parameters
	D3DCAPS8 d3dCaps;
	pD3DDevice->GetDeviceCaps(&d3dCaps);

	m_uiMaximumTextureWidth = d3dCaps.MaxTextureWidth;
	m_uiMaximumTextureHeight = d3dCaps.MaxTextureHeight;
	m_bTextureConstraintToSquare = (d3dCaps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) == D3DPTEXTURECAPS_SQUAREONLY;
	m_bTextureConstraintToPow2 = (d3dCaps.TextureCaps & D3DPTEXTURECAPS_POW2) == D3DPTEXTURECAPS_POW2;

	// First Terimate the Manager to clear texture and other resources
	Terminate();

	return true;
}

//
// Terminate
//
bool DX8TextureManager::Terminate()
{
	// Clear the texture list while releasing the textures
	ClearAllTextures();

	return true;
}

//
// BindTexture
//
bool DX8TextureManager::BindTexture(Texture * pTexture, IDirect3DBaseTexture8 ** ppTexture)
{
	// Check if the texture is loaded
	if ( false == pTexture->isLoaded() )
	{
		// Delay Loading, try and load the texture from the resource.
		// NOTE: If this method fails once, it will always fail for this texture (so we
		//		 don't try and load the texture for every BindTexture())

		if ( false == pTexture->delayLoad() )
			return false;
	}

	// Have we stored the texture in a DX8 Texture
	if ( pTexture->getBindID() == Texture::INVALID_BIND_ID )
	{
		if ( false == CreateTexture(pTexture) )
			return false;
	}

	// If the previous operation was successful, or if
	// the texture was already stored in a DX8 Texture
	// retrieve it from the list and return

	unsigned int uiBindID = pTexture->getBindID();
	if ( uiBindID != Texture::INVALID_BIND_ID )
	{
		*ppTexture = m_pTextureList[uiBindID];
		return true;
	}

	// If the texture shouldn't stay resident, unload it
	if ( false == pTexture->getStayResident() )
		pTexture->Unload();

	return true;
}

//
// ClearAllTextures
//
void DX8TextureManager::ClearAllTextures()
{
	for(unsigned int uiIndex = 0; uiIndex < MAX_TEXTURES; uiIndex++)
	{
		// Release the texture if necessary
		if ( m_pTextureList[uiIndex] != NULL )
			m_pTextureList[uiIndex]->Release();

		// Set it to NULL
		m_pTextureList[uiIndex] = NULL;
	}
}

//
// CreateTexture
//
bool DX8TextureManager::CreateTexture(Texture * pTexture)
{
	if ( !pTexture->getCubeMap() )
		return CreateBaseTexture(pTexture);
	else
		return CreateCubeTexture(pTexture);

	// Unknown Texture
	return false;
}

//
// CreateBaseTexture
//
bool DX8TextureManager::CreateBaseTexture(Texture * pTexture)
{
	// Grab Texture Parameters
	unsigned int uiTextureWidth = pTexture->getWidth();
	unsigned int uiTextureHeight = pTexture->getHeight();
	unsigned int uiTextureBitDepth = pTexture->getBitDepth();
	unsigned int uiMipMaps = pTexture->getFilterMode() != Texture::NEAREST_MIPMAP &&
							 pTexture->getFilterMode() != Texture::LINEAR_MIPMAP;

	// Check the Texture's bit depth
	if ( uiTextureBitDepth != 8 && uiTextureBitDepth != 24 && uiTextureBitDepth != 32 )
		return false;

	// Resample the Texture if Necessary
	unsigned int uiLogWidth = kmath::log2( uiTextureWidth );
	unsigned int uiLogHeight = kmath::log2( uiTextureHeight );

	if ( ( m_bTextureConstraintToPow2 && 
		 ( ( uiLogWidth >> uiTextureWidth ) != uiLogWidth ) ||
		 ( ( uiLogHeight >> uiTextureHeight ) != uiLogHeight ) ) )
	{
		 if ( false == ResampleTexture(pTexture, uiTextureWidth, uiTextureHeight) )
			 return false;
	}

	// Check whether the Texture is Square (and if it has to be)
	if ( m_bTextureConstraintToSquare && ( uiTextureWidth != uiTextureHeight ) )
		return false;

	// Create the texture
	HRESULT hr;
	IDirect3DTexture8 * pDX8Texture = 0;
	hr = m_pD3DDevice->CreateTexture( uiTextureWidth, uiTextureHeight, uiMipMaps, 0, 
									  D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pDX8Texture );

	if ( FAILED(hr) )
		return false;

	// Grab the image data
	unsigned char * pImageData = pTexture->getData();

    D3DLOCKED_RECT d3dlr;
    pDX8Texture->LockRect( 0, &d3dlr, 0, 0 );
	DWORD * pDst = (DWORD *)d3dlr.pBits;
	unsigned char a = 255; // fully opaque
	unsigned char r,g,b;

	if ( uiTextureBitDepth == 32 )
	{
		for(unsigned int i = 0; i < uiTextureWidth * uiTextureHeight; i++)
		{
			r = pImageData[(i*4)+0];
			g = pImageData[(i*4)+1];
			b = pImageData[(i*4)+2];
			a = pImageData[(i*4)+3];

			if ( pTexture->getPixelFormat() == Texture::RGB || 
				 pTexture->getPixelFormat() == Texture::RGBA )
				pDst[i] = (a << 24 | r << 16 | g << 8 | b );
			else
				pDst[i] = (a << 24 | b << 16 | g << 8 | r );
		}
	}

	if ( uiTextureBitDepth == 24 )
	{
		for(unsigned int i = 0; i < uiTextureWidth * uiTextureHeight; i++)
		{
			r = pImageData[(i*3)+0];
			g = pImageData[(i*3)+1];
			b = pImageData[(i*3)+2];

			if ( pTexture->getPixelFormat() == Texture::RGB )
				pDst[i] = (a << 24 | r << 16 | g << 8 | b );
			else
				pDst[i] = (a << 24 | b << 16 | g << 8 | r );
		}
	}

	if ( uiTextureBitDepth == 8 )
	{
		for(unsigned int i = 0; i < uiTextureWidth * uiTextureHeight; i++)
			pDst[i] = pImageData[i];
	}

	pDX8Texture->UnlockRect(0);

	// Generate the Mip Maps
	hr = D3DXFilterTexture(pDX8Texture, NULL, D3DX_DEFAULT, D3DX_DEFAULT);
	if ( FAILED(hr) )
		return false;

	// Push the texture onto the buffer and store this index in the Texture
	pTexture->setBindID(m_uiTextureCount);
	m_pTextureList[m_uiTextureCount++] = pDX8Texture;

	return true;
}

//
// CreateCubeTexture
//
bool DX8TextureManager::CreateCubeTexture(Texture * pTexture)
{
	// Grab Texture Parameters
	unsigned int uiTextureWidth = pTexture->getWidth();
	unsigned int uiTextureHeight = pTexture->getHeight();
	unsigned int uiTextureBitDepth = pTexture->getBitDepth();

	// Check the Texture's bit depth
	if ( uiTextureBitDepth != 8 && uiTextureBitDepth != 24 && uiTextureBitDepth != 32 )
		return false;

	// Resample the Texture if Necessary
	unsigned int uiLogWidth = kmath::log2( uiTextureWidth );
	unsigned int uiLogHeight = kmath::log2( uiTextureHeight );

	if ( ( m_bTextureConstraintToPow2 && 
		 ( ( uiLogWidth >> uiTextureWidth ) != uiLogWidth ) ||
		 ( ( uiLogHeight >> uiTextureHeight ) != uiLogHeight ) ) )
	{
		 if ( false == ResampleTexture(pTexture, uiTextureWidth, uiTextureHeight) )
			 return false;
	}

	// Check whether the Texture is Square (and if it has to be)
	if ( m_bTextureConstraintToSquare && ( uiTextureWidth != uiTextureHeight ) )
		return false;

	// Create the texture
	HRESULT hr;
	IDirect3DCubeTexture8 * pDX8CubeTexture = 0;
	hr = m_pD3DDevice->CreateCubeTexture( uiTextureWidth, 1, 0, 
										  D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pDX8CubeTexture );

	if ( FAILED(hr) )
		return false;

	for( int i = 0; i <= D3DCUBEMAP_FACE_NEGATIVE_Z; i++ )
	{
		D3DLOCKED_RECT d3dlr;
		pDX8CubeTexture->LockRect( (D3DCUBEMAP_FACES)i, 0, &d3dlr, 0, 0 );
		DWORD * pDst = (DWORD *)d3dlr.pBits;
		unsigned char a = 255; // fully opaque
		unsigned char r,g,b;

		// Grab the image data
		unsigned char * pImageData = pTexture->getData();
		pImageData += (i * (uiTextureWidth * uiTextureHeight * 4));

		if ( uiTextureBitDepth == 32 )
		{
			for(unsigned int i = 0; i < uiTextureWidth * uiTextureHeight; i++)
			{
				r = pImageData[(i*4)+0];
				g = pImageData[(i*4)+1];
				b = pImageData[(i*4)+2];
				a = pImageData[(i*4)+3];

				if ( pTexture->getPixelFormat() == Texture::RGB || 
					pTexture->getPixelFormat() == Texture::RGBA )
					pDst[i] = (a << 24 | r << 16 | g << 8 | b );
				else
					pDst[i] = (a << 24 | b << 16 | g << 8 | r );
			}
		}

		if ( uiTextureBitDepth == 24 )
		{
			for(unsigned int i = 0; i < uiTextureWidth * uiTextureHeight; i++)
			{
				r = pImageData[(i*3)+0];
				g = pImageData[(i*3)+1];
				b = pImageData[(i*3)+2];

				if ( pTexture->getPixelFormat() == Texture::RGB )
					pDst[i] = (a << 24 | r << 16 | g << 8 | b );
				else
					pDst[i] = (a << 24 | b << 16 | g << 8 | r );
			}
		}

		if ( uiTextureBitDepth == 8 )
		{
			for(unsigned int i = 0; i < uiTextureWidth * uiTextureHeight; i++)
				pDst[i] = pImageData[i];
		}

		pDX8CubeTexture->UnlockRect( (D3DCUBEMAP_FACES)i, 0 );
	}

	// Push the texture onto the buffer and store this index in the Texture
	pTexture->setBindID(m_uiTextureCount);
	m_pTextureList[m_uiTextureCount++] = pDX8CubeTexture;

	return true;
}


//
// ResampleTexture
//
bool DX8TextureManager::ResampleTexture(Texture * pTexture, unsigned int & uiNewWidth, unsigned int & uiNewHeight)
{
	unsigned int uiTextureWidth = pTexture->getWidth();
	unsigned int uiTextureHeight = pTexture->getHeight();

	uiNewWidth = kmath::nextPowerOf2( uiTextureWidth );
	uiNewHeight = kmath::nextPowerOf2( uiTextureHeight );

	if ( uiNewWidth > m_uiMaximumTextureWidth )
		return false;
	if ( uiNewHeight > m_uiMaximumTextureHeight )
		return false;

	return pTexture->Resample(uiNewWidth, uiNewHeight, true);
}
