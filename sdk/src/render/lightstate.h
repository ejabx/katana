/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		lightstate.h
	Author:		Eric Bryant

	Sets the renderer into hardware lighting mode. This is dependent on the driver
	for support (DX8,OGL), and is usually restricted to 8 hardware lights. To get more
	lights, try vertex/pixel shaders.
*/

#ifndef _LIGHTSTATE_H
#define _LIGHTSTATE_H

namespace Katana
{

// Forward declarations
class Light;

///
/// LightState
/// Use this RenderState to enable hardware lighting mode
///
class LightState : public RenderState
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( LightState );
	KDECLARE_SCRIPT;

public:
	/// Constructor
	LightState();

	/// Constructor which takes an array of lights
	LightState( vector< shared_ptr<Light> > & lights )			{ m_lights = lights; m_enableLights = true; }

	/// Returns the collection of lights in the lightstate
	const vector< shared_ptr<Light> > & getLights() const		{ return m_lights; }

	/// Returns the light status
	bool getEnableLights() const								{ return m_enableLights; }

public:
	/// Called before this render state is set within the renderer. This method
	/// is used to determine grab all the lights from the parent in order
	/// to route them to the renderer.
	virtual bool OnPreRender( SceneContext * context );

	/// Called after this render state is set within the renderer. This method
	/// is used to disable all the lights in the renderer
	virtual bool OnPostRender( SceneContext * context );

private:
	/// Collection of lights extracted from the parent node and
	/// will be applied to a scene. This array is cleared and
	/// populated during each prerender event.
	vector< shared_ptr<Light> >	m_lights;

	/// Internal flag which determines whether we're enabling the lights or disabling them
	bool			m_enableLights;
};

KIMPLEMENT_STREAM( LightState );
KIMPLEMENT_SCRIPT( LightState );

} // Katana


#endif // _LIGHTSTATE_H