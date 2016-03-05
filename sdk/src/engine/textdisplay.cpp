/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		textdisplay.cpp
	Author:		Eric Bryant

	System responsible for displaying text onto the screen. Generally 
	used for drawing debug text onto the screen (because of speed).
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "render/rendertypes.h"
#include "render/texture.h"
#include "render/render.h"
#include "render/geometry.h"
#include "render/vertexbuffer.h"
#include "render/renderstate.h"
#include "render/blendstate.h"
#include "render/multitexturestate.h"
#include "render/lightstate.h"
#include "render/cullstate.h"
#include "render/zbufferstate.h"
#include "scene/scenecontext.h"
#include "system/systemfile.h"
#include "textdisplay.h"

//
// TextDisplay::setFont
//
void TextDisplay::setFont( const string & fontTextureName, const string & fontWidthsFileName )
{
	vector<char> fontWidths;

	// Load the fonts file
	if ( !fontWidthsFileName.empty() )
	{
		SystemFile fontFile( fontWidthsFileName.c_str() );
		fontWidths.resize( 256 );
		fontFile.readBytes( &fontWidths[0], 256 );
	}

	if ( !fontTextureName.empty() )
		setFont( shared_ptr<Texture>( new Texture( fontTextureName.c_str() ) ), fontWidths );
}

void TextDisplay::setFont( shared_ptr<Texture> fontTexture, vector<char> fontWidths )
{
	// Store the font texture and widths
	m_fontTexture = fontTexture;
	m_fontWidths = fontWidths;

	// Setup the texture parameters
	m_fontTexture->setFilterMode( Texture::NEAREST );
	m_fontTexture->setRepeatMode( Texture::CLAMP_ST );

	// Setup the display parameters
	m_cellHeight = m_fontTexture->getHeight() / 16;
	m_cellWidth = m_fontTexture->getWidth() / 16;
}

//
// drawText
//
// Draws text onto the screen given a text draw operation
//
void TextDisplay::drawText( const TextDrawOperation & textDrawOp )
{
	// Push the text drawing operation on the vector
	m_textDrawOperations.push_back( textDrawOp );
}

//
// flushTextQueue
//
// Actually draws the text onto the screen. Clients should not execute this function, but infact the
// owner of this class (the GameEngine) should call this function all other objects are rendered
//
void TextDisplay::flushTextQueue( SceneContext * context )
{
	// Early exit if the text draw operations buffer is empty
	if ( m_textDrawOperations.empty() ) return;

	// Has the fonts been initialized properly?
	if ( !m_cellWidth || !m_cellHeight ) return;

	// Check if the vertex buffer has been created yet, and if it is of sufficient size
	if ( !m_textVertexBuffer || m_textVertexBuffer->getVertexCount() < ( m_maximumCharacters * 4 ) )
	{
		// Reset the text vertex buffer (in case we're resizing the buffer)
		m_textVertexBuffer.reset();

		// Ask the renderer to give us a vertex buffer of sufficient size
		m_textVertexBuffer.reset( context->currentRenderer->CreateVB( VERTEX_T | TEXTURE_0 | COLOR, 
																	  STATIC | WRITE_ONLY, 
																	  m_maximumCharacters * 4,	// Each character requires 4 vertices (as a quad)
																	  0 )						// No indices
		);

		// Setup the buffer parameters
		m_textVertexBuffer->setPrimitveType( TRIANGLE_STRIP );
		m_textVertexBuffer->setTriangleStripCount( 3 );
	}

	// Setup the render state
	context->currentRenderer->SetState( &LightState() );									// Disable lighting
	context->currentRenderer->SetState( &MultitextureState( m_fontTexture ) );				// Enable the font texture as the source texture
	context->currentRenderer->SetState( &BlendState( BLEND_SRCALPHA, BLEND_INVSRCALPHA ) ); // Alpha blending

	// Clear the character drawn to the screen
	m_currentCharacterCount = 0;
	
	// Lock the text vertex buffer
	m_textVertexBuffer->Lock();

	// Grab a pointer to the locked vertex data
	ksafearray<CharacterVertex> characterVertexData = m_textVertexBuffer->getVertexBufferData<CharacterVertex>();

	// Iterate over the text draw operations and draw them
	for( std::vector<TextDrawOperation>::iterator iter = m_textDrawOperations.begin();
		 iter != m_textDrawOperations.end();
		 iter++ )
	{
		// If we're over the character count, don't bother, exit this loop
		if ( m_currentCharacterCount > m_maximumCharacters )
			break;

		// Grab the text operation
		TextDrawOperation & textOperation = (*iter);

		// TODO: Convert the draw operation from absolute coordinate (in pixels)
		// to screen coordinates [0..1]
		if ( textOperation.fX < 0 || textOperation.fY < 0 )
		{
		}

		// Fill the vertex buffer with the character data
		fillTextVertexBuffer( characterVertexData, textOperation );
	}

	// Store the active vertex count (each character requires 4 vertices [as a quad])
	m_textVertexBuffer->setPrimitveCount( m_currentCharacterCount + 1 );

	// The vertex buffer has been filled with characters. Unlock the buffer and draw the vertex buffer
	m_textVertexBuffer->Unlock();
	context->currentRenderer->RenderVB( m_textVertexBuffer.get() );

	// Clear all the text drawing operations
	m_textDrawOperations.clear();

	// Disable the multitexturing and blending modes
	context->currentRenderer->SetState( &MultitextureState() );
	context->currentRenderer->SetState( &BlendState() );
}

