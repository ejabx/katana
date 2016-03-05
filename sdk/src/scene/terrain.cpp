/*
	Katana Engine
	Copyright © 2001-2003 Eric Bryant, Inc.

	File:		terrain.cpp
	Author:		Eric Bryant

	Terrain Node.
*/

#include <list>
#include <map>
#include <algorithm>
#include "../base/kbase.h"
#include "../base/rtti.h"
#include "../base/refcount.h"
#include "../base/streamable.h"
#include "../base/color.h"
#include "../base/kstring.h"
#include "../base/karray.h"
#include "../base/log.h"
#include "../math/kmath.h"
#include "../math/point.h"
#include "../math/plane.h"
#include "../math/matrix.h"
#include "../math/quaternion.h"
#include "../math/bound.h"
#include "../base/karray.h"
#include "../base/kstream.h"
#include "../base/kistream.h"
#include "../base/kostream.h"
#include "../script/scriptengine.h"
#include "../system/systemfile.h"
#include "../render/geometry.h"
#include "../render/vertexbuffer.h"
#include "../render/indexbuffer.h"
#include "../render/heightfield.h"
#include "../render/render.h"
#include "../render/renderstate.h"
#include "../render/cullstate.h"
#include "../scene/scenecontext.h"
#include "../scene/visible.h"
#include "../scene/camera.h"
#include "visible.h"
#include "visnode.h"
#include "terrain.h"
#include "terrainpatch.h"
#include "terrainsettings.h"
using namespace Katana;

//
// Macros
//
#define INDEX(x,z) x+z*m_patchesZ

//
// RTTI declaration
//
KIMPLEMENT_RTTI( Terrain, VisNode );

//
// Constructor
//
Terrain::Terrain() :
	m_isInitialized( false )
{
}

Terrain::Terrain( TerrainSettings & settings )
{
	m_isInitialized = construct( settings );
}

//
// Destructor
//
Terrain::~Terrain()
{
	// Local function to delete a single patch
	struct Local 
	{
		static void deletePatch( TerrainPatch * pPatch ) { delete pPatch; }
	};

	// Iterate through the patches and destroy them
	std::for_each( m_patches.begin(), m_patches.end(), &Local::deletePatch );
}

//
// construct
//
bool Terrain::construct(TerrainSettings & settings )
{
	unsigned int px, pz;

	// Start logging the terrain construction process
	KLOG( "Constructing terrain: '%s'", settings.settingsFile.c_str() );

	// Load the heightmap via the greyscale texture
	AutoPtr<Heightfield> heightmap = new Heightfield( settings.heightMapFileName.c_str(), settings.worldWidth, settings.worldHeight );
	if ( !heightmap->isValid() ) return false;

	// Determine the patch units in (X,Y)
	m_patchesX = ( heightmap->getWidth() - 1 ) / ( TerrainPatch::PATCH_VERTEX_WIDTH - 1);
	m_patchesZ = ( heightmap->getHeight() - 1 ) / ( TerrainPatch::PATCH_VERTEX_HEIGHT - 1);

	// Determine the patch size
	m_patchSizeX = settings.worldWidth / m_patchesX;
	m_patchSizeZ = settings.worldDepth / m_patchesZ;

	// Store the world height
	m_worldHeight = settings.worldHeight;

	// For each patch in the collection, create the appropiate TerrainPatch (based on blend type)
	// and store it in the array
	for( pz = 0; pz < m_patchesZ; pz++ )
	{
		for( px = 0; px < m_patchesX; px++ )
		{
			// Create the appropate patch and stuff it in the collection
			TerrainPatch * patch = createPatch( x, z );
			m_patches.push_back( patch );
			
			// Setup the patch parameters
			patch->setScale( Point3( (float)m_patchSizeX, (float)m_worldHeight, (float)m_patchSizeZ ) );
			patch->setWorldTranslation( Point3( float( px * m_patchSizeX ), 0.f, float( pz * m_patchSizeZ ) ) );
			patch->setHeightMap( heightmap, px * ( TerrainPatch::PATCH_VERTEX_WIDTH - 1 ), pz * ( TerrainPatch::PATCH_VERTEX_HEIGHT - 1 ) );

			// Allow the patch to precalculate the error values
			patch->calculateErrors();
			patch->calculateMinMaxY();
		}
	}

	// Setup the relationship between patches and their neighbors
	for( pz = 0; pz < m_patchesZ; pz++ )
	{
		for( px = 0; px < m_patchesX; px++ )
		{
			m_patches[ INDEX(x,z) ]->setNeighbors(	px > 0 ? m_patches[ INDEX(pz,px-1) ]			: NULL,
													px < m_patchesX-1 ? m_patches[ INDEX(pz,px+1) ]	: NULL,
													pz > 0 ? m_patches[ INDEX(pz-1,px) ]			: NULL,
													pz < m_patchesX-1 ? m_patches[ INDEX(pz+1,pz) ]	: NULL);
		}
	}

	// Store the render method to use for the terrain patches
	m_renderMethod = ( settings.blendType == TerrainSettings::BLEND_UNIFIED ? RENDER_UNIFIED : RENDER_SPLIT );

	// Store the maximum screen error for rendering terrain patches
	m_maximumScreenError = settings.maxScreenError;

	// Log the resultant terrain
	KLOG("Terrain Statistics:\r\n"
		 "\tPatches: %d x %d = %d\r\n"
		 "\tPatch Size (vertices): %d x %d = %d\r\n"
		 "\tTerrain Size (world units): %d x %d x %d\r\n",
		m_patchesX, m_patchesZ, m_patchesX * m_patchesZ,
		TerrainPatch::PATCH_VERTEX_WIDTH, TerrainPatch::PATCH_VERTEX_HEIGHT, TerrainPatch::MAXIMUM_VERTICES,
		settings.worldWidth, settings.worldHeight, settings.worldDepth
	);

	return true;
}

