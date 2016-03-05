/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		multitexturestate.cpp
	Author:		Eric Bryant

	RenderState which enables multitexturing within the renderer
	and sets up multiple source textures within a rendering pass.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "texture.h"
#include "material.h"
#include "rendertypes.h"
#include "renderstate.h"
#include "multitexturestate.h"

//
// RTTI Definitions
//
KIMPLEMENT_RTTI( MultitextureState, RenderState );

//
// Constructor
//
MultitextureState::MultitextureState()
{
	m_blendOperation[0] = NONE;
	m_blendOperation[1] = NONE;
	m_blendOperation[2] = NONE;
	m_blendOperation[3] = NONE;
}

MultitextureState::MultitextureState( shared_ptr<Texture> texture1, shared_ptr<Texture> texture2,
									  shared_ptr<Texture> texture3, shared_ptr<Texture> texture4 )
{
	m_sourceTexture[0] = texture1;
	m_sourceTexture[1] = texture2;
	m_sourceTexture[2] = texture3;
	m_sourceTexture[3] = texture4;

	m_blendOperation[0] = NONE;
	m_blendOperation[1] = NONE;
	m_blendOperation[2] = NONE;
	m_blendOperation[3] = NONE;
}

MultitextureState::MultitextureState( shared_ptr<Texture> texture1,	BlendType blend1,
									  shared_ptr<Texture> texture2,	BlendType blend2,
									  shared_ptr<Texture> texture3,	BlendType blend3,
									  shared_ptr<Texture> texture4,	BlendType blend4 )
{
	m_sourceTexture[0] = texture1;
	m_blendOperation[0] = blend1;

	m_sourceTexture[1] = texture2;
	m_blendOperation[1] = blend2;

	m_sourceTexture[2] = texture3;
	m_blendOperation[2] = blend3;

	m_sourceTexture[3] = texture4;
	m_blendOperation[3] = blend4;
}

MultitextureState::MultitextureState( const char * szTexture1, BlendType blend1,
									  const char * szTexture2, BlendType blend2,
									  const char * szTexture3, BlendType blend3,
									  const char * szTexture4, BlendType blend4 )
{
	m_sourceTexture[0] = Texture::Create( szTexture1 );
	m_blendOperation[0] = blend1;

	if ( szTexture2 && *szTexture2 != 0 )
	{
		m_sourceTexture[1] = Texture::Create( szTexture2 );
		m_blendOperation[1] = blend2;
	}
	else
	{
		m_sourceTexture[1].reset( new Texture( 1.f, 1.f, 1.f, 1.f ) );
		m_blendOperation[1] = MODULATE;
	}

	if ( szTexture3 && *szTexture3 != 0 )
	{
		m_sourceTexture[2] = Texture::Create( szTexture3 );
		m_blendOperation[2] = blend3;
	}
	else
	{
		m_sourceTexture[2].reset( new Texture( 1.f, 1.f, 1.f, 1.f ) );
		m_blendOperation[2] = MODULATE;
	}

	if ( szTexture4 && *szTexture4 != 0 )
	{
		m_sourceTexture[3]=  Texture::Create( szTexture4 );
		m_blendOperation[3] = blend4;
	}
	else
	{
		m_sourceTexture[3].reset( new Texture( 1.f, 1.f, 1.f, 1.f ) );
		m_blendOperation[3] = MODULATE;
	}
}

MultitextureState::MultitextureState( shared_ptr<Material> material )
{
	unsigned int usedSlot = 0;

	// By default assign slot 0 to the first diffuse map
	if ( material->diffuseMap0 )
		m_sourceTexture[usedSlot++] = material->diffuseMap0;

	// Assign the bump, specular, and opacity maps to the next available slots
	if ( material->bumpMap ) m_sourceTexture[usedSlot++] = material->bumpMap;
	if ( material->specularMap ) m_sourceTexture[usedSlot++] = material->specularMap;
	if ( material->opacityMap ) m_sourceTexture[usedSlot++] = material->opacityMap;

	// Finally, assign the diffuse maps only if we have enough slots
	if ( usedSlot < MAX_TEXTURE_PASSES && material->diffuseMap1 )
		m_sourceTexture[usedSlot++] = material->diffuseMap1;
	if ( usedSlot < MAX_TEXTURE_PASSES && material->diffuseMap2 )
		m_sourceTexture[usedSlot++] = material->diffuseMap2;
	if ( usedSlot < MAX_TEXTURE_PASSES && material->diffuseMap3 )
		m_sourceTexture[usedSlot++] = material->diffuseMap3;

	m_blendOperation[0] = NONE;
	m_blendOperation[1] = NONE;
	m_blendOperation[2] = NONE;
	m_blendOperation[3] = NONE;
}

//
// setSourceTexture
//
void MultitextureState::setSourceTexture( shared_ptr<Texture> texture, int pass )
{
	if ( pass < MAX_TEXTURE_PASSES )
		m_sourceTexture[pass] = texture;
}

//
// setBlendOperation
//
void MultitextureState::setBlendOperation( BlendType blend, int pass )
{
	if ( pass < MAX_TEXTURE_PASSES )
		m_blendOperation[pass] = blend;
}

//
// getSourceTexture
//
shared_ptr<Texture> MultitextureState::getSourceTexture(int pass)
{
	if ( pass < MAX_TEXTURE_PASSES )
		return m_sourceTexture[pass];
	else
		return shared_ptr<Texture>();
}

//
//
//
MultitextureState::BlendType MultitextureState::getBlendOperation(int pass)
{
	if ( pass < MAX_TEXTURE_PASSES )
		return m_blendOperation[pass];
	else
		return INVALID_BLEND;
}

// -------------------------------------------------------

//
// OnLoadStream
//
bool MultitextureState::OnLoadStream( kistream & istr )
{
	int maxTexPasses;

	// Load the number of texture passes from disk
	istr >> maxTexPasses;

	// Load the blending operations and textures from disk for each
	// texture pass, clamping to the maximum number of texture passes 
	// if necessary
	for( int pass = 0; pass < maxTexPasses; pass++ )
	{
		istr >> (long &)m_blendOperation[pass];
		istr >> m_sourceTexture[pass];
	}

	return true;
}

//
// OnSaveStream
//
bool MultitextureState::OnSaveStream( kostream & ostr ) const
{
	// Save the number of texture passes to disk
	ostr << MAX_TEXTURE_PASSES;

	// Save the blending operations and textures to disk
	// for each texture pass
	for( int pass = 0; pass < MAX_TEXTURE_PASSES; pass++ )
	{
		ostr << m_blendOperation[pass];
		ostr << m_sourceTexture[pass];
	}

	return true;
}