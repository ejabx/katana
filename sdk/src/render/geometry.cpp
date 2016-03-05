/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		geometry.cpp
	Author:		Eric Bryant

	Represents any geometric primitive within the game. It is
	a collection of per vertex data (texture coordinates, color, 
	normal, and tangent basis), which is organized by some primitive
	type (triangle strip, fan, etc.).
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "rendertypes.h"
#include "geometry.h"
#include "vertexbuffer.h"
#include <math.h>

// --------------------------------------------------------
// Macros
// --------------------------------------------------------

#define CHECK_FLAG(var, flag) (var&flag) == flag

// ----------------------------------------------------------
// Geometry
// ----------------------------------------------------------

//
// RTTI Definition
//
KIMPLEMENT_ROOT_RTTI( Geometry );

// ----------------------------------------------------------

//
// Constructor
//
Geometry::Geometry() 
	: m_primitiveType( TRIANGLE_LIST )
	, m_enabledBuffers( VERTEX )
	, m_primitiveCount( 0 )
	, m_vertexCount( 0 )
	, m_indexCount( 0 )
	, m_triangleStripCount( 0 )
{
}

//
// OnLoadStream
//
bool Geometry::OnLoadStream( kistream & istr )
{
	// Load the geometry information
	istr >> (long &)m_primitiveType;
	istr >> m_primitiveCount;
	istr >> m_vertexCount;
	istr >> m_indexCount;
	istr >> m_triangleStripCount;
	istr >> m_enabledBuffers;

	// Load the geometry array data
	istr >> m_vertexBuffer;
	istr >> m_normalBuffer;
	istr >> m_texture0Buffer;
	istr >> m_texture1Buffer;
	istr >> m_colorBuffer;
	istr >> m_indexBuffer;
	istr >> m_tangentBasisS;
	istr >> m_tangentBasisT;
	istr >> m_tangentBasisST;

	return true;
}

//
// OnSaveStream
//
bool Geometry::OnSaveStream( kostream & ostr ) const
{
	// Save the geometry information
	ostr << m_primitiveType;
	ostr << m_primitiveCount;
	ostr << m_vertexCount;
	ostr << m_indexCount;
	ostr << m_triangleStripCount;
	ostr << m_enabledBuffers;

	// Save the geometry array data
	ostr << m_vertexBuffer;
	ostr << m_normalBuffer;
	ostr << m_texture0Buffer;
	ostr << m_texture1Buffer;
	ostr << m_colorBuffer;
	ostr << m_indexBuffer;
	ostr << m_tangentBasisS;
	ostr << m_tangentBasisT;
	ostr << m_tangentBasisST;

	return true;
}

// ----------------------------------------------------------
// Geometry Static Functions
// ----------------------------------------------------------

// 
// createBox
//
// Static function which finds the minimum axis-aligned box which contains the set
// of points in the Geometry.
//
void Geometry::createBox( shared_ptr<Geometry> spGeometry, Point3 & vMin, Point3 & vMax )
{
	if ( spGeometry && spGeometry->m_vertexBuffer && spGeometry->m_vertexBuffer->size() )
	{
		// Reinterpret out vertex buffer points (which are floats) to Point3s
		const Point3 * pPoints = reinterpret_cast<const Point3 *>( &spGeometry->m_vertexBuffer->front() );
		const unsigned int uiPointCount = spGeometry->m_vertexBuffer->size() / 3;

		// Setup the initial min and max
		vMin = pPoints[0];
		vMax = vMin;

		// Iterate over the points and compute the min and max
		for ( unsigned int uiIndex = 1; uiIndex < uiPointCount; uiIndex++ )
		{
			if ( pPoints[uiIndex].x < vMin.x )
				vMin.x = pPoints[uiIndex].x;
			else if ( pPoints[uiIndex].x > vMax.x )
				vMax.x = pPoints[uiIndex].x;

			if ( pPoints[uiIndex].y < vMin.y )
				vMin.y = pPoints[uiIndex].y;
			else if ( pPoints[uiIndex].y > vMax.y )
				vMax.y = pPoints[uiIndex].y;

			if ( pPoints[uiIndex].z < vMin.z )
				vMin.z = pPoints[uiIndex].z;
			else if ( pPoints[uiIndex].z > vMax.z )
				vMax.z = pPoints[uiIndex].z;
		}
	}
}

