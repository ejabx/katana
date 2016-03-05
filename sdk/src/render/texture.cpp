/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		texture.cpp
	Author:		Eric Bryant

	A texture resource
*/

#include <math.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "texture.h"
#include "tgaloader.h"
#include "jpegloader.h"
#include "ddsloader.h"

//
// RTTI Information
//
KIMPLEMENT_ROOT_RTTI( Texture );

//
// Constructor
//
Texture::Texture() :
	m_data(0), 
	m_dataSize(0),
	m_loaded(false), 
	m_bindID(INVALID_BIND_ID),
	m_textureStage(0),
	m_textureWidth(0),
	m_textureHeight(0),
	m_bitDepth(0),
	m_mipMaps(1),
	m_cubeMap(false),
	m_pixelFormat(NONE),
	m_filterMode(LINEAR_MIPMAP),
	m_repeatMode(WRAP_ST),
	m_stayResident(false),
	m_delayLoadFailed(false)
{
}

Texture::Texture(const ColorA & rgba) :
	m_dataSize(4),
	m_loaded(true), 
	m_bindID(INVALID_BIND_ID),
	m_textureStage(0),
	m_textureWidth(1),
	m_textureHeight(1),
	m_bitDepth(32),
	m_mipMaps(1),
	m_cubeMap(false),
	m_pixelFormat(RGBA),
	m_filterMode(LINEAR_MIPMAP),
	m_repeatMode(WRAP_ST),
	m_stayResident(false),
	m_delayLoadFailed(false)
{
	m_data = new unsigned char [4];
	m_data[0] = (unsigned char)rgba.a * 255;
	m_data[1] = (unsigned char)rgba.g * 255;
	m_data[2] = (unsigned char)rgba.b * 255;
	m_data[3] = (unsigned char)rgba.r * 255;
}

Texture::Texture(float fR, float fG, float fB, float fA) :
	m_dataSize(4),
	m_loaded(true), 
	m_bindID(INVALID_BIND_ID),
	m_textureStage(0),
	m_textureWidth(1),
	m_textureHeight(1),
	m_bitDepth(32),
	m_mipMaps(1),
	m_cubeMap(false),
	m_pixelFormat(RGBA),
	m_filterMode(LINEAR_MIPMAP),
	m_repeatMode(WRAP_ST),
	m_stayResident(false),
	m_delayLoadFailed(false)
{
	m_data = new unsigned char [4];
	m_data[0] = (unsigned char)fR * 255;
	m_data[1] = (unsigned char)fG * 255;
	m_data[2] = (unsigned char)fB * 255;
	m_data[3] = (unsigned char)fA * 255;
}

Texture::Texture( unsigned char * pData, unsigned int uiWidth, unsigned uiHeight, unsigned int uiBitDepth, PixelFormat format ) :
	m_data(pData), 
	m_dataSize(0),
	m_loaded(true), 
	m_bindID(INVALID_BIND_ID),
	m_textureStage(0),
	m_textureWidth(uiWidth),
	m_textureHeight(uiHeight),
	m_bitDepth(uiBitDepth),
	m_mipMaps(1),
	m_cubeMap(false),
	m_pixelFormat(format),
	m_filterMode(LINEAR_MIPMAP),
	m_repeatMode(WRAP_ST),
	m_stayResident(false),
	m_delayLoadFailed(false)
{
	m_dataSize = uiWidth * uiHeight * ( uiBitDepth == 8 ? 1 : uiBitDepth == 24 ? 3 : 4 );
}

Texture::Texture(const char * szTexturePath) :
	m_data(0), 
	m_dataSize(0),
	m_loaded(false), 
	m_bindID(INVALID_BIND_ID),
	m_textureStage(0),
	m_textureWidth(0),
	m_textureHeight(0),
	m_bitDepth(0),
	m_mipMaps(1),
	m_cubeMap(false),
	m_pixelFormat(NONE),
	m_filterMode(LINEAR_MIPMAP),
	m_repeatMode(WRAP_ST),
	m_stayResident(false),
	m_delayLoadFailed(false)
{
	Load(szTexturePath, *this);
}