//
// OnAttach
//
bool Terrain::OnAttach( SceneContext * context )
{
	// The triangles in the terrain are triangle strips and orders in the
	// opposite direction DirectX likes (CLOCKWISE), so we'll compensate
	addState( new CullState( COUNTERCLOCKWISE, BACK ) );

	return true;
}

//
// OnDetach
//
bool Terrain::OnDetach( SceneContext * context )
{
	// Invalidate the vertex buffer
	m_vb = 0;
	m_ib = 0;

	return true;
}

//
// OnPreRender
//
bool Terrain::OnPreRender( SceneContext * context )
{
	// Call base class to determine whether we've visible and update our world matrices
	if ( !Visible::OnPreRender( context ) ) return false;

	// Clear all the active patches
	m_activePatches.clear();

	// Cull the terrain patches
	for( unsigned int pz = 0; pz < m_patchesZ; pz++ )
		for( unsigned int px = 0; px < m_patchesX; px++ )
			if ( !context->currentCamera->Cull( Bound( m_patches[ INDEX(px, pz) ]->getWorldTranslation(), (float)m_patchSizeX ) ) )
				m_activePatches.push_back( m_patches[ INDEX(px, pz) ] );

	// Compute the projected error matrix
	Matrix4 projectedErrorMatrix; projectedErrorMatrix.setIdentity();
	projectedErrorMatrix.pos = Point3( 0.f, 0.f, 1.2f * std::max( m_worldHeight, std::max( m_patchSizeX, m_patchSizeZ ) ) );
	projectedErrorMatrix *= context->currentCamera->getProjection();
	projectedErrorMatrix *= m_worldViewMatrix;

	// Iterate through all the active (visible) patches and update their
	// tesselation level based on the projected error
	for( std::list<TerrainPatch *>::iterator iter = m_activePatches.begin(); 
		iter != m_activePatches.end(); 
		iter++ )
	{
		// Update the projected error for this patch
		(*iter)->updateProjectedErrors( projectedErrorMatrix );

		// For each tesselation level, check if the projected error
		// difference is less than our threshold.
		for ( int tessLevel = TerrainPatch::MAXIMUM_SUBDIVISION; tessLevel >= 0; tessLevel-- )
		{
			if ( (*iter)->getProjectedError( tessLevel ) < m_maximumScreenError )
			{
				(*iter)->setRealTesselation( tessLevel );
				(*iter)->setTesselation( tessLevel );
				break;
			}
		}
	}

	// Reset the total number of patch vertices and indices
	m_requiredPatchVertices = m_requiredPatchIndices = 0;

	// Iterate through all the active (visible) patches and have them
	// update their tesselation level by generating vertex/index data
	for( std::list<TerrainPatch *>::iterator iter = m_activePatches.begin(); 
		iter != m_activePatches.end(); 
		iter++ )
	{
		(*iter)->updateTesselation();
		m_requiredPatchVertices += (*iter)->getNewVertexCount();
		m_requiredPatchIndices += (*iter)->getNewIndexCount() + 2;	// Two more for degen-tris
	}

	// Iterate through all the active patches and call the additional tesselation
	// methods. Patches may need additional update passes to adapt to neighbors. 
	// This is because our tesselation may depend on the tesselation of our neighboring patches.
	std::for_each( m_activePatches.begin(), 
		m_activePatches.end(), 
		std::mem_fun( &TerrainPatch::updateTesselation2 ) );

	std::for_each( m_activePatches.begin(), 
		m_activePatches.end(), 
		std::mem_fun( &TerrainPatch::updateTesselation3 ) );

	// The last patch doesn't need degenerate triangles, so decremenent
	if ( m_requiredPatchIndices > 0 ) m_requiredPatchIndices -= 2;

	return true;
}

