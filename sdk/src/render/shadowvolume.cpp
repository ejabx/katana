/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		shadowvolume.h
	Author:		Eric Bryant

	Represents a renderable shadow volume
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "rendertypes.h"
#include "render.h"
#include "light.h"
#include "geometry.h"
#include "shadowvolume.h"
#include "vertexbuffer.h"

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------

void addEdge( unsigned short* pEdges, unsigned long & dwNumEdges, unsigned short v0, unsigned short v1 );

//-----------------------------------------------------------------------------
// ShadowVolume
//-----------------------------------------------------------------------------

//
// create
// Creates a shadow volume. This initializes the internal vertex buffers
// given some geometry to use as the shadow caster.
//
bool ShadowVolume::create( Render & renderer, shared_ptr<Geometry> shadowCaster )
{
	// Store the shadow caster
	m_shadowCaster = shadowCaster;

	// Create a VB for the shadow volume.
	m_volumevb.reset( renderer.CreateVB( VERTEX, STATIC, 30000, 0 ) );
	if ( !m_volumevb ) return false;

	return true;
}

//
// update
// Given a light source, this update the shadow volume by extruding the
// volume's vertices in the direction of the light
//
bool ShadowVolume::update( const Point3 & lightDirection )
{
	// Check if we have a shadow caster
	if ( !m_shadowCaster ) return false;

	// Grab the face count, vertices, indices and direction of the light
	unsigned int faceCount = m_shadowCaster->m_indexCount / 3;
	const Point3 * pVertices = reinterpret_cast<const Point3 *>( &m_shadowCaster->m_vertexBuffer->front() );
	unsigned short * pIndices = &m_shadowCaster->m_indexBuffer->front();
	unsigned long dwNumEdges = 0;
	unsigned long dwNumVertices = 0;

	// Lock the shadow volume's vertex buffer and get a pointer to the raw vertices
	m_volumevb->Lock();
	Point3 * pShadowVertices = &m_volumevb->getVertexBufferData<Point3>()[0];

	// Allocate a temporary edge list
	unsigned short* pEdges = new unsigned short[ faceCount * 6 ];

	// Generate the edge list given all the faces
	for( unsigned long i = 0; i < faceCount; i++ )
	{
		unsigned short wFace0 = pIndices[3*i+0];
		unsigned short wFace1 = pIndices[3*i+1];
		unsigned short wFace2 = pIndices[3*i+2];

		Point3 v0 = pVertices[wFace0];
		Point3 v1 = pVertices[wFace1];
		Point3 v2 = pVertices[wFace2];

		// Transform vertices or transform light?
		Point3 vCross1(v2-v1);
		Point3 vCross2(v1-v0);
		Point3 vNormal;
		vNormal = vCross1.getCross( vCross2 );

		// If this normal is facing towards the light direction, then extrude add this face's edges
		if ( vNormal.getDot( lightDirection ) >= 0.f )
		{
			addEdge( pEdges, dwNumEdges, wFace0, wFace1 );
			addEdge( pEdges, dwNumEdges, wFace1, wFace2 );
			addEdge( pEdges, dwNumEdges, wFace2, wFace0 );
		}
	}

	// Now, for each edge, extrude them in the direction of the light
	for( i=0; i<dwNumEdges; i++ )
	{
		Point3 v1 = pVertices[ pEdges[ 2 * i + 0 ] ];
		Point3 v2 = pVertices[ pEdges[ 2 * i + 1 ] ];
		Point3 v3 = v1 - lightDirection * 10;
		Point3 v4 = v2 - lightDirection * 10;

		// Add a quad (two triangles) to the vertex list
		pShadowVertices[dwNumVertices++] = v1;
		pShadowVertices[dwNumVertices++] = v2;
		pShadowVertices[dwNumVertices++] = v3;

		pShadowVertices[dwNumVertices++] = v2;
		pShadowVertices[dwNumVertices++] = v4;
		pShadowVertices[dwNumVertices++] = v3;
	}

	// Setup the volume vb parameters
	m_volumevb->setPrimitveCount( dwNumVertices );
	m_volumevb->setActiveVertexCount( dwNumVertices );
	m_volumevb->setPrimitveType( TRIANGLE_LIST );

	// Delete the temporary edge list
	delete[] pEdges;

	// Unlock the shadow volume vb.
	m_volumevb->Unlock();

	return true;
}

//
// render
// Renders the shadow volume to the stencil buffer
//
void ShadowVolume::render( Render * render )
{
	// Render the shadow vertex buffer if it's available
	if ( m_volumevb && !m_volumevb->isDirty() )
		render->RenderVB( m_volumevb.get() );
}

//-----------------------------------------------------------------------------

//
// addEdge
// Adds an edge to a list of silohuette edges of a shadow volume.
//
void addEdge( unsigned short* pEdges, unsigned long & dwNumEdges, unsigned short v0, unsigned short v1 )
{
	// Remove interior edges (which appear in the list twice)
	for( unsigned long i=0; i < dwNumEdges; i++ )
	{
		if( ( pEdges[2*i+0] == v0 && pEdges[2*i+1] == v1 ) ||
			( pEdges[2*i+0] == v1 && pEdges[2*i+1] == v0 ) )
		{
			if( dwNumEdges > 1 )
			{
				pEdges[2*i+0] = pEdges[2*(dwNumEdges-1)+0];
				pEdges[2*i+1] = pEdges[2*(dwNumEdges-1)+1];
			}
			dwNumEdges--;
			return;
		}
	}

	pEdges[2*dwNumEdges+0] = v0;
	pEdges[2*dwNumEdges+1] = v1;
	dwNumEdges++;
}