Texture::Texture(const char * szTexturePath, FilterMode filter, RepeatMode repeat ) :
	m_data(0), 
	m_dataSize(0),
	m_loaded(false), 
	m_bindID(INVALID_BIND_ID),
	m_textureStage(0),
	m_textureWidth(0),
	m_textureHeight(0),
	m_bitDepth(0),
	m_mipMaps(1),
	m_cubeMap(false),
	m_pixelFormat(NONE),
	m_filterMode(filter),
	m_repeatMode(repeat),
	m_stayResident(false),
	m_delayLoadFailed(false)
{
	Load(szTexturePath, *this);
}

Texture::Texture(const char * szTexturePath, bool bDelayLoad) :
	m_data(0), 
	m_dataSize(0),
	m_loaded(false), 
	m_bindID(INVALID_BIND_ID),
	m_textureStage(0),
	m_textureWidth(0),
	m_textureHeight(0),
	m_bitDepth(0),
	m_mipMaps(1),
	m_cubeMap(false),
	m_pixelFormat(NONE),
	m_filterMode(LINEAR_MIPMAP),
	m_repeatMode(WRAP_ST),
	m_stayResident(false),
	m_delayLoadFailed(false)
{
	if ( false == bDelayLoad )
	{
		Load(szTexturePath, *this);
	}
	else
	{
		m_resourcePath = szTexturePath;
	}
}

//
// Destructor
//
Texture::~Texture()
{
	Unload();
}

//
// Unload
//
bool Texture::Unload()
{
	// Delete local texture memory
	delete [] m_data;
	m_data = 0;
	m_dataSize = 0;
	m_loaded = false;

	return true;
}

//
// delayLoad
//
bool Texture::delayLoad()
{
	// There is no need to load, this method already failed.
	if ( true == m_delayLoadFailed )
		return false;

	if ( m_resourcePath.c_str() && *m_resourcePath.c_str() && false == Load(m_resourcePath.c_str(), *this) )
	{
		m_delayLoadFailed = true;
		return false;
	}

	// We're loaded
	m_loaded = true;

	return true;
}

//
// Create
//
shared_ptr<Texture> Texture::Create(const char * szTextureName)
{
	if ( szTextureName )
	{
		shared_ptr<Texture> spTexture( new Texture );

		if ( Load(szTextureName, *spTexture.get()) )
			return spTexture;
	}

	return shared_ptr<Texture>();
}

shared_ptr<Texture> Texture::Create(unsigned int uiWidth, unsigned int uiHeight, 
									unsigned int uiBitDepth, unsigned char * pImageData,  PixelFormat eFormat, 
									RepeatMode eRepeat, FilterMode eFilter)
{
	shared_ptr<Texture> spTexture( new Texture );

	// If it's a GREYSCALE texture, adjust the bit depth
	if ( eFormat == GREYSCALE )
		uiBitDepth = 8;

	// Compute the RGB Pixel Stride
	const unsigned int uiRGBStride = (eFormat != GREYSCALE)?3:1;

	// Setup Texture Parameters
	spTexture->m_textureWidth = uiWidth;
	spTexture->m_textureHeight = uiHeight;
	spTexture->m_bitDepth = uiBitDepth;
	spTexture->m_dataSize = uiWidth * uiHeight * uiRGBStride;
	spTexture->m_pixelFormat = eFormat;
	spTexture->m_filterMode = eFilter;
	spTexture->m_repeatMode = eRepeat;
	spTexture->m_data = pImageData;

	// The Texture is considered "Loaded"
	spTexture->m_loaded = true;

	return spTexture;
}

