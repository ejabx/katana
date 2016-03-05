/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		zone.cpp
	Author:		Eric Bryant

	A "Zone" is essentially an octree which handles frustum culling for the
	geometry. The leaves of the octree are further divided into BSP nodes
	for collision detection. The octree depth is determined by the number
	of triangles in model.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "system/systemfile.h"
#include "render/rendertypes.h"
#include "render/render.h"
#include "render/geometry.h"
#include "render/vertexbuffer.h"
#include "render/indexbuffer.h"
#include "engine/debugoutput.h"
#include "visible.h"
#include "camera.h"
#include "scenecontext.h"
#include "bspnode.h"
#include "zone.h"

// ----------------------------------------------------------------
// Constants
// ----------------------------------------------------------------

const unsigned int ROOT_BSP_NODE = 0;

// ----------------------------------------------------------------
// RTTI declaration
// ----------------------------------------------------------------

KIMPLEMENT_RTTI( Zone, Streamable );

// ----------------------------------------------------------------
// BSPScene
// ----------------------------------------------------------------

//
// Constructor
//
Zone::Zone()
	: m_zoneID( 0 )
{
}

Zone::Zone( unsigned short zoneID )
	: m_zoneID( zoneID )
{
}

Zone::Zone( unsigned short zoneID, shared_ptr<Geometry> geometry, unsigned int uiMaximumDepth, unsigned int uiMinimumTriCount )
	: m_zoneID( zoneID )
{
	createZone( geometry, uiMaximumDepth, uiMinimumTriCount );
}

//
// createZone
// Creates a zone given the geometry
//
bool Zone::createZone( shared_ptr<Geometry> geometry, unsigned int uiMaximumDepth, unsigned int uiMinimumTriCount  )
{
	// Store the geometry for later rendering
	m_geometry = geometry;

	// Setup our BSP Node Constructor
	BSPNodeConstructor bspConstructor( m_zoneID, geometry, m_bspNodes );
	bspConstructor.setMaximumNodeDepthLimit( uiMaximumDepth );
	bspConstructor.setMinimumTriangleCountPerLeaf( uiMinimumTriCount );

	// Construct the BSP
	return bspConstructor.makeBSP();
}

//
// preRender
// Initializes the Zone. This prepares it for rendering
// by creating a vertex buffer for our geometry
//
bool Zone::preRender( SceneContext * context )
{
	// Do we have geometry?
	if ( m_geometry )
	{
		// Ask the render to create a vb with our parameters
		Render * render = context->currentRenderer;
		if ( !render ) return false;

		// Note we don't create an index buffer because that's used separately
		m_vb.reset( render->CreateVB( m_geometry->m_enabledBuffers & ~INDEX,
									  STATIC,
									  m_geometry->m_vertexCount,
									  0 ) );

		// This is our composite index buffer. When rendering we concatenate
		// all the indices of the renderable nodes into one buffer in order to
		// do one renderVB() call
		m_ib.reset( render->CreateIB( WRITE_ONLY, m_geometry->m_indexCount ) );

		// Check whether we have a valid VB
		if ( !m_vb || !m_ib ) return false;

		// The VB will use the geometry's buffers for rendering
		m_vb->shareBuffers( *m_geometry.get() );
	}

	return true;
}

//
// render
// Renders the Zone.
//
bool Zone::render( SceneContext * context )
{
	// Render the vertex buffer if it's available
	if ( m_vb )
	{
		// Lock the Index Buffer to get raw access to the data
		m_ib->Lock();

		// Convert the destination index buffer into a flat array
		unsigned short * pDestIndexData = &m_ib->getIndexBufferData()[0];

		// This source data comes from the original geometry's index buffer
		unsigned short * pSrcIndexData = &m_geometry->m_indexBuffer->front();

		// We keep track of the total index count
		unsigned int uiTotalIndexCount = 0;

		// Recursively check the nodes of the zone for visibility and add their indices
		// to our destination index buffer for rendering
		checkAndRenderNodes( context, ROOT_BSP_NODE, pSrcIndexData, pDestIndexData, uiTotalIndexCount );

		// Unlock our destination index buffer
		m_ib->Unlock();

		// Make sure we have some primitives to draw
		if ( uiTotalIndexCount )
		{
			// Setup the primitive count
			m_vb->setPrimitveCount( uiTotalIndexCount / 3 );

			// Draw the primitives
			context->currentRenderer->RenderVB( m_vb.get(), m_ib.get() );
		}

		return true;
	}

	return false;
}

