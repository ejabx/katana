/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		hardwarelitshader.cpp
	Author:		Eric Bryant

	Implements the standard hardware shader. When a material doesn't define
	a shader to use for rendering, this shader is used by default.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "scene/scenecontext.h"
#include "scene/visible.h"
#include "rendertypes.h"
#include "renderstate.h"
#include "multitexturestate.h"
#include "lightstate.h"
#include "materialstate.h"
#include "render.h"
#include "shader.h"
#include "material.h"
#include "hardwarelitshader.h"

// ------------------------------------------------------------------
// RTTI Definition
// ------------------------------------------------------------------

KIMPLEMENT_RTTI( HardwareLitShader, Shader );

// ------------------------------------------------------------------
// HardwareLitShader
// ------------------------------------------------------------------

//
// OnPreRender
// Called for every object which needs to render with this shader.
//
bool HardwareLitShader::OnPreRender( SceneContext * context )
{
	// Get a pointer to the currently rendered object
	Visible * pCurrentVisibleObject = context->currentVisibleObject;

	// Store this world view matrix with the render device. This is so
	// the next step in rendering, Visible::OnRender(), will already have 
	// the current world view matrix set.
	context->currentRenderer->SetMatrix( MODELVIEW, STORE, pCurrentVisibleObject->getWorldMatrix() );

	// If texture mapping is allowed, then grab the diffuse texture from
	// the visible object's material and set this as the target texture map
	if ( m_bTextureMaps && pCurrentVisibleObject->getMaterial() )
	{
		// Generate the texture states. The state will be responsible for setting the appropate texture slots
		// TODO: Support blending
		MultitextureState multiTextureState( pCurrentVisibleObject->getMaterial() );

		// Setup the textures within the renderer
		context->currentRenderer->SetState( &multiTextureState );
	}
	else
	{
		// If no texture maps are available, then set the texture state without any
		// textures. This will unbind all texture stages.
		context->currentRenderer->SetState( &MultitextureState() );
	}

	// If lighting is enabled, then setup the material states and the lighting parameters
	if ( m_bLighting && context->currentLights.size() )
	{
		// First, generate the material state (if applicable)
		if ( pCurrentVisibleObject->getMaterial() )
		{
			MaterialState materialState( pCurrentVisibleObject->getMaterial() );
			context->currentRenderer->SetState( &materialState );
		}

		// Next, tell the renderer to light the scene using vertex lighting
		LightState lightState( context->currentLights );
		context->currentRenderer->SetState( &lightState );
	}
	else
	{
		// If there are no lights or if it's disabled, turn off lighting
		context->currentRenderer->SetState( &LightState() );
	}

	// Call the base class to execute the pre render states
	return Shader::OnPreRender( context );
}

// ------------------------------------------------------------------------

//
// OnLoadStream
//
bool HardwareLitShader::OnLoadStream( kistream & istr )
{
	return true;
}

//
// OnSaveStream
//
bool HardwareLitShader::OnSaveStream( kostream & ostr ) const
{
	return true;
}