//
// Load
//
bool Texture::Load(const char * szTextureName, Texture & texture)
{
	// Strip the file extension from the texture name
	char * szToken = strrchr(szTextureName, '.'); szToken++;

	// Texture Parameters
	PixelFormat pixel;
	unsigned char * data = 0;
	unsigned int width, height, colorDepth, dataSize, mipmaps = 1; 
	bool cubemap = false;

	// Load the image from the given format
	if ( stricmp(szToken, "jpg") == 0 )	// JPEG Format
		data = JpegLoader::Load( szTextureName, width, height, dataSize, colorDepth, pixel);
	if ( stricmp(szToken, "tga") == 0 )	// Targa Format
		data = TgaLoader::Load( szTextureName, width, height, dataSize, colorDepth, pixel );
	if ( stricmp(szToken, "dds") == 0 ) // DDS Format
		data = DDSLoader::Load( szTextureName, width, height, mipmaps, cubemap, dataSize, colorDepth, pixel );

	// If the load was successful, store the texture parameters
	if ( data )
	{
		texture.m_bitDepth = colorDepth;
		texture.m_textureHeight = height;
		texture.m_textureWidth = width;
		texture.m_mipMaps = mipmaps;
		texture.m_cubeMap = cubemap;
		texture.m_dataSize = dataSize;
		texture.m_pixelFormat = pixel;
		texture.m_data = data;
		texture.m_loaded = true;

		// Log the texture load
		KLOG2( "Loaded texture: %s: w:%d h:%d d:%d mip:%d cube:%d",
				szTextureName, texture.m_textureWidth, texture.m_textureHeight, texture.m_bitDepth, texture.m_mipMaps, texture.m_cubeMap 
		);
	}
	else
	{
		// Log the error
		KLOG2( "Unable to load texture: %s", szTextureName );
	}

	return ( data != NULL );
}

//
// Export
//
bool Texture::Export(const char * szTextureName, const Texture & texture)
{
	return false;
}

//
// Resample
//
bool Texture::Resample(unsigned int uiNewWidth, unsigned int uiNewHeight, bool bUseFastInterp)
{
	// Silly, but we still need to check
	if ( ( uiNewWidth == m_textureWidth ) &&
		 ( uiNewHeight == m_textureHeight ) )
		return false;

	if ( m_bitDepth == 8 )
		return Resample8(uiNewWidth, uiNewHeight, bUseFastInterp);

	if ( m_bitDepth == 24 )
		return Resample24(uiNewWidth, uiNewHeight, bUseFastInterp);

	if ( m_bitDepth == 32 )
		return Resample32(uiNewWidth, uiNewHeight, bUseFastInterp);

	return false;
}

//
// Resample8
//
bool Texture::Resample8(unsigned int uiNewWidth, unsigned int uiNewHeight, bool bUseFastInterp)
{
	float fWidthRatio = m_textureWidth / float(uiNewWidth);
	float fHeightRatio = m_textureHeight / float(uiNewHeight);

	// Allocate Space for the New Texture
	unsigned char * pNewData = new unsigned char[uiNewWidth * uiNewHeight * sizeof(unsigned char)];

	// Iterate through the texture, interpolating to the new texture
	for(unsigned int uiCol = 0; uiCol < uiNewHeight; uiCol++)
	{
		for(unsigned int uiRow = 0; uiRow < uiNewWidth; uiRow++)
		{
			float fX = uiRow * fWidthRatio;
			float fZ = uiCol * fHeightRatio;

			unsigned int uiIndex = ((uiNewHeight - uiCol) * uiNewWidth + uiRow);

			// Store in Resultant Texture
			pNewData[uiIndex] = BilinearInterpolateTexture( m_data,
															m_textureWidth,
															fX, fZ,
															0);
		}
	}

	// Delete the Old Bitmap
	delete [] m_data;

	// Setup Texture Parameters
	m_data = pNewData;
	m_dataSize = uiNewWidth * uiNewHeight * sizeof(unsigned char);
	m_textureWidth = uiNewWidth;
	m_textureHeight = uiNewHeight;

	return true;
}

