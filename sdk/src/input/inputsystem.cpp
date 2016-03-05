/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		inputsystem.cpp
	Author:		Eric Bryant

	Responsible for quering various input devices
*/

// Use DirectX 8 Input 
#define DIRECTINPUT_VERSION 0x0800

#include <bitset>
#include <dinput.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "inputsystem.h"
#include "inputmessages.h"
#include "base/comptr.h"
#include "base/log.h"

// Make sure client includes DirectX8 libaries
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//
// Static Variables
//
int InputSystem::JOYSTICK_AXIS_RANGE_MIN	= -1000;
int InputSystem::JOYSTICK_AXIS_RANGE_MAX	= 1000;
int InputSystem::JOYSTICK_DEAD_ZONE			= 1;

//
// Definitions
//
#define PREVIOUS_STATE	0
#define CURRENT_STATE	1

//
// Local Variables
//
ComPtr<IDirectInput8>		g_DI;
ComPtr<IDirectInputDevice8>	g_keyboard;
ComPtr<IDirectInputDevice8>	g_mouse;
ComPtr<IDirectInputDevice8>	g_joystick;
DIJOYSTATE2					g_joyState[2];
DIMOUSESTATE2				g_mouseState[2];
char						g_keyState[2][256];

//
// Local Functions
//
BOOL CALLBACK __EnumJoysticksCallback( const DIDEVICEINSTANCE* pDIDInstance, void* pContext );
BOOL CALLBACK __EnumJoystickAxisCallback( const DIDEVICEOBJECTINSTANCE * pDIDInstance, void * pContext );

//
// InputMessages RTTI Definitions
KIMPLEMENT_RTTI( InputMessage_Keyboard, Message );
KIMPLEMENT_RTTI( InputMessage_Mouse, Message );
KIMPLEMENT_RTTI( InputMessage_Joystick, Message );

//
// Constructor
//
InputSystem::InputSystem() :
	m_enabledDevices( INPUT_KEYBOARD | INPUT_MOUSE | INPUT_JOYSTICK )
{
	// Zero out the states
	memset( &g_mouseState[PREVIOUS_STATE], 0, sizeof( DIMOUSESTATE2 ) );
	memset( &g_mouseState[CURRENT_STATE], 0, sizeof( DIMOUSESTATE2 ) );
}

//
// Destructor
//
InputSystem::~InputSystem()
{
	Terminate();
}

