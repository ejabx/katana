/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		shadingstate.cpp
	Author:		Eric Bryant

	Sets the renderer's shading mode
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "renderstate.h"
#include "shadingstate.h"

//
// RTTI Definition
// 
KIMPLEMENT_RTTI( ShadingState, RenderState );

// -------------------------------------------------------

//
// OnLoadStream
//
bool ShadingState::OnLoadStream( kistream & istr )
{
	// Load the shading mode
	istr >> (int&)m_shadingMode;

	return true;
}

//
// OnSaveStream
//
bool ShadingState::OnSaveStream( kostream & ostr ) const
{
	// Save the shading mode
	ostr << (int)m_shadingMode;

	return true;
}