//
// Resample24
//
bool Texture::Resample24(unsigned int uiNewWidth, unsigned int uiNewHeight, bool bUseFastInterp)
{
	float fWidthRatio = m_textureWidth / float(uiNewWidth);
	float fHeightRatio = m_textureHeight / float(uiNewHeight);

	// Allocate Space for the New Texture
	unsigned char * pNewData = new unsigned char[uiNewWidth * uiNewHeight * sizeof(unsigned char) * 3];

	// Iterate through the texture, interpolating to the new texture
	for(unsigned int uiCol = 0; uiCol < uiNewHeight; uiCol++)
	{
		for(unsigned int uiRow = 0; uiRow < uiNewWidth; uiRow++)
		{
			float fX = uiRow * fWidthRatio;
			float fZ = uiCol * fHeightRatio;

			// Interpolate each color separately
			for(char cColor = 0; cColor < 3; cColor++)
			{
				unsigned int uiIndex = ((uiCol) * uiNewWidth + uiRow)*3 + cColor;

				if ( m_pixelFormat == RGB )
				{
					// Store in Resultant Texture
					pNewData[uiIndex] = BilinearInterpolateTexture( m_data,
						m_textureWidth,
						fX, fZ,
						cColor);
				}
				else
				{
					// Store in Resultant Texture
					pNewData[uiIndex] = BilinearInterpolateTexture( m_data,
																	m_textureWidth,
																	fX, fZ,
																	2 - cColor);
				}
			}
		}
	}

	// Delete the Old Bitmap
	delete [] m_data;

	// Setup Texture Parameters
	m_data = pNewData;
	m_dataSize = uiNewWidth * uiNewHeight * sizeof(unsigned char) * 3;
	m_textureWidth = uiNewWidth;
	m_textureHeight = uiNewHeight;

	return true;
}

//
// Resample32
//
bool Texture::Resample32(unsigned int uiNewWidth, unsigned int uiNewHeight, bool bUseFastInterp)
{
	float fWidthRatio = m_textureWidth / float(uiNewWidth);
	float fHeightRatio = m_textureHeight / float(uiNewHeight);

	// Allocate Space for the New Texture
	unsigned char * pNewData = new unsigned char[uiNewWidth * uiNewHeight * sizeof(unsigned char) * 4];

	// Iterate through the texture, interpolating to the new texture
	for(unsigned int uiCol = 0; uiCol < uiNewHeight; uiCol++)
	{
		for(unsigned int uiRow = 0; uiRow < uiNewWidth; uiRow++)
		{
			float fX = uiRow * fWidthRatio;
			float fZ = uiCol * fHeightRatio;

			// Interpolate each color separately
			for(char cColor = 0; cColor < 4; cColor++)
			{
				unsigned int uiIndex = ((uiCol) * uiNewWidth + uiRow)*4 + cColor;

				// Store in Resultant Texture
				pNewData[uiIndex] = BilinearInterpolateTexture( m_data,
																m_textureWidth,
																fX, fZ,
																3 - cColor);
			}
		}
	}

	// Delete the Old Bitmap
	delete [] m_data;

	// Setup Texture Parameters
	m_data = pNewData;
	m_dataSize = uiNewWidth * uiNewHeight * sizeof(unsigned char) * 4;
	m_textureWidth = uiNewWidth;
	m_textureHeight = uiNewHeight;

	return true;
}

//
// BilinearInterpolateTexture
//
unsigned char Texture::BilinearInterpolateTexture(unsigned char * pTextureData, unsigned int uiImageCols,
												  float fX, float fZ, char cColorIndex)
{
    unsigned long ulX = (unsigned long) floor(fX);
    unsigned long ulY = (unsigned long) floor(fZ);
    unsigned char ucValue;

    unsigned long ulXP1 = ulX+1, ulYP1 = ulY+1;
    
    float fDeltaX = fX - ulX;
    float fDeltaY = fZ - ulY;

    unsigned long f00, f01, f10, f11;

    f00 = (ulX + ulY * uiImageCols)*3 + cColorIndex;
    f01 = (ulX + ulYP1 * uiImageCols)*3 + cColorIndex;
    f10 = (ulXP1 + ulY * uiImageCols)*3 + cColorIndex;
    f11 = (ulXP1 + ulYP1 * uiImageCols)*3 + cColorIndex;

    float d00, d01, d10, d11 = 0.0f;
    d00 = (1.0f - fDeltaY) * pTextureData[f00];
    d01 = fDeltaY * pTextureData[f01];
    d10 = (1.0f - fDeltaY) * pTextureData[f10];
    if (ulXP1 < uiImageCols && ulYP1 < uiImageCols)
        d11 = fDeltaY * pTextureData[f11];

    ucValue = (unsigned char) ((1.0 - fDeltaX) * (d00 + d01) +
                                      fDeltaX  * (d10 + d11));

	return ucValue;
}

