/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		cullstate.cpp
	Author:		Eric Bryant

	Sets the renderer different polygon backfacing mode.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "renderstate.h"
#include "cullstate.h"

//
// RTTI Definition
// 
KIMPLEMENT_RTTI( CullState, RenderState );

//
// Constructor
//
CullState::CullState() :
	m_enableCull(false),
	m_cullFacing(BACK),
	m_polygonWinding(CLOCKWISE)
{
}

CullState::CullState(PolygonWinding winding, CullFace face) :
	m_enableCull(true),
	m_cullFacing(face),
	m_polygonWinding(winding)
{
}