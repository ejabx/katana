/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		bspscene.cpp
	Author:		Eric Bryant

	BSP Scene is used for rendering indoor environments, ala, Quake3.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "system/systemfile.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include "render/render.h"
#include "render/vertexbuffer.h"
#include "engine/debugoutput.h"
#include "visible.h"
#include "camera.h"
#include "visible.h"
#include "visnode.h"
#include "scenecontext.h"
#include "bspscene.h"
#include "bspnode.h"
#include "zone.h"

// ----------------------------------------------------------------
// RTTI declaration
// ----------------------------------------------------------------

KIMPLEMENT_RTTI( BSPScene, VisNode );

// ----------------------------------------------------------------
// BSPScene
// ----------------------------------------------------------------

//
// Constructor
//
BSPScene::BSPScene()
{
}

BSPScene::BSPScene( shared_ptr<Geometry> geometry )
{
	// Create the zone with the defaults
	createZone( geometry, BSPNodeConstructor::MAXIMUM_NODE_DEPTH, BSPNodeConstructor::MINIMUM_TRIANGLE_COUNT );
}

BSPScene::BSPScene( shared_ptr<Geometry> geometry, unsigned int uiMaximumDepth, unsigned int uiMinimumTriCount )
{
	// Create the zone
	createZone( geometry, uiMaximumDepth, uiMinimumTriCount );
}

//
// createZone
//
void BSPScene::createZone( shared_ptr<Geometry> geometry, unsigned int uiMaximumDepth, unsigned int uiMinimumTriCount )
{
	// Constructs a BSP given the geometry
	// TODO: Worry about the textures/materials for the BSP

	// This creates a new zone with the given geometry.
	shared_ptr<Zone> zone( new Zone( getNextZoneID(), geometry, uiMaximumDepth, uiMinimumTriCount ) );

	// Adds the zone to our collection of zones to render
	addZone( zone );
}

//
// OnAttach
//
bool BSPScene::OnAttach(SceneContext * context)
{
	// Call Base Class
	if ( !Visible::OnAttach( context ) )
		return false;

	// Iterate through the zones and generate vertex buffers
	// for all their geometries
	for( vector< shared_ptr<Zone> >::iterator iter = m_zones.begin();
		 iter != m_zones.end();
		 iter++ ) 
	{
		 (*iter)->preRender( context );
	}

	return true;
}

//
// OnDetach
//
bool BSPScene::OnDetach(SceneContext * context)
{
	return true;
}

//
// OnPreRender
//
bool BSPScene::OnPreRender(SceneContext * context)
{
	// Call the base method to make sure we're renderable
	if ( !Visible::OnPreRender( context ) )
		return false;

	return true;
}

//
// OnRender
//
bool BSPScene::OnRender(SceneContext * context)
{
	// Iterate through the zones and renders them
	for( vector< shared_ptr<Zone> >::iterator iter = m_zones.begin();
		iter != m_zones.end();
		iter++ ) 
	{
		(*iter)->render( context );
	}

	return true;
}

//
// OnPostRender
//
bool BSPScene::OnPostRender(SceneContext * context)
{
	// If we need to draw the bounds the have each zone draw its bounds
	if ( context->debugOutput->getDrawBounds() )
	{
		for( vector< shared_ptr<Zone> >::iterator iter = m_zones.begin();
			iter != m_zones.end();
			iter++ ) 
		{
			(*iter)->debugRender( context );
		}
	}

	return true;
}

// ----------------------------------------------------------------

//
// OnLoadStream
//
bool BSPScene::OnLoadStream( kistream & istr )
{
	return true;
}

//
// OnSaveStream
//
bool BSPScene::OnSaveStream( kostream & ostr ) const
{
	return true;
}