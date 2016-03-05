/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		stencilstate.cpp
	Author:		Eric Bryant

	Sets the stencil buffer parameters within the renderer
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "renderstate.h"
#include "stencilstate.h"

//
// RTTI Definition
// 
KIMPLEMENT_RTTI( StencilState, RenderState );

// -------------------------------------------------------

//
// OnLoadStream
//
bool StencilState::OnLoadStream( kistream & istr )
{
	// Load the stencil values
	istr >> m_stencilEnabled;
	istr >> (int&)m_stencilFunc;
	istr >> (int&)m_zFailOp;
	istr >> (int&)m_stencilFailOp;
	istr >> (int&)m_stencilPassOp;
	istr >> m_ref;
	istr >> m_stencilMask;
	istr >> m_writeMask;

	return true;
}

//
// OnSaveStream
//
bool StencilState::OnSaveStream( kostream & ostr ) const
{
	// Save the stencil values
	ostr << (int)m_stencilEnabled;
	ostr << (int)m_stencilFunc;
	ostr << (int)m_zFailOp;
	ostr << (int)m_stencilFailOp;
	ostr << (int)m_stencilPassOp;
	ostr << m_ref;
	ostr << m_stencilMask;
	ostr << m_writeMask;

	return true;
}