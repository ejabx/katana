/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		hardwarelitshader.h
	Author:		Eric Bryant

	Implements the hardware shader. This is the default shader used for rendering if
	a shader isn't specified. It implements lighting using the render driver, e.g.,
	OpenGL or DirectX. It also supports multitexturing.
*/

#ifndef _STANDARDSHADER_H
#define _STANDARDSHADER_H

namespace Katana
{

///
/// HardwareLitShader
///
class HardwareLitShader 
	: public Shader
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( HardwareLitShader );
	KDECLARE_SCRIPT;

public:

	/// Enables texture mapping. This is TRUE by default.
	void enableTextures( bool enable );

	/// Enables blending (via multitexturing). This is TRUE by default.
	void enableBlending( bool enable );

	/// Enables vertex lighting. This is TRUE by default, however, the scene must contain at least one light
	void enableLighting( bool enable );

public:

	// Shader Interface 
	virtual bool OnPreRender( SceneContext * context );

protected:
	bool	m_bTextureMaps;	/// Are texture maps used?
	bool	m_bBlending;	/// Are blending operations allowed?
	bool	m_bLighting;	/// Is hardware lighting enabled?
};

KIMPLEMENT_STREAM( HardwareLitShader );
KIMPLEMENT_SCRIPT( HardwareLitShader );

//
// HardwareLitShader::enableTextures
//
inline void HardwareLitShader::enableTextures( bool enable ) { m_bTextureMaps = enable; }

//
// HardwareLitShader::enableBlending
//
inline void HardwareLitShader::enableBlending( bool enable ) { m_bBlending = enable; }

//
// HardwareLitShader::enableLighting
//
inline void HardwareLitShader::enableLighting( bool enable ) { m_bLighting = enable; }


}; // Katana

#endif // _STANDARDSHADER_H