//
// createSphere
//
// Static function which finds the sphere containing the points in the Geometry using 
// average of the values.
//
void Geometry::createSphere( shared_ptr<Geometry> spGeometry, Point3 & vCenter, float & fRadius )
{
	if ( spGeometry && spGeometry->m_vertexBuffer && spGeometry->m_vertexBuffer->size() )
	{
		// Reinterpret out vertex buffer points (which are floats) to Point3s
		const Point3 * pPoints = reinterpret_cast<const Point3 *>( &spGeometry->m_vertexBuffer->front() );
		const unsigned int uiPointCount = spGeometry->m_vertexBuffer->size() / 3;

		// Compute the Average of Points
		vCenter = pPoints[0];

		for( unsigned uiIndex = 1; uiIndex < uiPointCount; uiIndex++ )
			vCenter += pPoints[uiIndex];

		float fInvQuantity = 1.f / uiPointCount;
		vCenter *= fInvQuantity;

		// Compute the Smallest Radius from Pt to Average
		float fRadiusSqr = 0.f, fMaxRadiusSqr = 0.f;
		Point3 ptDiff;

		for(uiIndex = 0; uiIndex < uiPointCount; uiIndex++)
		{
			ptDiff = pPoints[uiIndex] - vCenter;
			fRadiusSqr = ptDiff.getSqrLength();
			if ( fRadiusSqr > fMaxRadiusSqr )
				fMaxRadiusSqr = fRadiusSqr;
		}

		// Store the radius
		fRadius = sqrt( fMaxRadiusSqr );
	}
}

//
// createCylinder
//
// Static function which finds cylinder containing the pointer in the Geometry
//
void Geometry::createCylinder( shared_ptr<Geometry> spGeometry, Point3 & vCenter, Point3 & vDirection, float & fHeight, float & fRadius )
{
	if ( spGeometry && spGeometry->m_vertexBuffer && spGeometry->m_vertexBuffer->size() )
	{
		// Reinterpret out vertex buffer points (which are floats) to Point3s
		const Point3 * pPoints = reinterpret_cast<const Point3 *>( &spGeometry->m_vertexBuffer->front() );
		const unsigned int uiPointCount = spGeometry->m_vertexBuffer->size() / 3;

		// Calculate the Cylinder

		// TODO: Given the points in the cylinder, determine the best axis.
		//		 Because the math is involed, we'll just assume the cylinder initially points upwards (positive Y)
		Point3 vLineOrigin, vLineDirection;
		vLineOrigin = Point3( 0, 0, 0 ); vLineDirection = Point3( 0, 1, 0 );
//		kmath::orthogonalLineFit( uiPointCount, pPoints, vLineOrigin, vLineDiretion );

		// Calculate the maximum radius of the cylinder
		float fMaxRadiusSqr = 0.f;
		for ( unsigned int i = 0; i < uiPointCount; i++)
		{
			// Compute the squared distance between the current point and the cylinder axis
			Point3 vDiff = pPoints[i] - vLineOrigin;
			float fSqrLen = vLineDirection.getSqrLength();
			float fT = vDiff.getDot( vLineDirection ) / fSqrLen;
			vDiff -= fT * vLineDirection;
			float fRadiusSqr = vDiff.getSqrLength();

			// Is this radius greater than the current maximum radius
			if ( fRadiusSqr > fMaxRadiusSqr )
				fMaxRadiusSqr = fRadiusSqr;
		}

		// Calculate the minimum/maximum height of the cylinder
		Point3 vDiff = pPoints[0] - vLineOrigin;
		float fWMin = vLineDirection.getDot( vDiff ), fWMax = fWMin;
		for ( unsigned int i = 1; i < uiPointCount; i++)
		{
			vDiff = pPoints[i] - vLineOrigin;
			float fW = vLineDirection.getDot( vDiff );
			if ( fW < fWMin )
				fWMin = fW;
			else if ( fW > fWMax )
				fWMax = fW;
		}

		// Store the Cylinder Parameters
		vCenter = vLineOrigin + ( 0.5f * ( fWMax + fWMin ) ) * vLineDirection;
		vDirection = vLineDirection;
		fRadius = sqrt( fMaxRadiusSqr );
		fHeight = fWMax - fWMin;
	}
}

