/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		blendstate.cpp
	Author:		Eric Bryant

	Sets the renderer into blending mode.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "renderstate.h"
#include "blendstate.h"

//
// RTTI Definition
// 
KIMPLEMENT_RTTI( BlendState, RenderState );

//
// Constructor
//
BlendState::BlendState() :
	m_enableBlending( false ),
	m_sourceOp( BLEND_ONE ),
	m_destOp( BLEND_ZERO )
{
}

BlendState::BlendState( bool enable ) :
	m_enableBlending( enable ),
	m_sourceOp( BLEND_ONE ),
	m_destOp( BLEND_ZERO )
{
}

BlendState::BlendState( BlendOp sourceOp, BlendOp destOp ) :
	m_enableBlending( true ),
	m_sourceOp( sourceOp ),
	m_destOp( destOp )
{
}

// -------------------------------------------------------

//
// OnLoadStream
//
bool BlendState::OnLoadStream( kistream & istr )
{
	// Load the blending states
	istr >> m_enableBlending;
	istr >> (int&)m_sourceOp;
	istr >> (int&)m_destOp;

	return true;
}

//
// OnSaveStream
//
bool BlendState::OnSaveStream( kostream & ostr ) const
{
	// Save the blending states
	ostr << m_enableBlending;
	ostr << (int)m_sourceOp;
	ostr << (int)m_destOp;

	return true;
}