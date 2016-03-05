/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		renderstate.h
	Author:		Eric Bryant

	Encapsulates a change in the render state (alpha mode, blending mode, etc.)
	Specific classes will derive from this class to implement specific
	states (WireframeState, AlphaState, etc.).
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "texture.h"
#include "renderstate.h"

//
// RTTI Definitions
//
KIMPLEMENT_RTTI( RenderState, Streamable );

// -------------------------------------------------------

//
// OnLoadStream
//
bool RenderState::OnLoadStream( kistream & istr )
{
	return true;
}

//
// OnSaveStream
//
bool RenderState::OnSaveStream( kostream & ostr ) const
{
	return true;
}