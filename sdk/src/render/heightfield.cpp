/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		heightfield.cpp
	Author:		Eric Bryant

	Represents heightmap data
*/

#include <math.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "texture.h"
#include "rendertypes.h"
#include "geometry.h"
#include "heightfield.h"

//
// Macro Definitions
//
#define LOOKUP(x,y) m_heightMapData[ ((y * m_dataWidth) + x) ]

//
// RTTI Definition
//
KIMPLEMENT_RTTI( Heightfield, Geometry );

//
// Constants
//
const Point3 Heightfield::INVALID_VERTEX = Point3( -1, -1, -1 );
const Point3 Heightfield::INVALID_NORMAL = Point3( -1, -1, -1 );

//
// Constructor
//
Heightfield::Heightfield() :
	m_heightMapData( NULL ),
	m_heightmapNormals( NULL ),
	m_dataWidth( 0 ),
	m_dataHeight( 0 ),
	m_worldScaleWidth( 1 ),
	m_worldScaleHeight( 1 )
{
}

Heightfield::Heightfield( const char * szHeightMapFile, int worldWidth, int worldHeight ) :
	m_heightMapData( NULL ),
	m_heightmapNormals( NULL ),
	m_dataWidth( 0 ),
	m_dataHeight( 0 ),
	m_worldScaleWidth( 1 ),
	m_worldScaleHeight( 1 )
{
	if ( loadFromTexture( szHeightMapFile ) )
	{
		m_worldScaleWidth = worldWidth / float(m_dataWidth);
		m_worldScaleHeight = worldHeight / float(m_dataHeight);

		computeNormals();
	}
}

Heightfield::~Heightfield()
{
	delete [] m_heightMapData;
	delete [] m_heightmapNormals;
}

bool Heightfield::loadFromTexture( const char * szHeightMapFile )
{
	// Attempt to load the height map file
	Texture heightmap( szHeightMapFile );
	if ( !heightmap.isLoaded() ) return false;

	// Copy the texture data to the height map
	m_heightMapData = new unsigned short[ heightmap.getWidth() * heightmap.getHeight() ];

	// If the pixel format is greyscale, we can do a simple blit
	if ( heightmap.getPixelFormat() == Texture::GREYSCALE )
	{
		// Because we're copying from a texture (whose data type is unsigned char)
		// to a height field data (whose data type is unsigned short), we can't do a memcpy
		for( unsigned int i = 0; i < heightmap.getDataSize(); i++ )
			m_heightMapData[i] = heightmap.getData()[i];
	}
	// If the pixel format is RGB, only use the Red channel
	else if ( heightmap.getPixelFormat() == Texture::RGB )
	{
		for( unsigned int i = 0; i < heightmap.getDataSize(); i+=3 )
			m_heightMapData[i / 3] = heightmap.getData()[i+0]; // Red channel
	}
	// If the pixel format is RGBA, only use the Red channel
	else if ( heightmap.getPixelFormat() == Texture::RGBA )
	{
		for( unsigned int i = 0; i < heightmap.getDataSize(); i+=4 )
			m_heightMapData[i / 4] = heightmap.getData()[i+0]; // Red channel
	}
	// If the pixel format is BGR, only use the Red channel
	else if ( heightmap.getPixelFormat() == Texture::BGR_EXT )
	{
		for( unsigned int i = 0; i < heightmap.getDataSize(); i+=3 )
			m_heightMapData[i / 3] = heightmap.getData()[i+2]; // Red channel
	}
	// Otherwise this is an unknown format.
	else
	{
		// Delete the height map information and return false
		delete [] m_heightMapData;

		return false;
	}

	// Store the image data
	m_dataWidth = heightmap.getWidth();
	m_dataHeight = heightmap.getHeight();

	return true;
}

