/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		zbufferstate.cpp
	Author:		Eric Bryant

	Sets the renderer into z buffering mode.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "renderstate.h"
#include "zbufferstate.h"

//
// RTTI Definition
// 
KIMPLEMENT_RTTI( ZBufferState, RenderState );

// -------------------------------------------------------

//
// OnLoadStream
//
bool ZBufferState::OnLoadStream( kistream & istr )
{
	// Load the z buffering operations
	istr >> m_zenable;
	istr >> m_zwrites;
	istr >> (int&)m_zfunction;

	return true;
}

//
// OnSaveStream
//
bool ZBufferState::OnSaveStream( kostream & ostr ) const
{
	// Save the z buffering operations
	ostr << m_zenable;
	ostr << m_zwrites;
	ostr << (int)m_zfunction;

	return true;
}