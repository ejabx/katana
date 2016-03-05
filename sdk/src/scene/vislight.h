/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		vislight.h
	Author:		Eric Bryant

	A Visible Light Source. This aggregates a light (see Light.h) and derives from Visible
	in order to be placed within the scene graph. During the VisLight's OnPreRender, it
	replaces the aggregated light's position with its position. As a result, lights can be animationed
	by attaching a KeyframeController.
*/

#ifndef _VISLIGHT_H
#define _VISLIGHT_H

namespace Katana
{

//
// Forward Declarations
//
class Light;

///
/// VisLight
/// A Light Source within the scene graph. It contains a Light, which updated within
/// the parent during it's OnPreRender().
///
class VisLight : public Visible
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(VisLight);
	KDECLARE_SCRIPT;

public:
	/// Constructor
	VisLight();

	/// Constructor which takes a light source
	VisLight( shared_ptr<Light> light );

	/// Sets the internal light source
	void setLight( shared_ptr<Light> light );

	/// Retrieves the internal light source
	shared_ptr<Light> getLight()					{ return m_light; }

protected:

	/// VisLight will attach it's light to the parent VisNode via VisNode::addLight()
	virtual bool OnAttach(SceneContext * context);

	/// VisLight will detach it's light to the parent VisNode via VisNode:removeLight()
	virtual bool OnDetach(SceneContext * context);

	/// VisLight will store it's world coordinates within the aggregated light
	virtual bool OnPreRender(SceneContext * context);

	/// VisLight will render itself if light drawing is enabled in the debug options
	virtual bool OnRender(SceneContext * context);

	/// VisLight will optionally draw the light as two overlapping circles
	virtual bool OnPostRender(SceneContext * context);

private:

	/// Draw the point light
	void Debug_DrawPointLight(SceneContext * context);

	/// Draw the spot light
	void Debug_DrawSpotLight(SceneContext * context);

	/// Draw the directional light
	void Debug_DrawDirectionalLight(SceneContext * context);

protected:
	/// The aggregated light source
	shared_ptr<Light>	m_light;

	/// Flag which indicates whether we've add our light to the parent's
	/// list of lights so it's children are affected by our light.
	bool			m_attachLightToParent;
};

KIMPLEMENT_STREAM( VisLight );
KIMPLEMENT_SCRIPT( VisLight );

}; // Katana

#endif // _VISLIGHT_H