//
// getHeightAt
//
unsigned short Heightfield::getHeightAt( unsigned int x, unsigned int z ) const
{
	// Check whether this height field has data
	if ( !isValid() ) return INVALID_HEIGHT;

	// Check whether the given (x,y) is within bounds
	if ( ( x >= getWidth() ) || z >= getHeight() ) return INVALID_HEIGHT;

	// Otherwise compute the height map's Y (also flipping the Y axis)
	return m_heightMapData[ x + ( m_dataHeight - 1 - z ) * m_dataWidth ];
}

//
// getSlopeAt
//
float Heightfield::getSlopeAt( unsigned int x, unsigned int z ) const
{
	// Check whether this height field has data
	if ( !isValid() ) return INVALID_SLOPE;

	// Check whether the given (x,y) is within bounds
	if ( ( x >= getWorldWidth() ) || ( z >= getWorldHeight() ) ) return INVALID_SLOPE;

	// Convert position from world coordinate to local texture coordinates
	int iX = int( x / m_worldScaleWidth );
	int iZ = int( z / m_worldScaleHeight );

	// Neighboring pixels
	unsigned short s1, s2, s3, s4, s5, s6, s7, s8;

	s1 = LOOKUP( iX-1, iZ-1 )	- LOOKUP( iX, iZ );
	s2 = LOOKUP( iX-1, iZ )		- LOOKUP( iX, iZ );
	s3 = LOOKUP( iX-1, iZ+1 )	- LOOKUP( iX, iZ );
	s4 = LOOKUP( iX, iZ+1 )		- LOOKUP( iX, iZ );
	s5 = LOOKUP( iX+1, iZ+1 )	- LOOKUP( iX, iZ );
	s6 = LOOKUP( iX+1, iZ )		- LOOKUP( iX, iZ );
	s7 = LOOKUP( iX+1, iZ-1 )	- LOOKUP( iX, iZ );
	s8 = LOOKUP( iX, iZ-1 )		- LOOKUP( iX, iZ );

	return float( (s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8) / (USHRT_MAX * 8) ); // Limit of unsigned short = 65536
}

//
// getVertexAt
//
Point3 Heightfield::getVertexAt( unsigned int x, unsigned int z ) const
{
	// Check whether this height field has data
	if ( !isValid() ) return INVALID_VERTEX;

	// Check whether the given (x,y) is within bounds
	if ( ( x >= getWidth() ) || ( z >= getHeight() ) ) return INVALID_VERTEX;

	return Point3( (float)x, 
				   getHeightAt( x, z ), 
				   (float)z );
}

//
// getNormalAt
//
Point3 Heightfield::getNormalAt( unsigned int x, unsigned int z ) const
{
	// Check whether this height field has data
	if ( !isValid() ) return INVALID_NORMAL;

	// Check whether the given (x,y) is within bounds
	if ( ( x >= getWorldWidth() ) || ( z >= getWorldHeight() ) ) return INVALID_NORMAL;

	if ( !m_heightmapNormals ) 
		return Point3();
	else
		return m_heightmapNormals[ x + z * m_dataWidth ];
}

//
// convertToTriangles
//
int Heightfield::convertToTriangles( float heightScale )
{
	// Setup the geometry parameters
	m_enabledBuffers = VERTEX | INDEX;
	m_primitiveType = TRIANGLE_LIST;

	return 0;
}

