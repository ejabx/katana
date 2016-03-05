/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		stencilshadowshader.h
	Author:		Eric Bryant

	Shader which prepares the scene for rendering stencil shadow volumes
*/

#ifndef _STENCILSHADOWSHADER_H
#define _STENCILSHADOWSHADER_H

namespace Katana
{

///
/// StencilShadowState
///
class StencilShadowShader
	: public Shader
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( StencilShadowShader );

public:

	// Shader Interface 

	/// Called by the scene in order to prepare it for rendering stencil shadow volumes.
	///
	/// Based on whether the render device supports two-pass stencil tests, this may
	/// be a one or two pass shader. Call this function and draw your shadow volumes
	/// until this function return true to indicate the shader is complete.
	virtual bool OnPreRender( SceneContext * context );

	/// Called by the scene in order to restore the render states
	virtual bool OnPostRender( SceneContext * context );

	/// Static helper method which renders an alpha blended full screen quad
	/// which stencil testing on to render the shadows
	static void renderShadows( SceneContext * context );

	/// Shadow color (medium grey)
	static const unsigned long m_shadowColor = 0x7f000000;

private:

	/// Enumation for the render passes
	enum { RENDER_SHADOW_VOLUME_FRONTFACE, RENDER_SHADOW_VOLUME_BACKFACE };

};

KIMPLEMENT_STREAM( StencilShadowShader );

} // Katana

#endif // _STENCILSHADOWSHADER_H