/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		sprite.h
	Author:		Eric Bryant

	A optionally billboarded sprite. Sprite can also contain animationed
	"filmstrips" to simulate traditional cell animation frames.

	This sprite can be considered a "heavy" class, it is a visible-derived
	object that acts likes a particle. For better performance, use a particle system
	instead.
*/

#ifndef _SPRITE_H
#define _SPRITE_H

namespace Katana
{

// Forward Declarations
class VertexBuffer;
class Texture;
struct SceneContext;

///
/// Sprite
///
class Sprite : public Visible
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:

	/// Default constructor
	Sprite();

	/// Constructor which takes a texture
	Sprite( shared_ptr<Texture> spriteTexture );

	/// Constructor which takes a texture and a default width and height
	Sprite( shared_ptr<Texture> spriteTexture, float fWidth, float fHeight );

	/// Sets whether this sprite is billboarded
	void setBillboard( bool billboard );

	/// Gets whether this sprite is billboarded
	bool getBillboard() const;

	/// Sets the sprite texture
	void setTexture( shared_ptr<Texture> spriteTexture );

	/// Gets the sprite texture
	shared_ptr<Texture> getTexture() const;

public:

	/// Sprite will attempt to create the VB specific to the device when it
	/// is first attached to the scene. It is a Quad.
	virtual bool OnAttach(SceneContext * context);

	/// Sprite will release it's reference count to the VB, which may delete
	/// it if it's reference count is zero.
	virtual bool OnDetach(SceneContext * context);

	/// Sprite will detect whether it needs to the change the VB (like if we
	/// are executing animation), and repopulate the VB with update data.
	virtual bool OnPreRender(SceneContext * context);

	/// Sprite will render the Quad VB during this event.
	virtual bool OnRender(SceneContext * context);

protected:

	/// Representative of a vertex in the vertex buffer for a single sprite
	struct SpriteVertex { float x, y, z; unsigned long color; float tu, tv; };

	/// Upload the vertex and texture coordinates to the vb
	void uploadSpriteToVB();

protected:

	/// Vertex buffer used to draw the text drawing operations
	shared_ptr<VertexBuffer>	m_quadVB;

	/// Texture which contains the sprite
	shared_ptr<Texture>			m_spriteTexture;

	/// The sprite dimensions
	float						m_fWidth, m_fHeight;

	/// Flags whether this sprite is billboarded
	bool						m_bBillboard;
};

KIMPLEMENT_SCRIPT( Sprite );

//
// Inline
//

//
// Sprite::constructors
//
inline Sprite::Sprite() 
	: m_fWidth( 0 )
	, m_fHeight( 0 )
{
}

inline Sprite::Sprite( shared_ptr<Texture> spriteTexture ) 
	: m_fWidth( 0 )
	, m_fHeight( 0 )
	, m_spriteTexture( spriteTexture )
{
}

inline Sprite::Sprite( shared_ptr<Texture> spriteTexture, float fWidth, float fHeight ) 
	: m_fWidth( fWidth )
	, m_fHeight( fHeight )
	, m_spriteTexture( spriteTexture )
{
}

//
// Sprite::setBillboard
//
inline void Sprite::setBillboard( bool billboard )
{
	m_bBillboard = billboard;
}

//
// Sprite::getBillboard
//
inline bool Sprite::getBillboard() const
{
	return m_bBillboard;
}

//
// Sprite::setTexture
//
inline void Sprite::setTexture( shared_ptr<Texture> spriteTexture )
{
	m_spriteTexture = spriteTexture;
}

//
// Sprite::getTexture
//
inline shared_ptr<Texture> Sprite::getTexture() const
{
	return m_spriteTexture;
}

}; // Katana

#endif // _SPRITE_H