//
// OnRender
//
bool Terrain::OnRender( SceneContext * context )
{
	// Render the terrain using the appropiate method
	if	( m_renderMethod == RENDER_UNIFIED ) renderUnified( context );
	else									 renderSplit( context );

	// Call the base class to render the child nodes
	return VisNode::OnRender( context );
}

//
// createPatch
//
TerrainPatch * Terrain::createPatch( unsigned int px, unsigned int pz )
{
	// TODO: Create terrain patch based on blending type
	return new TerrainPatch( this, px, pz );
}

//
// createBuffers
//
bool Terrain::createBuffers( SceneContext * context )
{
	// If no buffer creation is required, return false
	if ( !m_requiredPatchVertices || !m_requiredPatchIndices ) return false;

	if ( m_renderMethod == RENDER_UNIFIED )
	{
		if ( !m_ib.isValid() || ( m_requiredPatchIndices > m_ib->getIndexCount() ) )
		{
			m_ib = context->currentRenderer->CreateIB( DYNAMIC | WRITE_ONLY, m_requiredPatchIndices );
			if ( !m_ib.isValid() ) return false;
		}
		if ( !m_vb.isValid() || ( m_requiredPatchVertices > m_vb->getVertexCount() ) )
		{
			m_vb = context->currentRenderer->CreateVB( VERTEX | TEXTURE_0, DYNAMIC | WRITE_ONLY, m_requiredPatchVertices, 0 );
			if ( !m_vb.isValid() ) return false;
		}
	}

	return true;
}

//
// renderUnified
//
void Terrain::renderUnified( SceneContext * context )
{
	// If no patches are present, we can exit
	if ( !m_patches.size() || m_activePatches.size() == 0 ) return;

	// If unable to create the buffers, exit
	if ( !createBuffers( context ) ) return;

	// Lock the vertex buffer. This will get the raw hardware vertex and index buffer
	if ( !m_vb.isValid() || !m_vb->LockRange( 0, m_requiredPatchVertices ) ) return;
	if ( !m_ib.isValid() || !m_ib->LockRange( 0, m_requiredPatchIndices ) ) return;

	// Get safe arrays to the hardware vertex buffers
	ksafearray<TerrainVertex> vertexData = m_vb->getVertexBufferData<TerrainVertex>();
	ksafearray<unsigned short> indexData = m_ib->getIndexBufferData();

	// Check whether the we have vertex and index data in the raw buffers
	if ( vertexData.empty() || indexData.empty() ) return;

	int numVertices = 0, numIndices = 0, numPatches = 0;
	unsigned short lastIndex = 0xFFFF;

	// Iterate through the patches, filling in the vertex buffers
	for( std::list<TerrainPatch *>::iterator iter = m_activePatches.begin();
		 iter != m_activePatches.end();
		 iter++ )
	{
		int startIndexCount = numIndices;
		// Copy the vertex data from the patch to the vertex buffer
		memcpy( &vertexData[numVertices], 
				&(*iter)->getPatchVertices()[0], 
				(*iter)->getNewVertexCount() * sizeof(TerrainVertex) );

		// Add degenerate triangles to the concatenated strips
		if ( lastIndex != 0xFFFF )
		{
			indexData[numIndices++] = lastIndex;
			indexData[numIndices++] = (*iter)->getPatchIndices()[0] + numVertices;
		}

		// Copy the indices, which must be updated to reflect the moved vertex data
		for( int i = 0; i < (*iter)->getNewIndexCount(); i++ )
			indexData[numIndices++] = (*iter)->getPatchIndices()[i] + numVertices;

		// Increment the number of vertices and patches rendered
		lastIndex = indexData[numIndices - 1];
		numVertices += (*iter)->getNewVertexCount();
		numPatches++;
	}

	// Unlock the buffers
	m_vb->Unlock();
	m_ib->Unlock();

	// Setup the vertex/index buffer rendering parameters
	m_vb->setPrimitveType( TRIANGLE_STRIP );
	m_vb->setActiveVertexCount( numVertices );
	m_ib->setActiveIndexCount( numIndices - 2 );

	// Render the terrain geometry
	Render * render = context->currentRenderer;
	if ( render ) render->RenderVB( m_vb, m_ib );
}

//
// renderSplit
//
void Terrain::renderSplit( SceneContext * context )
{
}

// -------------------------------------------------------


//
// OnLoadStream
//
bool Terrain::OnLoadStream( kistream & istr )
{
	// Call base class
	VisNode::OnLoadStream( istr );

	return true;
}

//
// OnSaveStream
//
bool Terrain::OnSaveStream( kostream & ostr ) const
{
	// Call base class
	VisNode::OnSaveStream( ostr );

	return true;
}