//
// Initialize
//
bool InputSystem::Initialize( void * handle )
{
	// Create the Direct Input Device
	HRESULT hr = DirectInput8Create(GetModuleHandle(NULL), 
									DIRECTINPUT_VERSION, 
									IID_IDirectInput8, 
									(VOID **)&g_DI, 
									NULL);
	if ( FAILED(hr) )
		return false;

	// Log
	KLOG("Input Initialization");

	// Create the Keyboard Device
	if ( !g_keyboard.isValid() && isDeviceEnabled( INPUT_KEYBOARD ) )
	{
		IDirectInputDevice8 * pDID;

		// Obtain the Interface to the system Keyboard Device
		HRESULT hr = g_DI->CreateDevice( GUID_SysKeyboard, &pDID, NULL );
		if ( FAILED(hr) )
			return false;

		// Store the Device
		g_keyboard = pDID;

		// Set the data format to "keyboard format" - a predefined data format 
		//
		// A data format specifies which controls on a device we
		// are interested in, and how they should be reported.
		//
		// This tells DirectInput that we will be passing an array
		// of 256 bytes to IDirectInputDevice::GetDeviceState.
		hr = g_keyboard->SetDataFormat( &c_dfDIKeyboard );
		if ( FAILED(hr) )
			return false;

		// Set the cooperativity level to let DirectInput know how
		// this device should interact with the system and with other
		// DirectInput applications.
		hr = g_keyboard->SetCooperativeLevel( (HWND)handle, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND );
		if ( FAILED(hr) )
			return false;

		// Acquire the newly created device
		g_keyboard->Acquire();

		// Log
		KLOG2("Keyboard Device");
	}

	// Create the mouse device
	if ( !g_mouse.isValid() && isDeviceEnabled( INPUT_MOUSE )  )
	{
		IDirectInputDevice8 * pDID;

		// Obtain the Interface to the system Mouse Device
		HRESULT hr = g_DI->CreateDevice( GUID_SysMouse, &pDID, NULL );
		if ( FAILED(hr) )
			return false;

		// Store the Device
		g_mouse = pDID;

		// Set the data format to "mouse format" - a predefined data format 
		//
		// A data format specifies which controls on a device we
		// are interested in, and how they should be reported.
		//
		// This tells DirectInput that we will be passing a
		// DIMOUSESTATE2 structure to IDirectInputDevice::GetDeviceState.
		hr = g_mouse->SetDataFormat( &c_dfDIMouse2 );
		if ( FAILED(hr) )
			return false;

		// Set the cooperativity level to let DirectInput know how
		// this device should interact with the system and with other
		// DirectInput applications.
		hr = g_mouse->SetCooperativeLevel( (HWND)handle, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND );
		if ( FAILED(hr) )
			return false;

		// Acquire the newly created device
		g_mouse->Acquire();

		// Log
		KLOG2("Mouse Device");
	}

	// Create the Joystick Device
	if ( !g_joystick.isValid() && isDeviceEnabled( INPUT_JOYSTICK )  )
	{
		// Enumerate through the Joysticks
		HRESULT hr = g_DI->EnumDevices( DI8DEVCLASS_GAMECTRL, 
										__EnumJoysticksCallback,
										NULL, 
										DIEDFL_ATTACHEDONLY );

		if ( FAILED(hr) )
			return false;

		// Did we return a valid joystick from the enumeration
		if ( !g_joystick )
			return false;

		// Determine the Joystick Capabilities
		DIDEVCAPS diCaps;
		diCaps.dwSize = sizeof(diCaps);
		hr = g_joystick->GetCapabilities(&diCaps);
		if ( FAILED(hr) )
			return false;

		// Set the data format to "simple joystick" - a predefined data format 
		//
		// A data format specifies which controls on a device we are interested in,
		// and how they should be reported. This tells DInput that we will be
		// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
		hr = g_joystick->SetDataFormat( &c_dfDIJoystick2 );
		if ( FAILED(hr) )
			return false;

		// Enumerate the axes of the joyctick and set the range of each axis.
		hr = g_joystick->EnumObjects( __EnumJoystickAxisCallback, 
									  NULL, 
									  DIDFT_AXIS );
		if ( FAILED(hr) )
			return false;

		// Set the dead zone of the joystick, which ranges from 0 through 10,000, where 0 
		// indicates that there is no dead zone, 5,000 indicates that the dead zone extends 
		// over 50 percent of the physical range of the axis on both sides of center, and 10,000 
		// indicates that the entire physical range of the axis is dead. When the axis is within 
		// the dead zone, it is reported as being at the center of its range.
		DIPROPDWORD diPdw;
		diPdw.diph.dwSize		= sizeof(DIPROPDWORD);
		diPdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		diPdw.diph.dwHow        = DIPH_DEVICE; 
		diPdw.diph.dwObj        = 0;
		diPdw.dwData			= InputSystem::JOYSTICK_DEAD_ZONE * 100; // It's a percentage

		hr = g_joystick->SetProperty( DIPROP_DEADZONE, &diPdw.diph );
		if ( FAILED(hr) )
			return false;

		// Acquire the newly created device
		g_joystick->Acquire();

		// Log
		KLOG2("Joystick Device");
	}

	return true;
}

//
// Terminate
//
bool InputSystem::Terminate()
{
	g_keyboard = 0;
	g_mouse = 0;
	g_joystick = 0;
	g_DI = 0;

	return true;
}

