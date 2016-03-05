/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		dx9texturemanager.cpp
	Author:		Eric Bryant

	Responsible for loading and keeping textures in video/system memory
*/

#include <d3d9.h>
#include <d3dx9.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "base/comptr.h"
#include "rendertypes.h"
#include "texture.h"
#include "dx9texturemanager.h"

//
// Constructor
//
DX9TextureManager::DX9TextureManager() :
	m_uiTextureCount(0),
	m_uiMaximumTextureWidth(0),
	m_uiMaximumTextureHeight(0),
	m_bTextureConstraintToSquare(false),
	m_bTextureConstraintToPow2(false)
{
	// Preallocate the texture dictionary
	IncreaseTextureDictionary( PREALLOCATION_TEXTURE_DICTIONARY_SIZE );
}

//
// Destructor
//
DX9TextureManager::~DX9TextureManager()
{
	Terminate();
}

//
// Initialize
//
bool DX9TextureManager::Initialize( IDirect3DDevice9 * pD3DDevice )
{
	// Store the device
	m_pD3DDevice = pD3DDevice;

	// Grab Device Parameters
	D3DCAPS9 d3dCaps;
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
bool DX9TextureManager::Terminate()
{
	// Clear the texture list while releasing the textures
	ClearAllTextures();

	return true;
}

//
// BindTexture
//
bool DX9TextureManager::BindTexture( Texture * pTexture, IDirect3DBaseTexture9 ** ppTexture )
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
		if ( false == CreateTexture( pTexture ) )
			return false;
	}

	// If the previous operation was successful, or if
	// the texture was already stored in a DX8 Texture
	// retrieve it from the list and return

	unsigned int uiBindID = pTexture->getBindID();
	if ( uiBindID != Texture::INVALID_BIND_ID )
	{
		*ppTexture = m_TextureDictionary[uiBindID];
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
void DX9TextureManager::ClearAllTextures()
{
	// Iterate over the texture dictionary and release the textures
	for( vector< IDirect3DBaseTexture9 * >::iterator iter = m_TextureDictionary.begin();
		 iter != m_TextureDictionary.end();
		 iter++ )
	{
		(*iter)->Release();
	}

	// Now clear the vector
	m_TextureDictionary.clear();
}

//
// IncreaseTextureDictionary
// Increased the space within the texture dictionary for the specified amount
//
bool DX9TextureManager::IncreaseTextureDictionary( unsigned int uiSize )
{
	// This was the old dictionary size
	unsigned int uiOldSize = m_TextureDictionary.size();

	// Reallocate space on the new dictionary
	m_TextureDictionary.reserve( uiSize + uiOldSize );

	return true;
}


//
// CreateTexture
//
bool DX9TextureManager::CreateTexture( Texture * pTexture )
{
	IDirect3DBaseTexture9 * pDX9Texture = 0;

	// If this texture is cube mapped, convert the texture into a cube map
	if ( pTexture->getCubeMap() )
		pDX9Texture = CreateCubeTexture( pTexture );

	// Otherwise create a standard 2D texture
	else pDX9Texture = CreateBaseTexture( pTexture );

	// Did the texture creation fail?
	if ( !pDX9Texture ) return false;

	// Otherwise, generate a bind id for it and store it 
	// within the texture dictionary
	pTexture->setBindID( m_uiTextureCount++ );
	m_TextureDictionary.push_back( pDX9Texture );

	return true;
}

//
// CreateBaseTexture
//
IDirect3DBaseTexture9 * DX9TextureManager::CreateBaseTexture( Texture * pTexture )
{
	// Grab Texture Parameters
	unsigned int uiTextureWidth = pTexture->getWidth();
	unsigned int uiTextureHeight = pTexture->getHeight();
	unsigned int uiTextureBitDepth = pTexture->getBitDepth();
	unsigned int uiMipMaps = pTexture->getFilterMode() != Texture::NEAREST_MIPMAP &&
		pTexture->getFilterMode() != Texture::LINEAR_MIPMAP;

	// Check the Texture's bit depth
	if ( uiTextureBitDepth != 8 && uiTextureBitDepth != 24 && uiTextureBitDepth != 32 )
		return NULL;

	// Resample the Texture if Necessary
	unsigned int uiLogWidth = kmath::log2( uiTextureWidth );
	unsigned int uiLogHeight = kmath::log2( uiTextureHeight );

	if ( ( m_bTextureConstraintToPow2 && 
		( ( uiLogWidth >> uiTextureWidth ) != uiLogWidth ) ||
		( ( uiLogHeight >> uiTextureHeight ) != uiLogHeight ) ) )
	{
		if ( false == ResampleTexture(pTexture, uiTextureWidth, uiTextureHeight) )
			return NULL;
	}

	// Check whether the Texture is Square (and if it has to be)
	if ( m_bTextureConstraintToSquare && ( uiTextureWidth != uiTextureHeight ) )
		return false;

	// Create the texture
	IDirect3DTexture9 * pDX9Texture = 0;
	if( FAILED( m_pD3DDevice->CreateTexture(uiTextureWidth,		// Width
											uiTextureHeight,	// Height
											uiMipMaps,			// Levels
											0,					// Usage
											D3DFMT_A8R8G8B8,	// Format (32 bit w/ alpha)
											D3DPOOL_MANAGED,	// Pool
											&pDX9Texture,		// Resultant texture
											NULL ) ) )			// Reserved
		return NULL;

	// Grab the image data
	unsigned char * pImageData = pTexture->getData();

	D3DLOCKED_RECT d3dlr;
	pDX9Texture->LockRect( 0, &d3dlr, 0, 0 );
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

	pDX9Texture->UnlockRect(0);

	// Generate the Mip Maps
	if ( FAILED( D3DXFilterTexture( pDX9Texture, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) ) )
		return false;

	return pDX9Texture;
}

//
// CreateCubeTexture
//
IDirect3DBaseTexture9 * DX9TextureManager::CreateCubeTexture( Texture * pTexture )
{
	// Grab Texture Parameters
	unsigned int uiTextureWidth = pTexture->getWidth();
	unsigned int uiTextureHeight = pTexture->getHeight();
	unsigned int uiTextureBitDepth = pTexture->getBitDepth();

	// Check the Texture's bit depth
	if ( uiTextureBitDepth != 8 && uiTextureBitDepth != 24 && uiTextureBitDepth != 32 )
		return NULL;

	// Resample the Texture if Necessary
	unsigned int uiLogWidth = kmath::log2( uiTextureWidth );
	unsigned int uiLogHeight = kmath::log2( uiTextureHeight );

	if ( ( m_bTextureConstraintToPow2 && 
		( ( uiLogWidth >> uiTextureWidth ) != uiLogWidth ) ||
		( ( uiLogHeight >> uiTextureHeight ) != uiLogHeight ) ) )
	{
		if ( false == ResampleTexture(pTexture, uiTextureWidth, uiTextureHeight) )
			return NULL;
	}

	// Check whether the Texture is Square (and if it has to be)
	if ( m_bTextureConstraintToSquare && ( uiTextureWidth != uiTextureHeight ) )
		return NULL;

	// Create the texture
	IDirect3DCubeTexture9 * pDX9CubeTexture = 0;
	if ( FAILED( m_pD3DDevice->CreateCubeTexture( uiTextureWidth,	// Edge Level
												  1,				// Levels
												  0,				// Usage
												  D3DFMT_A8R8G8B8,	// Format (32 bit w/ alpha)
												  D3DPOOL_MANAGED,	// Pool
												  &pDX9CubeTexture,	// Resultant Texture
												  NULL ) ) )		// Reserved
		return NULL;

	for( int i = 0; i <= D3DCUBEMAP_FACE_NEGATIVE_Z; i++ )
	{
		D3DLOCKED_RECT d3dlr;
		pDX9CubeTexture->LockRect( (D3DCUBEMAP_FACES)i, 0, &d3dlr, 0, 0 );
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

		pDX9CubeTexture->UnlockRect( (D3DCUBEMAP_FACES)i, 0 );
	}

	return pDX9CubeTexture;
}


//
// ResampleTexture
//
bool DX9TextureManager::ResampleTexture(Texture * pTexture, unsigned int & uiNewWidth, unsigned int & uiNewHeight)
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