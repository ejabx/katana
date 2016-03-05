/*
	Katana Engine
	Copyright © 2001-2003 Eric Bryant, Inc.

	File:		terrainpatch.h
	Author:		Eric Bryant

	Terrain Patch.
*/

#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <math.h>
#include <assert.h>
#include "../base/kbase.h"
#include "../base/rtti.h"
#include "../base/refcount.h"
#include "../base/streamable.h"
#include "../base/karray.h"
#include "../math/kmath.h"
#include "../math/point.h"
#include "../math/matrix.h"
#include "../math/bound.h"
#include "../math/quaternion.h"
#include "../script/scriptengine.h"
#include "../render/geometry.h"
#include "../render/vertexbuffer.h"
#include "../render/heightfield.h"
#include "visible.h"
#include "visnode.h"
#include "terrain.h"
#include "terrainpatch.h"
using namespace Katana;

//
// Local Functions
//
void reduceTesselation(int tesselation, TerrainVertex * pData );

//
// Constructor
//
TerrainPatch::TerrainPatch() :
	m_parentTerrain( NULL ),
	m_gridPositionX( 0 ),
	m_gridPositionZ( 0 ),
	m_leftPatch( NULL ),
	m_rightPatch( NULL ),
	m_bottomPatch( NULL ),
	m_topPatch( NULL ),
	m_worldScale( Point3( 1, 1, 1 ) ),
	m_active( true ),
	m_heightMapX( 0 ),
	m_heightMapZ( 0 ),
	m_initBuffers( false ),
	m_currentError( -1 ),
	m_minHeightY( 0 ),
	m_newVertexCount( 0 ),
	m_newIndexCount( 0 )
{
	m_realTesselation = m_newRealTesselation = m_newTesselation = m_tesselation =
		m_leftTesslation = m_rightTesselation = m_leftTesslation = m_topTesselation = m_bottomTesselation = 
		m_oldRealTesselation = MAXIMUM_SUBDIVISION + 5;

	m_patchVertices.reserve( MAXIMUM_VERTICES );
	m_patchIndices.reserve( MAXIMUM_INDICES );
	m_patchIndexMap.reserve( MAXIMUM_VERTICES );
}

TerrainPatch::TerrainPatch( Terrain * parentTerrain, unsigned int indexX, unsigned int indexZ ) :
	m_parentTerrain( parentTerrain ),
	m_gridPositionX( indexX ),
	m_gridPositionZ( indexZ ),
	m_leftPatch( NULL ),
	m_rightPatch( NULL ),
	m_bottomPatch( NULL ),
	m_topPatch( NULL ),
	m_worldScale( Point3( 1, 1, 1 ) ),
	m_active( true ),
	m_heightMapX( 0 ),
	m_heightMapZ( 0 ),
	m_initBuffers( false ),
	m_currentError( -1 ),
	m_minHeightY( 0 ),
	m_newVertexCount( 0 ),
	m_newIndexCount( 0 )
{
	m_realTesselation = m_newRealTesselation = m_newTesselation = m_tesselation =
		m_leftTesslation = m_rightTesselation = m_leftTesslation = m_topTesselation = m_bottomTesselation = 
		m_oldRealTesselation = MAXIMUM_SUBDIVISION + 5;

	m_patchVertices.resize( MAXIMUM_VERTICES );
	m_patchIndices.resize( MAXIMUM_INDICES );
	m_patchIndexMap.reserve( MAXIMUM_VERTICES );
}

//
// setHeightMap
//
void TerrainPatch::setHeightMap( Heightfield & heightmap, unsigned int posX, unsigned posZ )
{
	m_heightmap = &heightmap;
	m_heightMapX = posX;
	m_heightMapZ = posZ;
}

//
// calculateErrors
//
void TerrainPatch::calculateErrors()
{
	for( int i=0; i <= MAXIMUM_SUBDIVISION ; i++ )
		calculateError( i );
}

//
// calculateMinMaxY
//
void TerrainPatch::calculateMinMaxY()
{
	for( int px = 0; px < PATCH_VERTEX_WIDTH; px++ )
	{
		for( int pz = 0; pz < PATCH_VERTEX_HEIGHT; pz++ )
		{
			Point3 vertex = getVertex( px, pz );
			if ( vertex.y < m_minHeightY ) m_minHeightY = vertex.y;
			if ( vertex.y > m_maxHeightY ) m_maxHeightY = vertex.y;
		}
	}
}