//
// Poll
//
bool InputSystem::Poll()
{
	// Poll the keyboard device
	if ( g_keyboard.isValid() && isDeviceEnabled( INPUT_KEYBOARD ) )
	{
		// Zero out the mouse state
		ZeroMemory( &g_keyState[CURRENT_STATE], sizeof(g_keyState[CURRENT_STATE]) );

		// DInput keyboard state
		HRESULT hr = g_keyboard->GetDeviceState( sizeof(g_keyState[CURRENT_STATE]), &g_keyState[CURRENT_STATE] );
		if ( FAILED(hr) )
		{
			// DirectInput may be telling us that the input stream has been
			// interrupted.  We aren't tracking any state between polls, so
			// we don't have any special reset that needs to be done.
			// We just re-acquire and try again.
    
			// If input is lost then acquire and keep trying 
			hr = g_keyboard->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = g_keyboard->Acquire();

			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of 
			// switching, so just try again later 
			return false;
		}

		// Check whether the current state is different from the previous state
		// and if so, send a message and sync states
		if ( memcmp( &g_keyState[PREVIOUS_STATE], 
					 &g_keyState[CURRENT_STATE], 
					 sizeof( g_keyState[CURRENT_STATE] ) ) != 0 )
		{
			// Set the keys based on the keyboard state
			std::bitset<MAX_KEYS> keys; keys.reset();

			for( int i = 0; i < MAX_KEYS; i++ )
			{
				if ( g_keyState[CURRENT_STATE][i] & 0x80 ) keys.set( i );
			}

			// Send the keyboard message
			Broadcast( &InputMessage_Keyboard( keys ) );

			// Sync up the states
			memcpy( &g_keyState[PREVIOUS_STATE], 
					&g_keyState[CURRENT_STATE], 
					sizeof( g_keyState[CURRENT_STATE] ) );
		}
	}

	// Poll the mouse device
	if ( g_mouse.isValid() && isDeviceEnabled( INPUT_MOUSE ) )
	{
		// Zero out the mouse state
		ZeroMemory( &g_mouseState[CURRENT_STATE], sizeof(DIMOUSESTATE2) );

		// Retrieve the Mouse's Device State
		HRESULT hr = g_mouse->GetDeviceState( sizeof(DIMOUSESTATE2), &g_mouseState[CURRENT_STATE] );
		if ( FAILED(hr) )
		{
			// DirectInput may be telling us that the input stream has been
			// interrupted.  We aren't tracking any state between polls, so
			// we don't have any special reset that needs to be done.
			// We just re-acquire and try again.

			// If input is lost then acquire and keep trying 
			hr = g_mouse->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = g_mouse->Acquire();

			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of 
			// switching, so just try again later
			return false;
		}

		// Check whether the current state is different from the previous state
		// and if so, send a message and sync states
		if ( memcmp( &g_mouseState[PREVIOUS_STATE], 
					 &g_mouseState[CURRENT_STATE], 
					 sizeof( DIMOUSESTATE2 ) ) != 0 )
		{
			// Set the buttons based on the mouse state
			std::bitset<MAX_MOUSE_BUTTONS> buttons; buttons.reset();

			for( int i = 0; i < MAX_MOUSE_BUTTONS; i++ )
			{
				if ( g_mouseState[CURRENT_STATE].rgbButtons[i] & 0x80 ) buttons.set( i );
			}

			// Send the mouse message
			Broadcast( &InputMessage_Mouse( (short)g_mouseState[CURRENT_STATE].lX, 
											(short)g_mouseState[CURRENT_STATE].lY, 
											(short)g_mouseState[CURRENT_STATE].lZ,
											buttons ) );

			// Sync up the states
			memcpy( &g_mouseState[PREVIOUS_STATE], 
					&g_mouseState[CURRENT_STATE], 
					sizeof( DIMOUSESTATE2 ) );
		}
	}

	// Poll the joystick device
	if ( g_joystick.isValid() && isDeviceEnabled( INPUT_JOYSTICK ) )
	{
		// Poll the device to read the current state
		HRESULT hr = g_joystick->Poll(); 
		if( FAILED(hr) ) 
		{
			// DInput is telling us that the input stream has been
			// interrupted. We aren't tracking any state between polls, so
			// we don't have any special reset that needs to be done. We
			// just re-acquire and try again.
			hr = g_joystick->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = g_joystick->Acquire();

			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of 
			// switching, so just try again later 
			return false; 
		}

		// Get the input's device state
		if( FAILED( hr = g_joystick->GetDeviceState( sizeof(DIJOYSTATE2), &g_joyState[CURRENT_STATE] ) ) )
			return false; // The device should have been acquired during the Poll()

		// Check whether the current state is different from the previous state
		// and if so, send a message and sync states
		if ( memcmp( &g_joyState[PREVIOUS_STATE], 
					 &g_joyState[CURRENT_STATE], 
					 sizeof( DIJOYSTATE2 ) ) != 0 )
		{
			// Set the buttons based on the mouse state
			std::bitset<MAX_JOYSTICK_BUTTONS> buttons; buttons.reset();

			for( int i = 0; i < MAX_JOYSTICK_BUTTONS; i++ )
			{
				if ( g_joyState[CURRENT_STATE].rgbButtons[i] & 0x80 ) buttons.set( i );
			}

			// Send the joystick message
			Broadcast( &InputMessage_Joystick(	(short)g_joyState[CURRENT_STATE].lX,
												(short)g_joyState[CURRENT_STATE].lY,
												(short)g_joyState[CURRENT_STATE].lZ,
												(short)g_joyState[CURRENT_STATE].lRx,
												(short)g_joyState[CURRENT_STATE].lRy,
												(short)g_joyState[CURRENT_STATE].lRz,
												(short)g_joyState[CURRENT_STATE].rgdwPOV[0],
												buttons ) );

			// Sync up the states
			memcpy( &g_joyState[PREVIOUS_STATE], 
					&g_joyState[CURRENT_STATE], 
					sizeof( DIJOYSTATE2 ) );
		}
	}

	return true;
}

