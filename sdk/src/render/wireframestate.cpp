/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		wireframestate.cpp
	Author:		Eric Bryant

	Sets the renderer into wireframe mode.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "renderstate.h"
#include "wireframestate.h"

//
// RTTI Definition
// 
KIMPLEMENT_RTTI( WireframeState, RenderState );

//
// Constructor
//
WireframeState::WireframeState( bool on ) :
	m_wireStatus(on)
{
}

// -------------------------------------------------------

//
// OnLoadStream
//
bool WireframeState::OnLoadStream( kistream & istr )
{
	// Load the wireframe status
	istr >> m_wireStatus;

	return true;
}

//
// OnSaveStream
//
bool WireframeState::OnSaveStream( kostream & ostr ) const
{
	// Save the wireframe status
	ostr << m_wireStatus;

	return true;
}