//
// setNeighbors
//
void TerrainPatch::setNeighbors( TerrainPatch * left, TerrainPatch * right, TerrainPatch * bottom, TerrainPatch * top )
{
	m_leftPatch = left;
	m_rightPatch = right;
	m_bottomPatch = bottom;
	m_topPatch = top;
}

//
// getVertex
//
Point3 TerrainPatch::getVertex( int positionX, int positionZ )
{
	// Check whether we have a valid heightmap for the operation
	if ( !m_heightmap.isValid() ) return Point3();

	return Point3(	(float)positionX,
					(float)m_heightmap->getHeightAt( m_heightMapX + positionX, m_heightMapZ + positionZ ),
					(float)positionZ );
}

//
// calculateError
//
void TerrainPatch::calculateError( int tesselation )
{
	float	sumError = 0.f;
	int		numErrors = 0;

	if ( tesselation )
	{
		int power = kmath::powerOf2( tesselation );
		int x0, y0, x1, y1;

		for( y0 = 0; y0 < PATCH_VERTEX_HEIGHT - power; y0 += power )
		{
			for( x0 = 0; x0 < PATCH_VERTEX_WIDTH - power; x0 += power )
			{
				for( y1 = 1; y1 < power; y1++ )
				{
					for( x1 = 1; x1 < power; x1++ )
					{
						int x = x0 + x1,
							y = y0 + y1;
						float fx0 = (float) x1 / (float)power, fx1 = 1.0f - fx0,
							  fy0 = (float) y1 / (float)power, fy1 = 1.0f - fy0;
						float height00 = getHeight( x0, y0 ),
							  height10 = getHeight( x0 + power, y0 ),
							  height01 = getHeight( x0, y0 + power ),
							  height11 = getHeight( x0 + power, y0 + power );
						float paintHeight = fx1 * fy1 * height00 +
											fx0 * fy1 * height10 +
											fx1 * fy0 * height01 +
											fx0 * fy0 * height11;
						float correctHeight = getHeight( x, y );
						float error = (float)fabs( correctHeight - paintHeight );

						numErrors++;
						sumError += error;
					}
				}
			}
		}

		float error = sumError / numErrors;
		m_errors[tesselation].correctVal = m_heightmap->getVertexAt( unsigned int( m_worldScale.x * m_worldTranslation.x + ( PATCH_VERTEX_WIDTH / 2) ), 
																	 unsigned int( m_worldScale.z * m_worldTranslation.z + ( PATCH_VERTEX_HEIGHT / 2 ) ) );
		m_errors[tesselation].realVal = m_errors[tesselation].correctVal;
		m_errors[tesselation].realVal.y += error;
		m_errors[tesselation].difference = error;
	}
}

//
// updateProjectedErrors
//
void TerrainPatch::updateProjectedErrors( const Matrix4 & screenProjection )
{
	Point3 correct, real;

	// Compute the raw screen projected error (from the heightmap)
	correct = m_errors[1].correctVal;
	correct *= screenProjection;

	// Clamp if z is less than 0
	if ( correct .z < 0.f )
		correct .z = 0.f;

	// For each subdivision, determine the real projected screen error
	// (which is determined by averaging the neighboring heighmap values)
	for( int i = 1; i <= MAXIMUM_SUBDIVISION; i++ )
	{
		// Compute the real screen projected error
		real = m_errors[i].realVal;
		real *= screenProjection;

		// Clamp if z is less than 0
		if ( real.z < 0.f )
			real.z = 0.f;

		// Store the difference between the correct and real values
		Point3 diff( correct.x - real.x, correct.y - real.y, correct.z - real.z );
		m_errors[i].difference = diff.getLength();

		// If the differance is within the tolerance of the last error, replace it
		if ( m_errors[i].difference < m_errors[i-1].difference * 1.1f )
			m_errors[i].difference = m_errors[i-1].difference * 1.1f;
	}
}

//
// getNewTesselationSafe
//
int TerrainPatch::getNewTesselationSafe() const
{
	if ( this == NULL ) 
		return MAXIMUM_SUBDIVISION;
	else if ( !isVisible() )
		return m_tesselation;
	else
		return m_newTesselation;
}

