/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		shadowvolume.h
	Author:		Eric Bryant

	Represents a renderable shadow volume
*/

#ifndef _SHADOWVOLUME_H
#define _SHADOWVOLUME_H

namespace Katana
{

// Forward Declarations
class Render;
struct Geometry;
class Light;
class VertexBuffer;

///
/// ShadowVolume
///
class ShadowVolume
{
public:

	/// Creates a shadow volume. This initializes the internal vertex buffers
	/// given some geometry to use as the shadow caster.
	bool create( Render & renderer, shared_ptr<Geometry> shadowCaster );

	/// Given a light source, this update the shadow volume by extruding the
	/// volume's vertices in the direction of the light
	bool update( const Point3 & lightDirection );

	/// Renders the shadow volume to the stencil buffer
	void render( Render * Render );

private:

	/// This is the geometry associated with this shadow volume
	shared_ptr<Geometry>	m_shadowCaster;

public:

	/// This is the shadow vertex buffer which contained the extruded
	/// shadow volume. Render this during OnRenderShadow
	shared_ptr<VertexBuffer> m_volumevb;

};

}; // Katana

#endif //  _SHADOWVOLUME_H