/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		bspgeometry.cpp
	Author:		Eric Bryant

	Geometry specific to a BSP Scene
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "bspgeometry.h"

// ----------------------------------------------------------
// RTTI Definition
// ----------------------------------------------------------

KIMPLEMENT_ROOT_RTTI( BSPGeometry );

// ----------------------------------------------------------


//
// OnLoadStream
//
bool BSPGeometry::OnLoadStream( kistream & istr )
{
	// Load the geometry information
	istr >> m_vertexCount;
	istr >> m_faceCount;

	// Load the geometry array data
//	istr << m_vertexBuffer;
//	istr << m_faceBuffer;

	return true;
}

//
// OnSaveStream
//
bool BSPGeometry::OnSaveStream( kostream & ostr ) const
{
	// Save the geometry information
	ostr << m_vertexCount;
	ostr << m_faceCount;

	// Save the geometry array data
//	ostr << m_vertexBuffer;
//	ostr << m_faceBuffer;

	return true;
}