//
// updateTesselation
//
bool TerrainPatch::updateTesselation()
{
	bool tesselation = false;

	// Determine the tesselation levels of the adjacent patches
	int leftTes		= m_leftPatch->getNewTesselationSafe(),
		rightTes	= m_rightPatch->getNewTesselationSafe(),
		topTes		= m_topPatch->getNewTesselationSafe(),
		bottomTes	= m_bottomPatch->getNewTesselationSafe();

	// Should we create the vertex and index buffers?
	if ( !m_initBuffers ||								// If the buffers hasn't been initalized yet, definitely
		 m_realTesselation != m_newRealTesselation ||	// OR our new real tesselation doesn't match the old
		 m_tesselation != m_newTesselation ||			// OR our current tesselation doesn't match the new
		 m_leftTesslation != leftTes ||
		 m_rightTesselation != rightTes ||
		 m_topTesselation != topTes ||
		 m_bottomTesselation != bottomTes )				// OR our neighbor's tesselation has changed
	{
		m_leftTesslation = leftTes;
		m_rightTesselation = rightTes;
		m_topTesselation = topTes;
		m_bottomTesselation = bottomTes;
		m_tesselation = m_newTesselation;
		m_realTesselation = m_newRealTesselation;

		// Create the tesselation triangles
		createTesselation( m_tesselation, m_leftTesslation, m_rightTesselation, m_topTesselation, m_bottomTesselation );

		m_initBuffers = tesselation = true;
	}

	// Recalculate the current screen error for this patch
	float error0 = m_errors[ m_realTesselation ].difference,
		  error1 = m_errors[ m_realTesselation+1].difference,
		  maxErr = m_parentTerrain->getMaximumScreenError();

	// Check whether the error is past the maximum number of subdivisions
	// and scale the error accordingly
	if ( m_realTesselation >= MAXIMUM_SUBDIVISION )
		error1 = 2.0f * error0;

	// Calculate the new error
	m_currentError = ( maxErr - error0 ) / ( error1 - error0 );

	// Clamp the error to 1
	if ( m_realTesselation >= 4 && m_currentError> 1.f ) m_currentError = 1.f;

	// Return if tesselation occured
	return tesselation;
}

// -------------------------------------------------------
// TESSELATION METHODS
// -------------------------------------------------------

//
// createTesselation
//
void TerrainPatch::createTesselation( int center, int left, int right, int top, int bottom )
{
	// Fills the index map
	m_patchIndexMap.reserve( MAXIMUM_VERTICES );
	memset( &m_patchIndexMap[0], 0xFF, MAXIMUM_VERTICES * sizeof(short) );

	// Reset the new number of vertices and indices
	m_newVertexCount = m_newIndexCount = 0;

	// Gets the next power of two of this patch's tesselation
	int power = kmath::powerOf2( center );

	// Flags whether the adjacent patches have less tesselation than the current
	bool bLeft( left < center ),
		 bRight( right < center ),
		 bTop( top < center ),
		 bBottom( bottom < center );

	// If our tesselation is at the lowest ( 2^4 == 16 ), create a basic quad
	if ( power == 16 ) 
		addTrianglePatch( bLeft, bRight, bBottom, bTop );
	else
		// Otherwise, sequentially add triangles rows for the patch height
		for( int y = 0; y < PATCH_VERTEX_HEIGHT - power; y+=power )
			addTriangleRow( y, power, bLeft, bRight, bBottom, bTop );

	// If the real tesselation is greater than the current tesselation
	// reduce the shape of the patch
	if ( m_realTesselation > m_tesselation )
		reduceTesselation( m_realTesselation, &m_patchVertices[0] );


	// Reduce the borders of the patch to accomodate the tesselation of
	// the neighboring patches
	reduceBorders( &m_patchVertices[0] );
}

//
// reduceTesselation
// Calls makeSimpler() sequentially to make a mesh with m_tesselation < tesselation
// to look like being created with tesselation
//
void TerrainPatch::reduceTesselation(int tesselation, TerrainVertex * pData )
{
	for( int t = tesselation; t > 0 ; t-- )
		if( tesselation < t )
			makeSimpler( t, pData );
}

