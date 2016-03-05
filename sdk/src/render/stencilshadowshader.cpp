/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		stencilshadowshader.cpp
	Author:		Eric Bryant

	Shader which prepares the scene for rendering stencil shadow volumes
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "scene/scenecontext.h"
#include "rendertypes.h"
#include "renderstate.h"
#include "zbufferstate.h"
#include "blendstate.h"
#include "shadingstate.h"
#include "cullstate.h"
#include "stencilstate.h"
#include "shaderstate.h"
#include "zbufferstate.h"
#include "lightstate.h"
#include "multitexturestate.h"
#include "render.h"
#include "shader.h"
#include "geometry.h"
#include "vertexbuffer.h"
#include "stencilshadowshader.h"

// ------------------------------------------------------------------
// RTTI Definition
// ------------------------------------------------------------------

KIMPLEMENT_RTTI( StencilShadowShader, Shader );

// ------------------------------------------------------------------
// Local Definitions
// ------------------------------------------------------------------

struct SHADOW_VERTEX_FVF 
{
	float x, y, z, rhw;
	unsigned long color;
};

// ------------------------------------------------------------------
// Local Variables
// ------------------------------------------------------------------

shared_ptr<VertexBuffer> shadowQuadVertexBuffer;

// ------------------------------------------------------------------
// StencilShadowShader
// ------------------------------------------------------------------

//
// OnPreRender
// Called by the scene in order to prepare it for rendering stencil shadow volumes
//
bool StencilShadowShader::OnPreRender( SceneContext * context )
{
	// Have we completed all shadow passes?
	if ( context->renderPass > RENDER_SHADOW_VOLUME_BACKFACE ) 
		return true;

	// Check the render pass and increment it
	switch( context->renderPass++ )
	{
		case RENDER_SHADOW_VOLUME_FRONTFACE:
		{
			// Setup the render state for stencil shadow
			Render * render = context->currentRenderer;
			render->SetState( &ShaderState() );
			render->SetState( &ZBufferState( true, false ) );				// Disable z buffer writes
			render->SetState( &ShadingState( SHADE_FLAT ) );	
			render->SetState( &StencilState( 1, STENCIL_OP_INCR ) );		// Enable stencil buffer with ref = 1
			render->SetState( &BlendState( BLEND_ZERO, BLEND_ONE ) );		// Make sure nothing is rendered into the frame buffer
			render->SetState( &CullState( COUNTERCLOCKWISE, BACK ) );
			break;
		}
		case RENDER_SHADOW_VOLUME_BACKFACE:
		{
			// We reverse the cull order in order to write the back sides of the shadow volume
			Render * render = context->currentRenderer;
			render->SetState( &CullState( CLOCKWISE, BACK ) );
			render->SetState( &StencilState( 1, STENCIL_OP_DECR ) );
			break;
		}
	}

	// Call the base class to execute the pre render states
	Shader::OnPreRender( context );

	// The client should execute another pass
	return false;
}

//
// OnPostRender
// Called by the scene in order to restore the render states
//
bool StencilShadowShader::OnPostRender( SceneContext * context )
{
	Render * render = context->currentRenderer;

	// Restore the render states
	render->SetState( &ZBufferState() );
	render->SetState( &CullState( CLOCKWISE, BACK ) );
	render->SetState( &StencilState() );
	render->SetState( &BlendState() );
	render->SetState( &ShadingState( SHADE_GOURAUD ) );

	// Call the base class to execute the post render states
	return Shader::OnPostRender( context );
}

//
// renderShadows
// Static helper method which renders an alpha blended full screen quad
// which stencil testing on to render the shadows
//
void StencilShadowShader::renderShadows( SceneContext * context )
{
	// Set renderstates (disable z-buffering, enable stencil, disable fog, and turn on alphablending)
	context->currentRenderer->SetState( &CullState() );
	context->currentRenderer->SetState( &LightState() );									// Disable lighting
	context->currentRenderer->SetState( &ZBufferState( false, false ) );
	context->currentRenderer->SetState( &StencilState( 1, STENCIL_COMPARE_LESSEQUAL ) );
	context->currentRenderer->SetState( &BlendState( BLEND_SRCALPHA, BLEND_INVSRCALPHA ) );
	context->currentRenderer->SetState( &MultitextureState() );

	// If the shadow quad's vertex buffer isn't created, do so now
	if ( !shadowQuadVertexBuffer ) {

		// Create the vertex buffer
		shadowQuadVertexBuffer.reset(	context->currentRenderer->CreateVB(  VERTEX_T | COLOR, 
																			STATIC | WRITE_ONLY, 
																			4,						// A quad is 4 vertices
																			0 )						// No indices
		);

		// Setup the buffer parameters
		shadowQuadVertexBuffer->setPrimitveType( TRIANGLE_STRIP );
		shadowQuadVertexBuffer->setTriangleStripCount( 2 );
		shadowQuadVertexBuffer->setPrimitveCount( 1 );

		// Lock the buffer to fill the vertices
		shadowQuadVertexBuffer->Lock();	

		// Flat array to the character vertex data (for speedy access)
		SHADOW_VERTEX_FVF * vdata = &shadowQuadVertexBuffer->getVertexBufferData<SHADOW_VERTEX_FVF>()[0];
		int sx = context->currentRenderer->GetRenderInfo().uiTargetWidth, 
			sy = context->currentRenderer->GetRenderInfo().uiTargetHeight;

		vdata[0].x= 0;	vdata[0].y= sy;	vdata[0].z= 0.9f; vdata[0].rhw= 1.0f; vdata[0].color= m_shadowColor;
		vdata[2].x= sx;	vdata[2].y= sy;	vdata[2].z= 0.9f; vdata[2].rhw= 1.0f; vdata[2].color= m_shadowColor;
		vdata[1].x= 0;	vdata[1].y= 0;	vdata[1].z= 0.9f; vdata[1].rhw= 1.0f; vdata[1].color= m_shadowColor;
		vdata[3].x= sx;	vdata[3].y= 0;	vdata[3].z= 0.9f; vdata[3].rhw= 1.0f; vdata[3].color= m_shadowColor;

		// Unlock the buffer
		shadowQuadVertexBuffer->Unlock();
	}

	// Render the quad
	context->currentRenderer->RenderVB( shadowQuadVertexBuffer.get() );
}

// ------------------------------------------------------------------------

//
// OnLoadStream
//
bool StencilShadowShader::OnLoadStream( kistream & istr )
{
	return true;
}

//
// OnSaveStream
//
bool StencilShadowShader::OnSaveStream( kostream & ostr ) const
{
	return true;
}

