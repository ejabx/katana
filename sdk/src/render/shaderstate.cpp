/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		shaderstate.cpp
	Author:		Eric Bryant

	Sets the render into the custom render pipeline state. This shader state
	is a persistent object that contains the shader code (stored as an identifier)
	and the constants needed to execute the shader.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "rendertypes.h"
#include "renderstate.h"
#include "shaderstate.h"

//
// RTTI Definition
// 
KIMPLEMENT_RTTI( ShaderState, RenderState );

// -------------------------------------------------------

//
// OnLoadStream
//
bool ShaderState::OnLoadStream( kistream & istr )
{
	return true;
}

//
// OnSaveStream
//
bool ShaderState::OnSaveStream( kostream & ostr ) const
{
	return true;
}