//
// reduceBorders
//
void TerrainPatch::reduceBorders( TerrainVertex * pData )
{
	int sT = m_tesselation;
	int x, y, idx, idx0, idx1, power,
		lDiff = sT - m_leftTesslation,
		rDiff = sT - m_rightTesselation,
		bDiff = sT - m_bottomTesselation,
		tDiff = sT - m_topTesselation;

	if( ( lDiff == 1 || lDiff == -1) && m_leftPatch )
	{
		power = kmath::powerOf2( std::min( m_leftTesslation, sT ) );

		for( y = power; y < PATCH_VERTEX_HEIGHT - 1; y += 2*power )
		{
			idx = m_patchIndexMap[  0 + y * PATCH_VERTEX_WIDTH ];
			idx0 = m_patchIndexMap[ 0 + (y - power) * PATCH_VERTEX_WIDTH ];
			idx1 = m_patchIndexMap[ 0 + (y + power) * PATCH_VERTEX_WIDTH ];

			assert(idx!=0xffff);
			assert(idx0!=0xffff);
			assert(idx1!=0xffff);

			pData[ idx ].position.y = ( pData[ idx0 ].position.y + pData[idx1].position.y ) / 2.0f;
		}
	}

	if ( ( rDiff ==1  ||  rDiff == -1 ) && m_rightPatch )
	{
		power = kmath::powerOf2( std::min( m_rightTesselation, sT ) );

		for( y = power; y< PATCH_VERTEX_HEIGHT - 1; y += 2*power )
		{
			idx = m_patchIndexMap[PATCH_VERTEX_WIDTH-1 + y*PATCH_VERTEX_WIDTH];
			idx0 = m_patchIndexMap[PATCH_VERTEX_WIDTH-1 + (y-power)*PATCH_VERTEX_WIDTH];
			idx1 = m_patchIndexMap[PATCH_VERTEX_WIDTH-1 + (y+power)*PATCH_VERTEX_WIDTH];

			assert(idx!=0xffff);
			assert(idx0!=0xffff);
			assert(idx1!=0xffff);

			pData[ idx ].position.y = ( pData[ idx0 ].position.y + pData[idx1].position.y ) / 2.0f;
		}
	}

	if( ( bDiff == 1 || bDiff == -1 ) && m_bottomPatch )
	{
		power = kmath::powerOf2( std::min( m_bottomTesselation, sT ) );

		for( x = power; x < PATCH_VERTEX_WIDTH-1; x += 2*power )
		{
			idx = m_patchIndexMap[x + 0*PATCH_VERTEX_WIDTH];
			idx0 = m_patchIndexMap[x-power + 0*PATCH_VERTEX_WIDTH];
			idx1 = m_patchIndexMap[x+power + 0*PATCH_VERTEX_WIDTH];

			assert(idx!=0xffff);
			assert(idx0!=0xffff);
			assert(idx1!=0xffff);

			pData[ idx ].position.y = ( pData[ idx0 ].position.y + pData[idx1].position.y ) / 2.0f;
		}
	}

	if( ( tDiff == 1 || tDiff == -1) && m_topPatch )
	{
		power= kmath::powerOf2( std::min( m_topTesselation, sT) );

		for( x = power; x < PATCH_VERTEX_WIDTH - 1 ; x += 2*power )
		{
			idx = m_patchIndexMap[x + (PATCH_VERTEX_HEIGHT-1)*PATCH_VERTEX_WIDTH];
			idx0 = m_patchIndexMap[x-power + (PATCH_VERTEX_HEIGHT-1)*PATCH_VERTEX_WIDTH];
			idx1 = m_patchIndexMap[x+power + (PATCH_VERTEX_HEIGHT-1)*PATCH_VERTEX_WIDTH];

			assert(idx!=0xffff);
			assert(idx0!=0xffff);
			assert(idx1!=0xffff);

			pData[ idx ].position.y = ( pData[ idx0 ].position.y + pData[idx1].position.y ) / 2.0f;
		}
	}
}


//
// makeSimpler
// this methods makes the mesh looking simpler by putting vertices into
// position inbetween its neighbors this method can do only for one step at a time
// (to make a tes_1 mesh look like tes_3 first call makeSimpler(3,...) then makeSimpler(2,...)
// Call reduceShapeTo() which cares about those constraints
// automatically
//
void TerrainPatch::makeSimpler( int tesselation, TerrainVertex * pData )
{
	assert(m_tesselation<tesselation);
	assert(tesselation>0);

	int power = kmath::powerOf2( tesselation - 1 ), x, y, xc, yc;

	for( yc = y = 0; y < PATCH_VERTEX_HEIGHT; y += power,yc++)
		for( xc = x = 0; x < PATCH_VERTEX_HEIGHT; x += power,xc++)
			calculateInbetweenVertex(x, y, xc, yc, power, pData);

}