//
// bilinearInterpolateHeight
//
unsigned short Heightfield::bilinearInterpolateHeight( float x, float z ) const
{
    unsigned int uiX = (unsigned int) floor(x);
    unsigned int uiY = (unsigned int) floor(z);
    unsigned int uiXP1 = uiX+1, uiYP1 = uiY+1;

    double dDeltaX = x - uiX;
    double dDeltaY = z - uiY;

    unsigned int i00, i01, i10, i11;
    i00 = uiX + uiY * m_dataHeight;
    i01 = uiX + uiYP1 * m_dataHeight;
    i10 = uiXP1 + uiY * m_dataHeight;
    i11 = uiXP1 + uiYP1* m_dataHeight;

    double d00, d01 = 0.0f, d10 = 0.0f, d11 = 0.0f;
    d00 = (1.0 - dDeltaY) * m_heightMapData[i00];
    if ( uiYP1 < m_dataHeight )
        d01 = dDeltaY * m_heightMapData[i01];
    if ( uiXP1 < m_dataWidth )
        d10 = (1.0 - dDeltaY) * m_heightMapData[i10];
    if ( uiXP1 < m_dataWidth && uiYP1 < m_dataHeight )
        d11 = dDeltaY * m_heightMapData[i11];

	return ( (unsigned short) ( (1.0 - dDeltaX) * ( d00 + d01 ) + dDeltaX  * ( d10 + d11 ) ) );
}

//
// computeNormals
//
void Heightfield::computeNormals()
{
	Point3 tmp, v0, v1, v2, v3, v4, v5, v6, v7, v8;
	Point3 f0, f1, f2, f3, f4, f5, f6, f7;

	// Allocate Normal Map Data
	m_heightmapNormals = new Point3[ m_dataHeight * m_dataWidth ];

	for( unsigned int j = 1; j < m_dataHeight - 1; j++)
	{
		for( unsigned int i = 1; i < m_dataWidth - 1; i++)
		{
			v0 = Point3( ( i   ) - m_dataWidth/2, m_heightMapData[ (j  ) * m_dataWidth+i  ], (j  ) - m_dataHeight/2 );
			v1 = Point3( ( i-1 ) - m_dataWidth/2, m_heightMapData[ (j-1) * m_dataWidth+i-1], (j-1) - m_dataHeight/2 );
			v2 = Point3( ( i   ) - m_dataWidth/2, m_heightMapData[ (j-1) * m_dataWidth+i  ], (j-1) - m_dataHeight/2 );
			v3 = Point3( ( i+1 ) - m_dataWidth/2, m_heightMapData[ (j-1) * m_dataWidth+i+1], (j-1) - m_dataHeight/2 );
			v4 = Point3( ( i+1 ) - m_dataWidth/2, m_heightMapData[ (j  ) * m_dataWidth+i+1], (j  ) - m_dataHeight/2 );
			v5 = Point3( ( i+1 ) - m_dataWidth/2, m_heightMapData[ (j+1) * m_dataWidth+i+1], (j+1) - m_dataHeight/2 );
			v6 = Point3( ( i   ) - m_dataWidth/2, m_heightMapData[ (j+1) * m_dataWidth+i  ], (j+1) - m_dataHeight/2 );
			v7 = Point3( ( i-1 ) - m_dataWidth/2, m_heightMapData[ (j+1) * m_dataWidth+i-1], (j+1) - m_dataHeight/2 );
			v8 = Point3( ( i-1 ) - m_dataWidth/2, m_heightMapData[ (j  ) * m_dataWidth+i-1], (j  ) - m_dataHeight/2 );

			f1 = (v0-v2).getCross( v2 - v0 );
			f2 = (v3-v2).getCross( v2 - v0 );
			f3 = (v4-v3).getCross( v3 - v0 );
			f4 = (v5-v4).getCross( v4 - v6 );
			f6 = (v6-v0).getCross( v0 - v7 );
			f7 = (v0-v8).getCross( v8 - v7 );

			tmp = ( f1 + f2 + f3 + f5 + f6 + f7 );
			tmp.getNormalized();
			m_heightmapNormals[ j * m_dataWidth + i  ] = 128.0f * tmp;
		}
	}
}

// ----------------------------------------------------------

//
// OnLoadStream
//
bool Heightfield::OnLoadStream( kistream & istr )
{
	// Call base class
	Geometry::OnLoadStream( istr );

	return true;
}

//
// OnSaveStream
//
bool Heightfield::OnSaveStream( kostream & ostr ) const
{
	// Call base class
	Geometry::OnSaveStream( ostr );

	return true;
}