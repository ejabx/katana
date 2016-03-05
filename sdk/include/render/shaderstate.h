/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		shaderstate.h
	Author:		Eric Bryant

	Sets the render into the custom render pipeline state. This shader state
	is a persistent object that contains the shader code (stored as an identifier)
	and the constants needed to execute the shader.
*/

#ifndef _SHADERSHADER_H
#define _SHADERSHADER_H

namespace Katana
{

///
/// ShaderID
/// This shader handle is stored with the shader state to cache the internal
/// shader used by the Render. Generally, this is a pointer cast as a long
///
typedef long ShaderID;

///
/// ShaderState
/// Use this RenderState to enable hardware lighting mode
///
class ShaderState : public RenderState
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( ShaderState );

public:
	/// Default constructor
	ShaderState();

	/// Constructor which takes shader file to read and execute. Note the code is
	/// uncompiled, it is up to the renderer to compile and execute the code
	ShaderState( const string & shaderFile );

public:

	/// Retrieve the uncompiled shader file
	const string & getShaderFile();

	/// Sets the handle to the compiled shader. 
	
	/// This handle is set by the Renderer and is used to cache the internal
	/// shader so it does not have to recompile the shader file per execution.
	///
	/// A shader file may contain multiple shader programs (for instance, a 
	/// vertex shader and pixel shader). Shader 0 is generally the vertex 
	/// shader and Shader 1 is the pixel shader. Shader2 is for additional shader support.
	void setShaderHandle0( ShaderID shaderHandle );
	void setShaderHandle1( ShaderID shaderHandle );
	void setShaderHandle2( ShaderID shaderHandle );

	/// Gets the shader handles to the compiled shaders
	ShaderID getShaderHandle0() const;
	ShaderID getShaderHandle1() const;
	ShaderID getShaderHandle2() const;

	/// Retrieve the constants for the specificed shader handler
	vector<ShaderConstant> & getShaderConstants0();
	vector<ShaderConstant> & getShaderConstants1();
	vector<ShaderConstant> & getShaderConstants2();

private:
	/// Shader file in its uncompiled form. It is up to the renderer to
	/// compile the code and execute it. It will cache the compiled shader.
	string					m_shaderFile;

	/// This references a handle to an internal shader in the render engine
	ShaderID				m_shaderHandle0, m_shaderHandle1, m_shaderHandle2;

	/// Vector of shader constant
	vector<ShaderConstant>	m_shaderConstant0, m_shaderConstant1, m_shaderConstant2;
};

KIMPLEMENT_STREAM( ShaderState );
KIMPLEMENT_SCRIPT( ShaderState );

//
// Inline
//

//
// ShaderState::constructor
//
inline ShaderState::ShaderState() 
	: m_shaderHandle0( 0 )
	, m_shaderHandle1( 0 )
	, m_shaderHandle2( 0 )
{}

inline ShaderState::ShaderState( const string & shaderFile ) 
	: m_shaderHandle0( 0 )
	, m_shaderHandle1( 0 )
	, m_shaderHandle2( 0 )
	, m_shaderFile( shaderFile )
{}

//
// ShaderState::getShaderFile
//
inline const string & ShaderState::getShaderFile()
{
	return m_shaderFile;
}

//
// ShaderState::setShaderHandle*
//
inline void ShaderState::setShaderHandle0( ShaderID shaderHandle )
{
	m_shaderHandle0 = shaderHandle;
}

inline void ShaderState::setShaderHandle1( ShaderID shaderHandle )
{
	m_shaderHandle1 = shaderHandle;
}

inline void ShaderState::setShaderHandle2( ShaderID shaderHandle )
{
	m_shaderHandle2 = shaderHandle;
}

//
// ShaderState::getShaderHandle*
//
inline ShaderID ShaderState::getShaderHandle0() const
{
	return m_shaderHandle0;
}

inline ShaderID ShaderState::getShaderHandle1() const
{
	return m_shaderHandle1;
}

inline ShaderID ShaderState::getShaderHandle2() const
{
	return m_shaderHandle2;
}

//
// ShaderState::getShaderConstants*
//
inline vector<ShaderConstant> & ShaderState::getShaderConstants0()
{
	return m_shaderConstant0;
}

inline vector<ShaderConstant> & ShaderState::getShaderConstants1()
{
	return m_shaderConstant1;
}

inline vector<ShaderConstant> & ShaderState::getShaderConstants2()
{
	return m_shaderConstant2;
}

} // Katana

#endif // _SHADERSHADER_H