//
// calculateInbetweenVertex
//
void TerrainPatch::calculateInbetweenVertex(int x, int y, int xc, int yc, int power, TerrainVertex * pData)
{
	float	heightLeftTop = 0.0f, heightRightBottom = 0.0f;
	int		idx, idxLT,idxRB;

	idx = calculateDiagonalVertices( x, y, xc, yc, power, pData, heightLeftTop, idxLT, heightRightBottom, idxRB );

	if ( idx >= 0 ) pData[idx].position.y = (heightLeftTop + heightRightBottom) / 2.0f;
}

//
// calculateDiagonalVertices
//
int TerrainPatch::calculateDiagonalVertices(int x, int y, int xc, int yc, int power, TerrainVertex * pData, float& nHeightLeftTop, int& idxLT, float& nHeightRightBottom, int& idxRB)
{
	int	c = (xc&1) + (yc&1)*2,
		idx = m_patchIndexMap[x + y*PATCH_VERTEX_WIDTH];

	assert(idx!=0xffff);

	switch( c )
	{
		case 0:			// even-x & even-y
			// nothing to do...
			return -1;

		case 1:			// odd-x & even-y
			assert( x > 0 && x < PATCH_VERTEX_WIDTH - 1 );

			idxLT = m_patchIndexMap[ x - power + y * PATCH_VERTEX_WIDTH ];
			idxRB = m_patchIndexMap[ x + power + y * PATCH_VERTEX_WIDTH ];

			assert(idxLT!=0xffff);
			assert(idxRB!=0xffff);

			nHeightLeftTop = pData[idxLT].position.y;
			nHeightRightBottom = pData[idxRB].position.y;

			return idx;

		case 2:			// even-x & odd-y
			assert( y > 0 && y < PATCH_VERTEX_HEIGHT - 1 );

			idxLT = m_patchIndexMap[ x + ( y + power ) * PATCH_VERTEX_WIDTH ];
			idxRB = m_patchIndexMap[ x + ( y - power ) * PATCH_VERTEX_WIDTH ];

			assert(idxLT!=0xffff);
			assert(idxRB!=0xffff);

			nHeightLeftTop = pData[idxLT].position.y;
			nHeightRightBottom = pData[idxRB].position.y;
			return idx;

		case 3:			// odd-x & odd-y
			assert( x > 0 && x < PATCH_VERTEX_WIDTH - 1 );
			assert( y > 0 && y < PATCH_VERTEX_HEIGHT - 1 );

			idxLT = m_patchIndexMap[ x - power + ( y + power ) * PATCH_VERTEX_WIDTH ];
			idxRB = m_patchIndexMap[ x + power + ( y - power ) * PATCH_VERTEX_WIDTH ];

			assert(idxLT!=0xffff);
			assert(idxRB!=0xffff);

			nHeightLeftTop = pData[idxLT].position.y;
			nHeightRightBottom = pData[idxRB].position.y;
			return idx;

		default:
			assert(false);	// should never come here...
			return -1;
	}

	return 0;
}

//
// addTriangleRow
//
void TerrainPatch::addTriangleRow(int nZ, int power, bool nLeft, bool nRight, bool nBottom, bool nTop)
{
	int powerH = power / 2;

	// special case: bottom line with nBottom
	//
	if( nZ == 0 && nBottom )
	{
		addTriangleBottomRow( power, nLeft, nRight );
		return;
	}

	if( nZ != 0 )
	{
		addLastIndexAgain();
		addIndex( getIndex ( 0, nZ ) );
	}

	// special case: top line with nTop
	//
	if( nZ >= PATCH_VERTEX_HEIGHT - 2 * power && nTop )
	{
		addTriangleTopRow( power, nLeft, nRight );
		return;
	}

	// all other cases
	//
	int x, x0 = 0, x1 = PATCH_VERTEX_WIDTH;

	if( nLeft )
	{
		assert(powerH>=1);

		addIndex( getIndex( 0, nZ) );
		addIndex( getIndex( 0, nZ + powerH ) );
		addIndex( getIndex( power, nZ ) );
		addIndex( getIndex( 0, nZ + power ) );
		addIndex( getIndex( power, nZ + power ) );
		addLastIndexAgain();							// finalize sub-strip
		x0 = power;
	}

	if( nRight )
		x1 -= power;

	for(x = x0 ; x < x1; x += power)
	{
		addIndex( getIndex( x, nZ ) );
		addIndex( getIndex( x, nZ + power ) );
	}

	if( nRight )
	{
		assert(powerH>=1);

		addIndex( getIndex( PATCH_VERTEX_WIDTH - 1 - power, nZ ) );
		addLastIndexAgain();							// reverse oriented triangles
		addIndex( getIndex( PATCH_VERTEX_WIDTH - 1, nZ ) );
		addIndex( getIndex( PATCH_VERTEX_WIDTH - 1 - power, nZ + power ) );
		addIndex( getIndex( PATCH_VERTEX_WIDTH -1, nZ + powerH ) );
		addIndex( getIndex( PATCH_VERTEX_WIDTH -1, nZ + power ) );
	}
}

