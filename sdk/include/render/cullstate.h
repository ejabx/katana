/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		cullstate.h
	Author:		Eric Bryant

	Sets the renderer different polygon backfacing mode.
*/

#ifndef _CULLSTATE_H
#define _CULLSTATE_H

namespace Katana
{

///
/// CullFace
///
enum CullFace
{
	FRONT,
	BACK,
};

///
/// PolygonWinding
///
enum PolygonWinding
{
	CLOCKWISE,
	COUNTERCLOCKWISE,
};


///
/// CullState
/// Use this RenderState to set a different winding mode for the renderer
///
class CullState : public RenderState
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:
	/// Constructor (default is [OFF,BACKFACE,CLOCKWISE] which is equal to ogl)
	CullState(); 

	/// Enable culling with specified winding
	CullState(PolygonWinding winding, CullFace face);

	/// Sets the cull status
	void setCull( bool enabled )		{ m_enableCull = enabled; }

	/// Retrieves the cull state
	bool getCull() const				{ return m_enableCull; }

	/// Sets the polygon facing for culling
	void setCullFace( CullFace face )	{ m_cullFacing = face; }

	/// Retrieves the polygon facing
	CullFace getCullFace() const			{ return m_cullFacing; }

	/// Sets the polygon winding
	void setWinding( PolygonWinding winding )	{ m_polygonWinding = winding; }

	/// Retrieves the polygon winding
	PolygonWinding getWinding() const			{ return m_polygonWinding; }

private:
	/// Is polygon culling enabled?
	bool			m_enableCull;

	/// Which polygon facing is culled?
	CullFace		m_cullFacing;

	/// Which polygon winding determine a "Front-Face"?
	PolygonWinding	m_polygonWinding;
};

KIMPLEMENT_SCRIPT( CullState );

}; // Katana

#endif // _CULLSTATE_H