//
// createNormals
// Static function which generates normals given the vertices in a Geometry
//
void Geometry::createNormals( shared_ptr<Geometry> spGeometry )
{
	if ( spGeometry && spGeometry->m_vertexBuffer && spGeometry->m_vertexBuffer->size() )
	{
		// Reinterpret out vertex buffer points (which are floats) to Point3s
		const Point3 * pPoints = reinterpret_cast<const Point3 *>( &spGeometry->m_vertexBuffer->front() );
		unsigned short * pIndices = &spGeometry->m_indexBuffer->front();
		const unsigned int uiVertexBufferCount = spGeometry->m_vertexBuffer->size(); 
		const unsigned int uiPointCount = uiVertexBufferCount / 3;

		// Create normal buffer and allocate space for points
		spGeometry->m_normalBuffer.reset( new vector< float > );
		spGeometry->m_normalBuffer->resize( uiVertexBufferCount );

		// Reinterpret our normal buffer points (which are floats) to Point3s
		Point3 * pNormals = reinterpret_cast<Point3 *>( &spGeometry->m_normalBuffer->front() );

		// Iterate over the faces and calculate the face normals
		vector< Point3 > faceNormals;
		for( unsigned int faceIndex = 0; faceIndex < spGeometry->m_indexBuffer->size(); faceIndex +=3 )
		{
			// Compute the edges of the face
			Point3 edge0 = pPoints[ pIndices[ faceIndex + 0 ] ] - pPoints[ pIndices[ faceIndex + 1 ] ],
				   edge1 = pPoints[ pIndices[ faceIndex + 2 ] ] - pPoints[ pIndices[ faceIndex + 0 ] ];

			// Compute the face normal and normalize
			Point3 faceNormal = edge0.getCross( edge1 );

			// Store our vertex normals, accumulating them per face normal
			pNormals[ pIndices[ faceIndex + 0 ] ] += faceNormal;
			pNormals[ pIndices[ faceIndex + 1 ] ] += faceNormal;
			pNormals[ pIndices[ faceIndex + 2 ] ] += faceNormal;
		}

		// Renormalize all the vertex normals
		for( unsigned int normalIndex = 0; normalIndex < uiPointCount; normalIndex++ )
			pNormals[ normalIndex ].getNormalized();
	}
}