//
// addTriangleBottomRow
//
void TerrainPatch::addTriangleBottomRow(int power, bool nLeft, bool nRight)
{
	int powerH = power/2;
	int x0 = 0, x1 = PATCH_VERTEX_WIDTH - 1;

	assert( powerH >= 1 );

	if( nLeft )
	{
		addIndex( getIndex( 0, 0 ) );
		addIndex( getIndex( 0, powerH ) );
		addIndex( getIndex( powerH, 0 ) );
		addIndex( getIndex( 0, power ) );
		addIndex( getIndex( power, 0 ) );
		addIndex( getIndex( power, power ) );
		x0 = power;
	}

	assert( ( m_newIndexCount & 1 ) == 0 );
	
	if ( nRight )
		x1 -= power;

	for( int x = x0 ; x < x1; x += power )
	{
		addIndex( getIndex( x,0 ) );
		addLastIndexAgain();						// reverse oriented triangles
		addIndex( getIndex( x + powerH, 0 ) );
		addIndex( getIndex( x, power ) );
		addIndex( getIndex( x + power, 0 ) );
		addIndex( getIndex( x + power, power ) );
	}

	assert( ( m_newIndexCount & 1 ) == 0 );

	if(nRight)
	{
		addIndex( getIndex( PATCH_VERTEX_WIDTH - 1 - power, 0 ) );
		addIndex( getIndex( PATCH_VERTEX_WIDTH - 1 - power, power ) );
		addIndex( getIndex( PATCH_VERTEX_WIDTH - 1 - powerH, 0 ) );
		addIndex( getIndex( PATCH_VERTEX_WIDTH - 1, 0 ) );
		addLastIndexAgain();							// finalize sub-strip
		addIndex( getIndex( PATCH_VERTEX_WIDTH - 1 - power, power ) );
		addIndex( getIndex( PATCH_VERTEX_WIDTH - 1, powerH ) );
		addIndex( getIndex( PATCH_VERTEX_WIDTH - 1, power ) );
	}

	assert( ( m_newIndexCount & 1 ) == 0 );
}

//
// addTriangleTopRow
//
void TerrainPatch::addTriangleTopRow(int power, bool nLeft, bool nRight)
{
	int powerH = power / 2;
	int x0 = 0, x1 = PATCH_VERTEX_WIDTH - 1;

	assert( powerH >= 1 );

	if ( nLeft )
	{
		addIndex( getIndex( 0, PATCH_VERTEX_HEIGHT - 1 - power ) );
		addIndex( getIndex( 0, PATCH_VERTEX_HEIGHT - 1 - powerH ) );
		addIndex( getIndex( power, PATCH_VERTEX_HEIGHT - 1 - power ) );
		addIndex( getIndex( 0, PATCH_VERTEX_HEIGHT - 1 ) );
		addIndex( getIndex( powerH, PATCH_VERTEX_HEIGHT - 1 ) );
		addLastIndexAgain();						// reverse oriented triangles
		addIndex( getIndex( power, PATCH_VERTEX_HEIGHT - 1 - power ) );
		addIndex( getIndex( power, PATCH_VERTEX_HEIGHT - 1 ) );
		x0 = power;
	}

	assert( ( m_newIndexCount & 1 ) == 0 );

	if ( nRight )
		x1 -= power;

	int x;
	for( x = x0; x < x1; x += power )
	{
		addIndex( getIndex( x, PATCH_VERTEX_HEIGHT - 1 - power ) );
		addIndex( getIndex( x, PATCH_VERTEX_HEIGHT - 1 ) );
		addIndex( getIndex( x + power, PATCH_VERTEX_HEIGHT - 1 - power ) );
		addIndex( getIndex( x + powerH, PATCH_VERTEX_HEIGHT - 1 ) );
		addIndex( getIndex( x + power, PATCH_VERTEX_HEIGHT - 1 ) );
		addLastIndexAgain();						// reverse oriented triangles
	}

	if(nRight)
	{
		addIndex( getIndex( x, PATCH_VERTEX_HEIGHT - 1 - power ) );
		addIndex( getIndex( x, PATCH_VERTEX_HEIGHT - 1 ) );
		addIndex( getIndex( x + power, PATCH_VERTEX_HEIGHT - 1 - power ) );
		addIndex( getIndex( x + powerH, PATCH_VERTEX_HEIGHT - 1 ) );
		addIndex( getIndex( x + power, PATCH_VERTEX_HEIGHT - 1 - powerH ) );
		addIndex( getIndex( x + power, PATCH_VERTEX_HEIGHT - 1 ) );
	}

}

