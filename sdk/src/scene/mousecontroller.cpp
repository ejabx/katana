/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		mousecontroller.cpp
	Author:		Eric Bryant

	Simulates the mouse look functionality, similar to Maya.
*/

#include <bitset>
#include <math.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "visible.h"
#include "controller.h"
#include "mousecontroller.h"
#include "input/inputmessages.h"
#include "input/inputsystem.h"

//
// RTTI declaration
//
KIMPLEMENT_RTTI( MouseController, Controller );

//
// Externally defined structures
// 
extern shared_ptr<InputSystem> katana_input;

//
// Constructor
//

MouseController::MouseController( shared_ptr<Visible> visible ) :
	Controller(visible),
	m_rotationSensitivity(1.f),
	m_translationSensitivity(0.05f)
{
	// Grab the current translation, rotation
	m_objTranslation = m_target->getTranslation();
	m_objRotation = m_target->getRotation();
}

MouseController::MouseController( shared_ptr<Visible> visible, float rot, float trans ) :
	Controller(visible),
	m_rotationSensitivity(rot),
	m_translationSensitivity(trans)
{
	// Grab the current translation, rotation
	m_objTranslation = m_target->getTranslation();
	m_objRotation = m_target->getRotation();
}

//
// OnAttach
//
bool MouseController::OnAttach( SceneContext * context )
{
	// Register the controller as a listener for mouse messages
	katana_input->addListener( this );

	return true;
}

//
// OnDetach
//
bool MouseController::OnDetach( SceneContext * context )
{
	// Unregister the controller as a listener for mouse messages
	katana_input->removeListener( this );

	return true;
}

//
// OnMessage
//
bool MouseController::OnMessage( Message * message )
{
	MESSAGE_SWITCH_BEGIN( message )

		MESSAGE_CASE( InputMessage_Mouse, mouse )

			// Were any buttons pressed?
			if ( mouse->buttons.at( BUTTON_1 ) ||
				 mouse->buttons.at( BUTTON_2 ) ||
				 mouse->buttons.at( BUTTON_3 ) || 
				 mouse->z )
			{

				// Detect the Change between the previous mouse move
				float deltaX = mouse->x;
				float deltaY = mouse->y;
				float deltaZ = mouse->z;

				// Any Change?
				if ( ( deltaX != 0.f ) || ( deltaY != 0.f ) || mouse->z )
				{
					// Check for Pan
					if ( mouse->buttons.at( BUTTON_1 ) )
					{
						m_objTranslation = Point3( -deltaX, -deltaY, 0.f ) * m_translationSensitivity
							+ m_target->getTranslation();
					}

					// Check for Zooming with the mouse wheel
					if ( deltaZ )
					{
						m_objTranslation = Point3( 0.f, 0.f, -deltaZ ) * m_translationSensitivity
							+ m_target->getTranslation();
					}
					// Check for Regular Zoom
					else if ( mouse->buttons.at( BUTTON_2 ) )
					{
						m_objTranslation = Point3( 0.f, 0.f, -deltaX ) * m_translationSensitivity
							+ m_target->getTranslation();
					}

					// Check for Rotation
					if ( mouse->buttons.at( BUTTON_3 ) )
					{
						float mag = (float)sqrt( deltaX * deltaX + deltaY * deltaY );

						float dx = ( deltaX / mag );
						float dy = ( deltaY / mag );

						// Find a vector in the plane perpendicular to delta vector
						float perpx = dy;
						float perpy = dx;

						// Rotate around this vector
						Point3 axis( perpx, perpy, 0.0f );

						// Convert the Maginitude from Degrees to Radians
						float radAngle = (m_rotationSensitivity * mag) * (kmath::PI / 180.f);

						// Builds a matrix that rotates around an arbitrary axis
						Quaternion temp;
						temp.fromAngleAxis( radAngle, axis );

						// Multiply the Rotation Matrix
						m_objRotation *= temp;
					}

					// Store the Rotation/Translation in the Scene
					m_target->setTransform( m_objTranslation, m_objRotation );
					m_target->setDirty();
				}
			}

		MESSAGE_CASE_END()

	MESSAGE_SWITCH_END()

	return true;
}