/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		shadingstate.h
	Author:		Eric Bryant

	Sets the renderer's shading mode
*/

#ifndef _SHADINGSTATE_H
#define _SHADINGSTATE_H

namespace Katana
{

///
/// ShadeMode
/// The different shading modes
enum ShadeMode
{
	SHADE_FLAT = 1,
	SHADE_GOURAUD = 2,
	SHADE_PHONG = 3,
};

///
/// ShadingState
/// Use this RenderState to set the renderer's shading mode
///
class ShadingState : public RenderState
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( ShadingState );

public:
	/// Constructor (using Gouraud shading by default)
	ShadingState() : m_shadingMode( SHADE_GOURAUD ) {}

	/// Constructor which takes a shading mode
	ShadingState( ShadeMode mode ) : m_shadingMode( mode ) {}
	
	/// Gets the shading mode
	ShadeMode getShadingMode() const { return m_shadingMode; }

private:

	/// The shading mode
	ShadeMode	m_shadingMode;
};

KIMPLEMENT_STREAM( ShadingState );

}; // Katana

#endif // _SHADINGSTATE_H