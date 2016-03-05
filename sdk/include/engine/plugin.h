/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		plugin.h
	Author:		Eric Bryant

	Enables clients to register "plugins" into the game engine, which
	take part in the normal game update/render routine.
*/

#ifndef _PLUGIN_H
#define _PLUGIN_H

namespace Katana
{

//
// Forward Declaration
//
class GameEngine;

///
/// Plugin
///
class Plugin 
	: public RefCount, public RTTI
{
	KDECLARE_RTTI;

public:
	/// Constructor
	Plugin()					{}
	
	/// Destructor
	virtual ~Plugin()			{}

	/// Event is called when the game engine requests the plugin to initalize itself.
	/// Return FALSE if you want to stop registration.
	virtual bool OnInitialize( GameEngine * game ) = 0;

	/// Event is called when the game engine is about to terminate the plugin.
	/// Note there is no return value because it is non-preemptive.
	virtual void OnTerminate() = 0;

	/// Retrieve the friendly name of the plugin (used for lookups)
	virtual kstring getPluginName() = 0;
};

} // Katana

#endif // _PLUGIN_H