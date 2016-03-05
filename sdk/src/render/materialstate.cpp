/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		materialstate.cpp
	Author:		Eric Bryant

	Sets the renderer into a new material mode
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "renderstate.h"
#include "material.h"
#include "materialstate.h"

// -------------------------------------------------------
// RTTI Definition
// -------------------------------------------------------

KIMPLEMENT_RTTI( MaterialState, RenderState );

// -------------------------------------------------------
// MaterialState
// -------------------------------------------------------

//
// OnLoadStream
//
bool MaterialState::OnLoadStream( kistream & istr )
{
	// Load the material
	istr >> m_material;

	return true;
}

//
// OnSaveStream
//
bool MaterialState::OnSaveStream( kostream & ostr ) const
{
	// Save the material
	ostr << m_material;

	return true;
}