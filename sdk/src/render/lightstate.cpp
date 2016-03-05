/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		lightstate.cpp
	Author:		Eric Bryant

	Sets the renderer into hardware lighting mode. This is dependent on the driver
	for support (DX8,OGL), and is usually restricted to 8 hardware lights. To get more
	lights, try vertex/pixel shaders.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "rendertypes.h"
#include "render.h"
#include "renderstate.h"
#include "light.h"
#include "lightstate.h"
#include "scene/scenecontext.h"
#include "scene/visible.h"
#include "scene/visnode.h"

//
// RTTI Definition
// 
KIMPLEMENT_RTTI( LightState, RenderState );

//
// Constructor
//
LightState::LightState()
	: m_enableLights( false )
{
}

//
// OnPreRender
//
bool LightState::OnPreRender( SceneContext * context )
{
	// Clear all the lights from the previous operation
	m_lights.clear();

	// If we don't have a parent, return without setting the lights
	if ( !context->currentParent ) return false;

	// Otherwise, grab the lights from the parent
	m_lights = context->currentLights;

	// We want to draw the lights during the next render pass
	m_enableLights = true;

	return true;
}

//
// OnPostRender
//
bool LightState::OnPostRender( SceneContext * context )
{
	// Don't draw the lights
	m_enableLights = false;

	// Sets the state
	return context->currentRenderer->SetState( this );
}

// -------------------------------------------------------

//
// OnLoadStream
//
bool LightState::OnLoadStream( kistream & istr )
{
	// Call the base class -- LightState doesn't have any member variables to load
	return RenderState::OnLoadStream( istr );
}

//
// OnSaveStream
//
bool LightState::OnSaveStream( kostream & ostr ) const
{
	// Call the base class -- LightState doesn't have any member variables to store
	return RenderState::OnSaveStream( ostr );
}