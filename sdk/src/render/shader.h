/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		shader.h
	Author:		Eric Bryant

	Class which represents vertex and fragment shaders.
*/

#ifndef _SHADER_H
#define _SHADER_H

namespace Katana
{

// Forward declarations
struct SceneContext;
class RenderState;

///
/// Shader
/// Abstract class for all shaders. They implement the render pipeline
/// by transforming our object into world space and drawing it. Clients can use
/// their custom render pipeline by:
/// 1.	Globally by replacing the Render's standard shader via Render::setStandardShader(),
///		if an object doesn't have a specific shader/material, it will use this by default.
/// 2.	On a per-object basis by setting this object's material's shader, via Material:m_shader
///		this will change the shader FOR THIS OBJECT only.
///
class Shader 
	: public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:

	/// Called for every object which needs to render with this shader.
	/// Use this function to setup the renderstate (like transforming the object
	/// from local space to world space).
	///
	/// If this is a multipass shader, then returning false means an additional
	/// pass is needed, while true means the shader has completed all passes.
	///
	virtual bool OnPreRender( SceneContext * context );

	/// Called after an object has used this shader.
	virtual bool OnPostRender( SceneContext * context );

public:

	/// Adds a render state to the PreRender collection
	void addPreRenderState( shared_ptr<RenderState> renderState );

	/// Adds a render state to the PostRender collection
	void addPostRenderState( shared_ptr<RenderState> renderState );

	/// Clears all entries from the PreRender collection
	void clearPreRenderStates();

	/// Clears all entries from the PostRender collection
	void clearPostRenderStates();

protected:

	/// Render states which are executed during OnPreRender
	vector< shared_ptr<RenderState> > m_preRenderStates;

	/// Render states which are executed during OnPostRender
	vector< shared_ptr<RenderState> > m_postRenderStates;

};

KIMPLEMENT_SCRIPT( Shader );

//
// Inline
//

//
// Shader::addPreRenderState
//
inline void Shader::addPreRenderState( shared_ptr<RenderState> renderState )
{
	m_preRenderStates.push_back( renderState );
}

//
// Shader::addPostRenderState
//
inline void Shader::addPostRenderState( shared_ptr<RenderState> renderState )
{
	m_postRenderStates.push_back( renderState );
}

//
// Shader::clearPreRenderStates
// 
inline void Shader::clearPreRenderStates()
{
	m_preRenderStates.clear();
}

//
// Shader::clearPostRenderStates
// 
inline void Shader::clearPostRenderStates()
{
	m_postRenderStates.clear();
}

}; // Katana

#endif // _SHADER_H