//
// addTrianglePatch
//
void TerrainPatch::addTrianglePatch(bool nLeft, bool nRight, bool nBottom, bool nTop)
{
	const int	powerH = 8, power = 16;
	int			code = (nLeft ? 1 : 0) | (nRight ? 2 : 0) | (nBottom ? 4 : 0) | (nTop ? 8 : 0);

	// use the created code for fast selection of the case to build...
	//
	switch ( code )
	{
		case 0:
			addIndex( getIndex( 0, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power ,0 ) );
			addIndex( getIndex( power, power ) );
			break;
		case 1:
			addIndex( getIndex( 0, 0 ) );
			addIndex( getIndex( 0, powerH ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power, power ) );
			addLastIndexAgain();
			break;
		case 2:
			addIndex( getIndex( power ,power ) );
			addIndex( getIndex( power ,powerH ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( 0, 0 ) );
			addLastIndexAgain();
			break;
		case 3:
			addIndex( getIndex( 0, 0 ) );
			addIndex( getIndex( 0, powerH ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power, powerH ) );
			addIndex( getIndex( power, power ) );
			break;
		case 4:
			addIndex( getIndex( 0, 0 ) );
			addLastIndexAgain();
			addIndex(getIndex(powerH, 0 ) );
			addIndex(getIndex(0, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( power, power ) );
			break;

		case 5:
			addIndex( getIndex( 0, 0 ) );
			addIndex( getIndex( 0, powerH ) );
			addIndex( getIndex( powerH, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( power, power ) );
			break;

		case 6:
			addIndex( getIndex( 0, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( powerH, 0 ) );
			addIndex( getIndex( power, 0 ) );
			addLastIndexAgain();
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power, powerH ) );
			addIndex( getIndex( power, power ) );
			break;

		case 7:
			addIndex( getIndex( power, power ) );
			addIndex( getIndex( power, powerH ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( 0, powerH ) );
			addIndex( getIndex( powerH, 0) );
			addIndex( getIndex( 0, 0));
			addLastIndexAgain();
			break;

		case 8:
			addIndex( getIndex( power, power ) );
			addLastIndexAgain();
			addIndex( getIndex( powerH, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( 0, 0 ) );
			break;

		case 9:
			addIndex( getIndex( power, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( powerH, power ) );
			addIndex( getIndex( 0, power ) );
			addLastIndexAgain();
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( 0, powerH ) );
			addIndex( getIndex( 0, 0 ) );
			break;

		case 10:
			addIndex( getIndex( power, power ) );
			addIndex( getIndex( power, powerH ) );
			addIndex( getIndex( powerH, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( 0, 0 ) );
			break;

		case 11:
			addIndex( getIndex( 0, 0 ) );
			addIndex( getIndex( 0, powerH ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power, powerH ) );
			addIndex( getIndex( powerH, power ) );
			addIndex( getIndex( power, power ) );
			addLastIndexAgain();
			break;

		case 12:
			addIndex( getIndex( 0, 0 ) );
			addLastIndexAgain();
			addIndex( getIndex( powerH, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( powerH, power ) );
			addIndex( getIndex( power, power ) );
			addLastIndexAgain();
			break;

		case 13:
			addIndex( getIndex( 0, 0 ) );
			addIndex( getIndex( 0, powerH ) );
			addIndex( getIndex( powerH, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( powerH, power ) );
			addIndex( getIndex( power, power ) );
			addLastIndexAgain();
			break;

		case 14:
			addIndex( getIndex( power, power ) );
			addIndex( getIndex( power, powerH ) );
			addIndex( getIndex( powerH, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( powerH, 0 ) );
			addIndex( getIndex( 0, 0 ) );
			addLastIndexAgain();
			break;

		case 15:
			addIndex( getIndex( 0, 0 ) );
			addIndex( getIndex( 0, powerH ) );
			addIndex( getIndex( powerH, 0 ) );
			addIndex( getIndex( 0, power ) );
			addIndex( getIndex( power, 0 ) );
			addIndex( getIndex( powerH, power ) );
			addIndex( getIndex( power, powerH ) );
			addIndex( getIndex( power, power ) );
			break;
	}

	assert( ( m_newIndexCount & 1 ) == 0 );		// indices must always be an even number (we store quads as two triangles)
}

//
// getHeight
//
float TerrainPatch::getHeight( int nX, int nZ ) const
{
	assert( nX >= 0 && nX < PATCH_VERTEX_WIDTH );
	assert( nZ >= 0 && nZ < PATCH_VERTEX_HEIGHT );
	assert( m_heightmap.isValid() );

	return m_worldTranslation.y + m_worldScale.y * 
			m_heightmap->getHeightAt( 
				m_heightMapX + nX, m_heightMapZ + nZ ) / 255.f; // - scale.y*0.5f;
}

//
// getVertex
//
void TerrainPatch::getVertex( int nX, int nZ, Point3 & point ) const
{
	assert( nX >= 0 && nX < PATCH_VERTEX_WIDTH );
	assert( nZ >= 0 && nZ < PATCH_VERTEX_HEIGHT );

	point.x = m_worldTranslation.x + m_worldScale.x * nX / ( float )( PATCH_VERTEX_WIDTH - 1 );
	point.y = getHeight( nX, nZ );
	point.z = m_worldTranslation.z + m_worldScale.z * nZ / ( float )( PATCH_VERTEX_HEIGHT - 1 );
}

//
// getTexCoord
//
void TerrainPatch::getTexCoord( int nX, int nZ, Point2 & tcoord ) const
{
	assert( nX >= 0 && nX < PATCH_VERTEX_WIDTH );
	assert( nZ >= 0 && nZ < PATCH_VERTEX_HEIGHT );
	assert( m_heightmap.isValid() );

	tcoord.x = (float) ( m_heightMapX + nX ) / (float)( m_heightmap->getWidth()- 1 );	
	tcoord.y = (float) m_heightmap->getHeight() - 1 - ( m_heightMapZ + nZ ) / (float)( m_heightmap->getHeight() - 1 );
}

//
// getNormal
//
void TerrainPatch::getNormal(int nX, int nZ, Point3 & normal) const
{
	assert( nX >= 0 && nX < PATCH_VERTEX_WIDTH );
	assert( nZ >= 0 && nZ < PATCH_VERTEX_HEIGHT );
	assert( m_heightmap.isValid() );

	normal = m_heightmap->getNormalAt( m_heightMapX + nX, m_heightMapZ + nZ );
}

//
// getIndex
//
unsigned short TerrainPatch::getIndex(int nX, int nZ)
{
	int idx = nX + nZ * PATCH_VERTEX_WIDTH;

	assert( nX >= 0 && nX < PATCH_VERTEX_WIDTH );
	assert( nZ >= 0 && nZ < PATCH_VERTEX_WIDTH );
	assert( idx < MAXIMUM_VERTICES );

	if ( m_patchIndexMap[idx] == unsigned short( 0xFFFF ) ) // unsigned short(0xFFFF) = -1
	{
		m_patchIndexMap[idx] = (unsigned short)m_newVertexCount;

		getVertex( nX, nZ, m_patchVertices[m_newVertexCount].position );
		getTexCoord( nX, nZ, m_patchVertices[m_newVertexCount].texture );

		m_newVertexCount++;
	}

	return m_patchIndexMap[idx];
}

//
// addIndex
//
inline
void TerrainPatch::addIndex( unsigned short nIdx )
{ 
	m_patchIndices[m_newIndexCount++] = nIdx;
}

//
// addLastIndexAgain
//
inline 
void TerrainPatch::addLastIndexAgain()
{ 
	if ( m_newIndexCount > 0 ) 
		m_patchIndices[m_newIndexCount++] = m_patchIndices[m_newIndexCount - 1]; 
}

// -------------------------------------------------------


//
// OnLoadStream
//
bool TerrainPatch::OnLoadStream( kistream & istr )
{
	return true;
}

//
// OnSaveStream
//
bool TerrainPatch::OnSaveStream( kostream & ostr ) const
{
	return true;
}