//
// fillTextVertexBuffer
//
// Function that fills the vertex buffer with the text operation results
//
void TextDisplay::fillTextVertexBuffer( ksafearray<CharacterVertex> & characterVertexData, const TextDrawOperation & textDrawOp )
{
	// Source string to draw to the screen
	const string & sourceText = textDrawOp.strText;

	// Extract the text drawing parameters
	const unsigned int uiBitmapHeight = m_fontTexture->getHeight();
	const unsigned int uiBitmapWidth = m_fontTexture->getWidth();
	const long colorBottom = CreateColor( textDrawOp.colorBottom.r, textDrawOp.colorBottom.b, textDrawOp.colorBottom.g, textDrawOp.colorBottom.a );
	const long colorTop = CreateColor( textDrawOp.colorTop.r, textDrawOp.colorTop.b, textDrawOp.colorTop.g, textDrawOp.colorTop.a );
	float fScreenX1 = (float)textDrawOp.uiX;
	float fScreenY1 = (float)textDrawOp.uiY;
	float fScale = textDrawOp.fScale;
	float fScreenX2 = 0.f;
	float fScreenY2 = 0.f;

	// Flat array to the character vertex data (for speedy access)
	CharacterVertex * vdata = &characterVertexData[0] + ( ( m_currentCharacterCount + 1 ) * 4 );

	// Iterate over all the character of the string
	for( unsigned int cpos = 0; cpos < sourceText.size() + 1; cpos++ )
	{
		// Extract the character
		char chr = sourceText[cpos];

		// Is the font monospaced or proportional
		unsigned int cellWidth;
		if ( !m_fontWidths.empty() ) cellWidth = m_fontWidths[chr];
		else cellWidth = m_cellWidth;

		// Screen space dimensions for this character
		fScreenX2 = fScreenX1 + (float)cellWidth * fScale; // TODO: This should be variable based on the individual character's width, store in a data file
		fScreenY2 = fScreenY1 + (float)m_cellHeight * fScale;

		// Which part of the bitmap do we want for this character, scaled from 0 to 1
		float bmy1 = float( ( m_cellHeight ) *( chr / 16 ) ) / float( uiBitmapHeight );
		float bmx1 = float( ( m_cellWidth ) * ( chr % 16 ) ) / float( uiBitmapWidth ); 
		float bmy2 = bmy1 + ( float( m_cellHeight ) ) / (float( uiBitmapHeight ) );
		float bmx2 = bmx1 + ( float( cellWidth ) ) / (float( uiBitmapWidth ) ); // TODO: This should be variable based on the individual character's width, store in a data file

		// Set the vertex buffer data for each character.
		// Each character is actually a triangle strip with 4 vertices (3 strips) making a square
		vdata[cpos*4+0].x= fScreenX1-0.5f;	vdata[cpos*4+0].y= fScreenY2-0.5f;	vdata[cpos*4+0].z= 0.9f; vdata[cpos*4+0].rhw= 1.0f; vdata[cpos*4+0].color= colorBottom; vdata[cpos*4+0].tu= bmx1; vdata[cpos*4+0].tv= bmy2;
		vdata[cpos*4+2].x= fScreenX1-0.5f;	vdata[cpos*4+2].y= fScreenY1-0.5f;	vdata[cpos*4+2].z= 0.9f; vdata[cpos*4+2].rhw= 1.0f; vdata[cpos*4+2].color= colorTop;	vdata[cpos*4+2].tu= bmx1; vdata[cpos*4+2].tv= bmy1;
		vdata[cpos*4+1].x= fScreenX2-0.5f;	vdata[cpos*4+1].y= fScreenY2-0.5f;	vdata[cpos*4+1].z= 0.9f; vdata[cpos*4+1].rhw= 1.0f; vdata[cpos*4+1].color= colorBottom; vdata[cpos*4+1].tu= bmx2; vdata[cpos*4+1].tv= bmy2;
		vdata[cpos*4+3].x= fScreenX2-0.5f;	vdata[cpos*4+3].y= fScreenY1-0.5f;	vdata[cpos*4+3].z= 0.9f; vdata[cpos*4+3].rhw= 1.0f; vdata[cpos*4+3].color= colorTop;	vdata[cpos*4+3].tu= bmx2; vdata[cpos*4+3].tv= bmy1;

		// Increase the starting position of the next character
		fScreenX1 += cellWidth * fScale; // TODO: This should be variable based on the individual character's width, store in a data file		
	}

	// Increment the character count
	m_currentCharacterCount += sourceText.size();
}