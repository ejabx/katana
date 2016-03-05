/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		multitexturestate.h
	Author:		Eric Bryant

	RenderState which enables multitexturing within the renderer
	and sets up multiple source textures within a rendering pass.
*/

#ifndef _MULTITEXTURESTATE_H
#define _MULTITEXTURESTATE_H

namespace Katana
{

//
// Forward Declarations
//
class Texture;
struct Material;

///
/// MultitextureState
/// Enabled a multitexture operation within the renderer.
///
class MultitextureState : public RenderState
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(MultitextureState);
	KDECLARE_SCRIPT;

public:
	/// The blending mode for each texture pass
	enum BlendType
	{
		INVALID_BLEND = -1,
		NONE,
		REPLACE,
		MODULATE,
		DECAL,
		ADD,
		ADD_SIGNED,
		DETAIL,
		ALPHA,
	};

public:
	/// Constructor
	MultitextureState();

	/// Constructor which takes a reference to four existing textures.
	/// By default it will use blending mode of NONE.
	MultitextureState( shared_ptr<Texture> texture1, shared_ptr<Texture> texture2,
					   shared_ptr<Texture> texture3, shared_ptr<Texture> texture4 );

	/// Constructor which takes a reference to existing texture/operations.
	/// Up to three additional textures and bledning operations are available.
	MultitextureState( shared_ptr<Texture> texture1,							BlendType blend1 = REPLACE,
					   shared_ptr<Texture> texture2 = shared_ptr<Texture>(),	BlendType blend2 = NONE,
					   shared_ptr<Texture> texture3 = shared_ptr<Texture>(),	BlendType blend3 = NONE,
					   shared_ptr<Texture> texture4 = shared_ptr<Texture>(),	BlendType blend4 = NONE );

	/// Constructor which will load the textures from a resouce.
	/// Up to three additional textures and blending operations are available.
	MultitextureState( const char * szTexture1,		BlendType blend1 = REPLACE,
					   const char * szTexture2 = 0, BlendType blend2 = NONE,
					   const char * szTexture3 = 0, BlendType blend3 = NONE,
					   const char * szTexture4 = 0, BlendType blend4 = NONE );

	/// Constructor which takes a material. The texture slots are assigned according to priority:
	/// DiffuseMap0 is always assigned to texture slot 0. Afterwards, bump mapping (if available)
	/// is assigned to texture slot 1. If there are specular and opacity maps, they are assigned
	/// to the other avaialble. Finally, if there are additional diffuse maps, they are also
	/// assigned to the other available slots.
	MultitextureState( shared_ptr<Material> material );

	/// Overrides a source texture for a pass
	void setSourceTexture( shared_ptr<Texture> texture, int pass );

	/// Overrides a blending operation for a pass
	void setBlendOperation( BlendType blend, int pass );

	/// Retrieves a source texture for a pass
	shared_ptr<Texture> getSourceTexture(int pass);

	/// Retrieves a blending operation for a pass
	BlendType getBlendOperation(int pass);

protected:
	/// Reference to the source textures
	shared_ptr<Texture>	m_sourceTexture[MAX_TEXTURE_PASSES];

	/// Blending operation for each texture pass
	BlendType			m_blendOperation[MAX_TEXTURE_PASSES];
};

KIMPLEMENT_STREAM( MultitextureState );
KIMPLEMENT_SCRIPT( MultitextureState );

}; // Katana

#endif // _MULTITEXTURESTATE_H