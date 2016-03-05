/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		inputsystem.h
	Author:		Eric Bryant

	Responsible for quering various input devices
*/

#ifndef _INPUTSYSTEM_H
#define _INPUTSYSTEM_H

namespace Katana
{

///
/// InputDevice
///
enum
{
	INPUT_KEYBOARD	= 1,
	INPUT_MOUSE		= 2,
	INPUT_JOYSTICK	= 4,

	INPUT_JOYSTICK_A = INPUT_JOYSTICK,
	INPUT_JOYSTICK_B = INPUT_JOYSTICK >> 1,
};
typedef int InputDevice;

///
/// InputSystem
///
class InputSystem : public MessageRouter
{
	KDECLARE_SCRIPT;

public:
	/// Constructor
	InputSystem();

	/// Destructor
	virtual ~InputSystem();

	/// Initialize the input system
	bool Initialize( WindowHandle handle );

	/// Terminates the input system
	bool Terminate();

	/// Call to poll the attached devices and send the appropiate messages to the listeners
	bool Poll();

public:
	/// Enables a particular device
	bool enableDevice( InputDevice device );

	/// Disables a particular device
	bool disableDevice( InputDevice device );

	/// Queries whether a device is enabled
	bool isDeviceEnabled( InputDevice device )				{ return (m_enabledDevices & device) == device ? true : false ; }

public:
	/// Minimum range of a joystick axis (default is -1000)
	static int JOYSTICK_AXIS_RANGE_MIN; 

	/// Maximum range of a joystick axis (default is 1000)
	static int JOYSTICK_AXIS_RANGE_MAX;

	/// Percentage of dead zone in the joystick axis (percentage). Default is 1 (as in 1 percent)
	static int JOYSTICK_DEAD_ZONE;

private:
	/// Specifies which devices are enabled. By default, the input system
	/// attempts to enable all devices.
	InputDevice		m_enabledDevices;
};

KIMPLEMENT_SCRIPT( InputSystem );

} // Katana

#endif