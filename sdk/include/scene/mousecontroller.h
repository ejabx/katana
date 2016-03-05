/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		mousecontroller.h
	Author:		Eric Bryant

	Simulates the mouse look functionality, similar to Maya.
*/

#ifndef _MOUSECONTROLLER_H
#define _MOUSECONTROLLER_H

namespace Katana
{

//
// Forward Declaration
//
struct Message;

///
/// MouseController
///
class MouseController 
	: public Controller, public Listener
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:
	/// Constructor which takes a visible target. Mouse look is centered around this target.
	MouseController( shared_ptr<Visible> visible );

	/// Constructor which takes a visible target. Mouse look is centered around this target.
	/// Parameters for setting the translation and rotation sensitivity are allowed
	MouseController( shared_ptr<Visible> visible, float rot, float trans );

	/// Sets the mouse sensitivity
	void setSensitivity( float rot, float trans ) { m_rotationSensitivity = rot; m_translationSensitivity = trans; }

	/// Called by the scene graph when it is first attached to the scene graph
	virtual bool OnAttach( SceneContext * context );

	/// Called by the scene graph when it is detached from the scene graph
	virtual bool OnDetach( SceneContext * context );

	/// Called when we receive input updates from the mouse device
	virtual bool OnMessage( Message * message );

private:
	/// The initial transform of the object
	Point3		m_objTranslation;
	Quaternion	m_objRotation;

	/// Mouse sensitivity
	float m_rotationSensitivity;		// Default 1.0
	float m_translationSensitivity;		// Default 0.05

};

KIMPLEMENT_SCRIPT( MouseController );

}; // Katana

#endif // _MOUSECONTROLLER_H