//
// adjustGamma
// Modify the gamma of the texture by a factor
//
void Texture::adjustGamma( float fFactor )
{
	// This is our pixel data
	unsigned char * pImage = m_data;
	unsigned char colorWidth = 1;
	if ( m_bitDepth == 24 ) 
		colorWidth = 3;
	else
		colorWidth = 4;

	// Go through every pixel in the lightmap
	for(unsigned int i = 0; i < m_dataSize / colorWidth; i++, pImage += colorWidth) 
	{
		float scale = 1.0f, temp = 0.0f;
		float r = 0;

		// extract the current R values
		r = (float)pImage[0];

		// Multiply the factor by the RGB values, while keeping it to a 255 ratio
		r = r * fFactor / 255.0f;

		// Check if the the values went past the highest value
		if(r > 1.0f && (temp = (1.0f/r)) < scale) scale=temp;

		// Get the scale for this pixel and multiply it by our pixel values
		scale*=255.0f;		
		r*=scale;

		// Assign the new gamma'nized RGB values to our image
		pImage[0] = (unsigned char)r;

		// Do the same for the other colors if the bitdepth is greater than 8
		if ( colorWidth > 1 )
		{
			float scale = 1.0f, temp = 0.0f;
			float g = 0, b = 0;

			// extract the current GB values
			g = (float)pImage[1];
			b = (float)pImage[2];

			// Multiply the factor by the GB values, while keeping it to a 255 ratio
			g = g * fFactor / 255.0f;
			b = b * fFactor / 255.0f;

			// Check if the the values went past the highest value
			if(g > 1.0f && (temp = (1.0f/g)) < scale) scale=temp;
			if(b > 1.0f && (temp = (1.0f/b)) < scale) scale=temp;

			// Get the scale for this pixel and multiply it by our pixel values
			scale*=255.0f;		
			g*=scale;	b*=scale;

			// Assign the new gamma'nized GB values to our image
			pImage[1] = (unsigned char)g;
			pImage[2] = (unsigned char)b;
		}
	}
}

// -------------------------------------------------------

//
// OnLoadStream
//
bool Texture::OnLoadStream( kistream & istr )
{
	int bytesLoaded;

	// Load the texture information
	istr >> m_dataSize;
	istr >> m_stayResident;
	istr >> m_textureStage;
	istr >> m_resourcePath;
	istr >> m_textureWidth;
	istr >> m_textureHeight;
	istr >> m_bitDepth;
	istr >> m_mipMaps;
	istr >> (long &)m_pixelFormat;
	istr >> (long &)m_repeatMode;
	istr >> (long &)m_filterMode;

	// Load the raw texture data
	loadAsBlob( istr, (void *&)m_data, bytesLoaded );

	// There was a size decreprancy
	if ( bytesLoaded != m_dataSize ) return false;

	// If we loaded data, initialize the internal variables
	if ( m_dataSize && m_data ) m_loaded = true;

	return true;
}

//
// OnSaveStream
//
bool Texture::OnSaveStream( kostream & ostr ) const
{
	// Save the texture information
	ostr << m_dataSize;
	ostr << m_stayResident;
	ostr << m_textureStage;
	ostr << m_resourcePath;
	ostr << m_textureWidth;
	ostr << m_textureHeight;
	ostr << m_bitDepth;
	ostr << m_mipMaps;
	ostr << m_pixelFormat;
	ostr << m_repeatMode;
	ostr << m_filterMode;

	// Save the raw texture data
	if ( m_loaded ) saveAsBlob( ostr, m_data, m_dataSize );
	else			saveAsBlob( ostr, 0, 0 );

	return true;
}