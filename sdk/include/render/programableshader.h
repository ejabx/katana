/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		programableshader.h
	Author:		Eric Bryant

	Class which represents vertex and fragment shaders.
*/

#ifndef _PROGRAMABLESHADER_H
#define _PROGRAMABLESHADER_H

namespace Katana
{

//
// Forward declaration
//
class ShaderState;

///
/// ProgramableShader
/// Shader which contains vertex or framgment code.
///
class ProgramableShader
	: public Shader
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( ProgramableShader );
	KDECLARE_SCRIPT;

public:
	/// Constructor
	ProgramableShader();

	/// Constructor which takes a shader file
	ProgramableShader( const char * szShaderFile );

	// Shader Interface 
	virtual bool OnPreRender( SceneContext * context );
	virtual bool OnPostRender( SceneContext * context );

protected:

	// This is the shader state which is passed to the renderer to enable programmable shaders
	shared_ptr<ShaderState>	m_shaderState;
};

KIMPLEMENT_STREAM( ProgramableShader );
KIMPLEMENT_SCRIPT( ProgramableShader );

}; // Katana

#endif // _PROGRAMABLESHADER_H