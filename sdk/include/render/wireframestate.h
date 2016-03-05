/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		wireframestate.h
	Author:		Eric Bryant

	Sets the renderer into wireframe mode.
*/

#ifndef _WIREFRAMESTATE_H
#define _WIREFRAMESTATE_H

namespace Katana
{

///
/// WireframeState
/// Use this RenderState to put the renderer into wireframe mode
///
class WireframeState : public RenderState
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( WireframeState );

public:
	/// Constructor
	WireframeState( bool on = false );

	/// Sets the wireframe mode
	void setWireframe( bool on ) { m_wireStatus = on; }

	/// Retrieves the wireframe mode
	bool getWireframe()			{ return m_wireStatus; }

private:
	/// Is the state bit set for wireframe mode
	bool m_wireStatus;
};

KIMPLEMENT_STREAM( WireframeState );

}; // Katana

#endif // _WIREFRAMESTATE_H