//
// debugRender
//
void Zone::debugRender( SceneContext * context )
{
	// Because our BSP is state, we only need to create the geometry one time
	// Check if the geometry exists, if not, create it and generate it
	if ( !m_debugGeometry )
	{
		// Create the debug geometry
		m_debugGeometry.reset( new Geometry );
		m_debugGeometry->m_primitiveType = LINESTRIP;
		m_debugGeometry->m_enabledBuffers = VERTEX;
		m_debugGeometry->m_primitiveCount = 0;
		m_debugGeometry->m_vertexBuffer.reset( new vector<float> );

		// Allocate enough space in the vertex buffer for all the lines
		m_debugGeometry->m_vertexBuffer->resize( m_bspNodes.size() * 17 * 3 ); // 17 lines per node

		// Start at the root and fill in the geometry for the bounds
		unsigned int uiNodeDepth = 0;
		createDebugGeometry( ROOT_BSP_NODE, uiNodeDepth );
	}
	// Otherwise we draw it
	else
	{
		// Render the geometry
		context->currentRenderer->RenderGeometry( m_debugGeometry.get() );
	}
}

//
// checkAndRenderNodes
//
void Zone::checkAndRenderNodes( SceneContext * context, unsigned int uiCurrentNodeIndex, 
								unsigned short * pSrcIndexData, unsigned short * pDestIndexData, 
								unsigned int & uiTotalIndexCount )
{
	// Retrieve this node by index
	BSPNode & node = m_bspNodes[uiCurrentNodeIndex];

	// Grab the current camera
	const Camera & camera = *context->currentCamera;

	// We can assume that the current visible object is our parent,
	// the BSPScene. We need to get the translation because our coordination
	// system is based on it's position (for bounding purposes)
	Visible * pBSPScene = context->currentVisibleObject;

	// Our world view matrix is the BSPScene's world view matrix multiplied by the scene
	Matrix4 worldViewMatrix = Matrix4( pBSPScene->getWorldMatrix() ) * camera.getWorldMatrix();

	// Transform the node's bound to world coordinates
	Bound nodeWorldBound = node.bound;
	nodeWorldBound.transform( worldViewMatrix );

	// Check whether this node is visible in the frustum. If not
	// there isn't any need to iterate over the children
	if ( context->debugOutput->getEnableFrustumCulling() && !camera.Cull( nodeWorldBound ) ) return;

	// Is this node a leaf and does it have triangle faces? If so,
	// add the faces to the destination index buffer
	if ( node.isLeaf() )
	{
		// Check that we actually have triangles to render
		if ( node.faceCount )
		{
			// Do a memcpy to copy the indices from this node's index to our destination index buffer
			memcpy( pDestIndexData + uiTotalIndexCount, 
					pSrcIndexData + node.faceIndex * 3, 
					node.faceCount * 3 * sizeof(unsigned short) );

			// Increment the total face count
			uiTotalIndexCount += node.faceCount * 3;
		}
	}
	// Otherwise we iterate over it's children and recursively perform this test
	else
	{
		for( unsigned int uiChildIdx = 0; uiChildIdx < MAX_OCTANTS; uiChildIdx++ )
			checkAndRenderNodes( context, node.children[uiChildIdx], pSrcIndexData, pDestIndexData, uiTotalIndexCount );
	}
}

void Zone::createDebugGeometry( unsigned int uiCurrentNodeIndex, unsigned int & uiNodeDepth )
{
	// Increment the node depth
	uiNodeDepth++;

	// Retrieve this node by index
	BSPNode & node = m_bspNodes[uiCurrentNodeIndex];

	// Increment the primitive count
	m_debugGeometry->m_primitiveCount += 16;

	// Add the corners to the geometry
	node.box.getCorners( (Point3 *)&m_debugGeometry->m_vertexBuffer->at( uiCurrentNodeIndex * 17 * 3 ), true );

	// If this node has children, render them also
	if ( !node.isLeaf() )
	{
		for( unsigned int uiChildIdx = 0; uiChildIdx < MAX_OCTANTS; uiChildIdx++ )
			createDebugGeometry( node.children[uiChildIdx], uiNodeDepth );
	}

	// Decrement the node depth
	uiNodeDepth--;
}

// ----------------------------------------------------------------

//
// OnLoadStream
//
bool Zone::OnLoadStream( kistream & istr )
{
	return true;
}

//
// OnSaveStream
//
bool Zone::OnSaveStream( kostream & ostr ) const
{
	return true;
}