//
// createTangents
// Static function which generates tangent coordinates given the vertices and normals in a Geometry
//
void Geometry::createTangents( shared_ptr<Geometry> spGeometry )
{
	if ( spGeometry && spGeometry->m_vertexBuffer && spGeometry->m_vertexBuffer->size() && 
					   spGeometry->m_texture0Buffer && spGeometry->m_texture0Buffer->size() &&
					   spGeometry->m_normalBuffer && spGeometry->m_normalBuffer->size() )
	{
		// Reinterpret out vertex buffer points (which are floats) to Point3s
		const Point3 * pPoints = reinterpret_cast<const Point3 *>( &spGeometry->m_vertexBuffer->front() );
		unsigned short * pIndices = &spGeometry->m_indexBuffer->front();
		const unsigned int uiVertexBufferCount = spGeometry->m_vertexBuffer->size(); 
		const unsigned int uiPointCount = uiVertexBufferCount / 3;

		// Reinterpret our texture buffer points (which are floats) to Point2s
		const Point2 * pTextures = reinterpret_cast<const Point2 *>( &spGeometry->m_texture0Buffer->front() );

		// Reinterpret our normal buffer points (which are floats) to Point3s
		Point3 * pNormals = reinterpret_cast<Point3 *>( &spGeometry->m_normalBuffer->front() );

		// Buffer points reinterpreted into Point3s
		Point3 * pTangentS = 0, * pTangentT = 0, * pTangentST = 0;

		// Create S buffers, allocate space for points, and
		// reinterpret our S buffer points (which are floats) to Point3s
		spGeometry->m_tangentBasisS.reset( new vector< float > );
		spGeometry->m_tangentBasisS->resize( uiVertexBufferCount );
		pTangentS = reinterpret_cast<Point3 *>( &spGeometry->m_tangentBasisS->front() );

		// Optionally create the T buffers, allocate space for the points,
		// and reinterpret our T buffer points (which are floats) to Point3s
		if ( CHECK_FLAG( spGeometry->m_enabledBuffers, TANGENT_T ) ) {
			spGeometry->m_tangentBasisT.reset( new vector< float > );
			spGeometry->m_tangentBasisT->resize( uiVertexBufferCount );
			pTangentT = reinterpret_cast<Point3 *>( &spGeometry->m_tangentBasisS->front() );
		}

		// Optionally create the ST buffers, allocate space for the points,
		// and reinterpret our ST buffer points (which are floats) to Point3s
		if ( CHECK_FLAG( spGeometry->m_enabledBuffers, TANGENT_ST ) ) {
			spGeometry->m_tangentBasisST.reset( new vector< float > );
			spGeometry->m_tangentBasisST->resize( uiVertexBufferCount );
			pTangentST = reinterpret_cast<Point3 *>( &spGeometry->m_tangentBasisST->front() );
		}

		// reserve space for all verts and init to zero
		vector<Point3> avgS( uiPointCount, Point3( 0, 0, 0 ) );
		vector<Point3> avgT( uiPointCount, Point3( 0, 0, 0 ) );

		// Iterate over the faces and calculate the S, T, ST coordinates
		for( unsigned int faceIndex = 0; faceIndex < spGeometry->m_indexBuffer->size(); faceIndex += 3 )
		{
			// Get our vertices and UVs
			const Point3 & ptA = pPoints[ pIndices[ faceIndex + 0 ] ];
			const Point3 & ptB = pPoints[ pIndices[ faceIndex + 1 ] ];
			const Point3 & ptC = pPoints[ pIndices[ faceIndex + 2 ] ];
			const Point2 & texA = pTextures[ pIndices[ faceIndex + 0 ] ];
			const Point2 & texB = pTextures[ pIndices[ faceIndex + 1 ] ];
			const Point2 & texC = pTextures[ pIndices[ faceIndex + 1 ] ];

			// Compute the edges of the face
			Point3 edge0 = pPoints[ pIndices[ faceIndex + 0 ] ] - pPoints[ pIndices[ faceIndex + 1 ] ],
				   edge1 = pPoints[ pIndices[ faceIndex + 2 ] ] - pPoints[ pIndices[ faceIndex + 0 ] ];

			//Calculate face normal
			Point3 normal = edge1.getCross( edge0 );
			normal.getNormalized();

			// Calculate S (which is the tangent)
			float deltaV0 = texA.y - texB.y;
			float deltaV1 = texC.y - texA.y;
			Point3 tangent = deltaV1 * edge0 - deltaV0 * edge1;
			tangent.getNormalized();

			// Calculate T (which is the binormal)
			float deltaU0 = texA.x - texB.x;
			float deltaU1 = texC.x - texA.x;
			Point3 binormal = deltaU1 * edge0 - deltaU0 * edge1;
			binormal.getNormalized();

			// Now, we take the cross product of the tangents to get a vector which 
			// should point in the same direction as our normal calculated above. 
			// If it points in the opposite direction (the dot product between the normals is less than zero), 
			// then we need to reverse the s and t tangents. 
			// This is because the triangle has been mirrored when going from tangent space to object space.
			// reverse tangents if necessary
			Point3 tangentCross = tangent.getCross( binormal );
			if ( tangentCross.getDot( normal ) < 0.0f )
			{
				tangent = -tangent;
				binormal = -binormal;
			}

			// Accumulate the S,T for this face into each vertex...
			avgS[ pIndices[ faceIndex + 0 ] ] += tangent;
			avgS[ pIndices[ faceIndex + 1 ] ] += tangent;
			avgS[ pIndices[ faceIndex + 2 ] ] += tangent;
			avgT[ pIndices[ faceIndex + 0 ] ] += binormal;
			avgT[ pIndices[ faceIndex + 1 ] ] += binormal;
			avgT[ pIndices[ faceIndex + 2 ] ] += binormal;
		}

		// Compute the average S,T,ST for each vertex. This is acheived through a normalization.
		// Store this average S,T,ST into the newly created tangent buffers
		//
		// Note: Althrough we explicitly calculated the S, we implicitly calculate the T by
		// using the already compute Normal for this vertex, and ST by the cross between S and T
		for ( unsigned i = 0; i < uiPointCount; i++ ) {
			pTangentS[ i ] = avgS[i].getNormalized();
			if ( CHECK_FLAG( spGeometry->m_enabledBuffers, TANGENT_T ) ) pTangentT[ i ] = avgT[i].getNormalized();
			if ( CHECK_FLAG( spGeometry->m_enabledBuffers, TANGENT_ST ) ) pTangentST[ i ] = pTangentS[ i ].getCross( pTangentT[ i ] ).getNormalized();
		}
	}
}