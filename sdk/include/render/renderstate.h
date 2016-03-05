/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		renderstate.h
	Author:		Eric Bryant

	Encapsulates a change in the render state (alpha mode, blending mode, etc.)
	Specific classes will derive from this class to implement specific
	states (WireframeState, AlphaState, etc.).
*/

#ifndef _RENDERSTATE_H
#define _RENDERSTATE_H

namespace Katana
{

//
// Forward Declarations
//
struct SceneContext;

///
/// RenderState
/// Base class for any state change within the renderer (alpha mode,
/// blending mode, z buffer mode, etc.). Derived classes are passes
/// to the renderer, and if it supports the renderstate, it will
/// incorporate the state change.
///
class RenderState : public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(RenderState);
	KDECLARE_SCRIPT;

public:
	/// Constructor
	RenderState()										{}

	/// Destructor
	virtual ~RenderState()								{}

public:
	/// Called before this render state is set within the renderer.
	virtual bool OnPreRender( SceneContext * context )	{ return true; }

	/// Called after this render state is set within the renderer and after the
	/// children nodes have been renderer.
	virtual bool OnPostRender( SceneContext * context ) { return true; }
};

KIMPLEMENT_STREAM( RenderState );
KIMPLEMENT_SCRIPT( RenderState );

}; // Katana

#endif // _RENDERSTATE_H
