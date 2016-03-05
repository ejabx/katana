/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		sprite.cpp
	Author:		Eric Bryant

	A optionally billboarded sprite. Sprite can also contain animationed
	"filmstrips" to simulate traditional cell animation frames.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "render/texture.h"
#include "render/rendertypes.h"
#include "render/render.h"
#include "render/renderstate.h"
#include "render/geometry.h"
#include "render/vertexbuffer.h"
#include "render/blendstate.h"
#include "render/shaderstate.h"
#include "render/multitexturestate.h"
#include "render/lightstate.h"
#include "visible.h"
#include "sprite.h"
#include "scenecontext.h"

//
// RTTI declaration
//
KIMPLEMENT_RTTI( Sprite, Visible );

//
// OnAttach 
// Sprite will attempt to create the VB specific to the device when it
// is first attached to the scene. It is a Quad.
//
bool Sprite::OnAttach( SceneContext * context )
{
	// Call base class
	if ( !Visible::OnAttach( context ) )
		return false;

	// Ask the renderer to give us a vertex buffer of sufficient size
	m_quadVB.reset( context->currentRenderer->CreateVB( VERTEX | TEXTURE_0 | COLOR,
													    STATIC | WRITE_ONLY, 
														4,						// Each sprite requires 4 vertices (as a quad)
														0 )						// No indices
	);

	// Setup the VB parameters
	m_quadVB->setPrimitveCount( 1 ); // We're only rendering one quad
	m_quadVB->setPrimitveType( TRIANGLE_STRIP );
	m_quadVB->setTriangleStripCount( 2 );

	// Return false if the vb wasn't filled
	if ( !m_quadVB ) return false;

	// Upload the vertex and texture coordinates to the vb
	uploadSpriteToVB();

	return true;
}


//
// OnDetach 
// Sprite will release it's reference count to the VB, which may delete
// it if it's reference count is zero.
//
bool Sprite::OnDetach( SceneContext * context )
{
	// Release the vb
	m_quadVB.reset();

	return true;
}

//
// OnRender 
// Sprite will detect whether it needs to the change the VB (like if we
// are executing animation), and repopulate the VB with update data.
//
bool Sprite::OnPreRender( SceneContext * context )
{
	// Call the base method to make sure we're renderable
	if ( !Visible::OnPreRender( context ) )
		return false;

	return true;
}

//
// OnRender 
// Sprite will render the Quad VB during this event.
//
bool Sprite::OnRender( SceneContext * context )
{
	// Render the vertex buffer if it's available
	if ( m_quadVB && m_spriteTexture )
	{
		Render * render = context->currentRenderer;

		// Setup the render state
		render->SetState( &LightState() );										// Disable lighting
		render->SetState( &ShaderState() );										// Disable shaders
		render->SetState( &MultitextureState( m_spriteTexture ) );				// Enable the sprite texture as the source texture
//		render->SetState( &BlendState( BLEND_SRCALPHA, BLEND_INVSRCALPHA ) );	// Alpha blending

		// Render the Sprite
		render->RenderVB( m_quadVB.get() );

		// Disable the multitexturing and blending modes
		render->SetState( &MultitextureState() );
		render->SetState( &BlendState() );

		return true;
	}

	return false;
}

//
// uploadSpriteToVB
// Upload the vertex and texture coordinates to the vb
// 
void Sprite::uploadSpriteToVB()
{
	// Lock the text vertex buffer
	m_quadVB->Lock();

	// Grab a pointer to the locked vertex data
	ksafearray<SpriteVertex> spriteVertexData = m_quadVB->getVertexBufferData<SpriteVertex>();

	// Flat array to the character vertex data (for speedy access)
	SpriteVertex * vdata = &spriteVertexData[0];

	// Currently all sprite use a white color and don't support vertex coloring
	const long COLOR_WHITE = CreateColor( 1, 1, 1, 1 );

	// Dimensions of this sprite
	float fScreenX1 = 0;
	float fScreenY1 = 0;
	float fScreenX2 = m_fWidth;
	float fScreenY2 = m_fHeight;

	// Texture coordinate for this sprite
	float bmy1 = 1,  bmx1 = 0, bmy2 = 1, bmx2 = 0;

	// Set the vertex buffer data for each character.
	// Each character is actually a triangle strip with 4 vertices (3 strips) making a square
	vdata[0].x= fScreenX1-0.5f;	vdata[0].y= fScreenY2-0.5f;	vdata[0].z= 0.9f; vdata[0].color= COLOR_WHITE; vdata[0].tu= bmx1; vdata[0].tv= bmy2;
	vdata[2].x= fScreenX1-0.5f;	vdata[2].y= fScreenY1-0.5f;	vdata[2].z= 0.9f; vdata[2].color= COLOR_WHITE; vdata[2].tu= bmx1; vdata[2].tv= bmy1;
	vdata[1].x= fScreenX2-0.5f;	vdata[1].y= fScreenY2-0.5f;	vdata[1].z= 0.9f; vdata[1].color= COLOR_WHITE; vdata[1].tu= bmx2; vdata[1].tv= bmy2;
	vdata[3].x= fScreenX2-0.5f;	vdata[3].y= fScreenY1-0.5f;	vdata[3].z= 0.9f; vdata[3].color= COLOR_WHITE; vdata[3].tu= bmx2; vdata[3].tv= bmy1;

	// The vertex buffer has been filled with the quad. Unlock the buffer
	m_quadVB->Unlock();
}