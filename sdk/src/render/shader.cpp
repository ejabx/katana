/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		shader.cpp
	Author:		Eric Bryant

	Class which represents vertex and fragment shaders.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "scene/scenecontext.h"
#include "rendertypes.h"
#include "render.h"
#include "shader.h"

//
// RTTI Definition
// 
KIMPLEMENT_ROOT_RTTI( Shader );

//
// OnPreRender
//
bool Shader::OnPreRender( SceneContext * context )
{
	// Iterate and execute the PreRender states
	for( vector< shared_ptr<RenderState> >::iterator iter = m_preRenderStates.begin();
		 iter != m_preRenderStates.end();
		 iter++ )
	{
		context->currentRenderer->SetState( (*iter).get() );
	}

	return true;
}

//
// OnPostRender
//
bool Shader::OnPostRender( SceneContext * context )
{
	// Iterate and execute the PostRender states
	for( vector< shared_ptr<RenderState> >::iterator iter = m_postRenderStates.begin();
		iter != m_postRenderStates.end();
		iter++ )
	{
		context->currentRenderer->SetState( (*iter).get() );
	}

	return true;
}