/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		katana_config.h
	Author:		Eric Bryant

	Build Configuration Parameters
*/

#ifndef _KATANA_CONFIG_H_
#define _KATANA_CONFIG_H_


// Define this paramater if you want Katana to define a default windows. If undefined,
// it is up to the client to create a window (if applicable).
#define APPLICATION_CREATE_DEFAULT_WINDOW


// ODE (Open Dynamic Engine) requies that we define whether we're using single or double percision.
// We use dSINGLE because dDOUBLE require double for all vectors, and we currently use floats (see Math/Point3)
#define dSINGLE


// Luabind will not generate exceptions during errors
#define LUABIND_NO_EXCEPTIONS


// Define one of these parameters to determine which Physics SDK to use. Currently, the choices are 
// TOKAMAK and ODE (Open Dynamics Engine). You must define one (and only one) of these defines; currently
// there is no option to NOT use a Physics Engine.
#define PHYSICS_USE_TOKAMAK
//#define PHYSICS_USE_ODE


// Define these paramters to determine which render systems to enable. They are displayed during the
// initial dialog to allow clients to select the render device
#define RENDER_NULL_DEVICE
#define RENDER_DIRECTX8_DEVICE
#define RENDER_DIRECTX9_DEVICE


#endif // _KATANA_CONFIG_H_