//
// enableDevice
//
bool InputSystem::enableDevice( InputDevice device )
{
	m_enabledDevices |= device;

	switch( device )
	{
		case INPUT_KEYBOARD: 
		{
			if ( g_keyboard.isValid() ) 
				if ( SUCCEEDED( g_keyboard->Acquire() ) )
					return true;
		}
		case INPUT_MOUSE: 
		{
			if ( g_mouse.isValid() ) 
				if ( SUCCEEDED( g_mouse->Acquire() ) )
					return true;
		}
		case INPUT_JOYSTICK: 
		{
			if ( g_joystick.isValid() ) 
				if ( SUCCEEDED( g_joystick->Acquire() ) )
					return true;
		}
	}

	return false;
}

//
// disableDevice
//
bool InputSystem::disableDevice( InputDevice device )
{
	m_enabledDevices ^= device;

	switch( device )
	{
		case INPUT_KEYBOARD: 
		{
			if ( g_keyboard.isValid() ) 
				if ( SUCCEEDED( g_keyboard->Unacquire() ) )
					return true;
		}
		case INPUT_MOUSE: 
		{
			if ( g_mouse.isValid() ) 
				if ( SUCCEEDED( g_mouse->Unacquire() ) )
					return true;
		}
		case INPUT_JOYSTICK: 
		{
			if ( g_joystick.isValid() ) 
				if ( SUCCEEDED( g_joystick->Unacquire() ) )
					return true;
		}
	}

	return false;
}

//
// __EnumJoysticksCallback
//
BOOL CALLBACK __EnumJoysticksCallback( const DIDEVICEINSTANCE* pDIDInstance, void* pContext )
{
	// TODO: Be More Selective. For now, we'll just take the first joystick found
	IDirectInputDevice8 * pDID;

    // Obtain an interface to the enumerated joystick.
    HRESULT hr = g_DI->CreateDevice( pDIDInstance->guidInstance, &pDID, NULL );

    // If it failed, then we can't use this joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
	if ( FAILED(hr) )
        return DIENUM_CONTINUE;

	// Store the Joystick Device
	g_joystick = pDID;

    // Stop enumeration. Note: we're just taking the first joystick we get. You
    // could store all the enumerated joysticks and let the user pick.
    return DIENUM_STOP;
}

//
// __EnumJoystickAxisCallback
//
BOOL CALLBACK __EnumJoystickAxisCallback( const DIDEVICEOBJECTINSTANCE * pDIDInstance, void * pContext )
{
    DIPROPRANGE diPrg; 
    diPrg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diPrg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diPrg.diph.dwHow        = DIPH_BYID; 
    diPrg.diph.dwObj        = pDIDInstance->dwType; // Specify the enumerated axis
    diPrg.lMin              = InputSystem::JOYSTICK_AXIS_RANGE_MIN; 
    diPrg.lMax              = InputSystem::JOYSTICK_AXIS_RANGE_MAX;

	// Set the range for the axis
	if( FAILED( g_joystick->SetProperty( DIPROP_RANGE, &diPrg.diph ) ) )
		return DIENUM_STOP;

	// Continue the enumeration
    return DIENUM_CONTINUE;
}