/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		inputmessages.h
	Author:		Eric Bryant

	Messages which the input system can send to listeners
*/

#ifndef _INPUTMESSAGES_H
#define _INPUTMESSAGES_H

namespace Katana
{

/// Constraints on the input device parameters
enum
{
	MAX_KEYS = 256,
	MAX_MOUSE_BUTTONS = 8,
	MAX_JOYSTICK_BUTTONS = 128,
};

/// Button constants
enum
{
	BUTTON_1 = 0,
	BUTTON_2 = 1,
	BUTTON_3 = 2,
	BUTTON_4 = 3,
	BUTTON_5 = 4,
	BUTTON_6 = 5,
	BUTTON_7 = 6,
	BUTTON_8 = 7,
};

///
/// InputMessage_Keyboard
/// Message sent by the input system in response to a keyboard event
///
struct InputMessage_Keyboard : public Message
{
	/// Constructor
	InputMessage_Keyboard( std::bitset<MAX_KEYS> in ) : keys(in) {}

public:
	std::bitset<MAX_KEYS> keys;	// Keyboard keys

public:
	KDECLARE_RTTI;
};

///
/// InputMessage_Mouse
/// Message sent by the input system in response to a mouse event
///
struct InputMessage_Mouse : public Message
{	
	/// Constructor
	InputMessage_Mouse( short ix, short iy, short iz, std::bitset<MAX_MOUSE_BUTTONS> in ) : x(ix), y(iy), z(iz), buttons(in) {}

public:
	short x, y, z;								/// Mouse axis
	std::bitset<MAX_MOUSE_BUTTONS> buttons;		/// Mouse buttons

public:
	KDECLARE_RTTI;
};

///
/// InputMessage_Joystick
/// Message sent by the input system in response to a input event
///
struct InputMessage_Joystick : public Message
{
	/// Constructor
	InputMessage_Joystick( short ix, short iy, short iz, 
						   short ixr, short iyr, short izr,
						   short ipov, std::bitset<MAX_JOYSTICK_BUTTONS> in ) 
		: x(ix), y(iy), z(iz), xr(ixr), yr(iyr), zr(izr), pov(ipov), buttons(in) {}

public:
	short x, y, z, xr, yr, zr;					/// Joystick Axis
	short pov;									/// POV hat
	std::bitset<MAX_JOYSTICK_BUTTONS> buttons;	/// Joystick buttons

public:
	KDECLARE_RTTI;
};


} // Katana

#endif // _INPUTMESSAGES_H