/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		controller.h
	Author:		Eric Bryant

	Controllers are associated with visible objects and attached to
	the scene graph. They're updated every game tick, and somehow modify
	the attributes of the visible object every frame.
	Examples include keyframe animation controller, mouse controller, etc.
*/

#ifndef _CONTROLLER_H
#define _CONTROLLER_H

namespace Katana
{

//
// Forward Declaration
//
struct SceneContext;

///
/// Controller
/// 
class Controller : public RTTI
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:
	/// Constructor
	Controller()										{}

	/// Constructor takes a visible object as a parameter. A controller
	Controller( shared_ptr<Visible> target)				{ m_target = target; }

	/// Called by the scene graph when it is first attached to the scene graph
	virtual bool OnAttach( SceneContext * context )		{ return true; }

	/// Called by the scene graph when it is detached from the scene graph
	virtual bool OnDetach( SceneContext * context )		{ return true; }

	/// Called by the scene graph every game update
	virtual void OnUpdate( SceneContext * context )		{};

protected:

	/// The target object of the controller's operation
	shared_ptr<Visible>	m_target;
};

KIMPLEMENT_SCRIPT( Controller );

}; // Katana

#endif // _CONTROLLER_H 