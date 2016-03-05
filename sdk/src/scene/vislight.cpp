/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		vislight.cpp
	Author:		Eric Bryant

	A Visible Light Source. This aggregates a light (see Light.h) and derives from Visible
	in order to be placed within the scene graph. Also, as a result, it can be animated by
	attaching a KeyframeController.
*/

#include <math.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "visible.h"
#include "camera.h"
#include "vislight.h"
#include "visnode.h"
#include "scenecontext.h"
#include "engine/debugoutput.h"
#include "render/rendertypes.h"
#include "render/render.h"
#include "render/geometry.h"
#include "render/renderstate.h"
#include "render/light.h"
#include "system/systemfile.h"

//
// Externally defined structures
// 
extern shared_ptr<DebugOutput> katana_debug;

//
// RTTI declaration
//
KIMPLEMENT_RTTI( VisLight, Visible );

//
// Constructor
//
VisLight::VisLight() :
	m_attachLightToParent( false )
{
}

VisLight::VisLight( shared_ptr<Light> light ) :
	m_attachLightToParent( false )
{
	setLight( light );
}

//
// setLight
//
void VisLight::setLight( shared_ptr<Light> light )
{
	if ( light )
	{
		// By default, we set our position to the light's local position
		setTranslation( light->getPosition() );
	}

	// Store the light internally
	m_light = light;
}

//
// OnAttach
//
bool VisLight::OnAttach(SceneContext * context)
{
	// If we have a parent, add our light to it. This makes it so
	// our light will affect our parent's children
	if ( !m_parent.expired() && m_light && !m_attachLightToParent )
	{
		m_attachLightToParent = true;
		m_parent.lock()->addLight( m_light );
	}

	return true;
}

//
// OnDetach
//
bool VisLight::OnDetach(SceneContext * context)
{
	// When we detach ourselves, remove our light so it won't
	// affect our parent's children anymore
	if ( !m_parent.expired() && m_light )
	{
		m_parent.lock()->removeLight( m_light );
	}

	m_attachLightToParent = false;

	return true;
}

//
// OnPreRender
//
bool VisLight::OnPreRender(SceneContext * context)
{
	// If we have a parent, add our light to it. This makes it so
	// our light will affect our parent's children
	if ( !m_parent.expired() && m_light && !m_attachLightToParent )
	{
		m_attachLightToParent = true;
		m_parent.lock()->addLight( m_light );
	}

	// Cache the dirty parameter because it will be cleared during OnPreRender()
	bool isDirty = m_isDirty;

	// Call the base class to update our world transform
	if ( !Visible::OnPreRender( context ) )
		return false;

	// If we aggregate a light source, update it's position and orientation
	// with our own transform (by only if our transform was changed)
	if ( m_light && isDirty )
	{
		m_light->setPosition( m_worldViewMatrix.pos );
	}

	return true;
}

//
// OnRender
//
bool VisLight::OnRender(SceneContext * context)
{
	return true;
}

//
//
//
bool VisLight::OnPostRender(SceneContext * context)
{
	// Display the light
	if ( katana_debug->getDrawLights() ) 
	{
		// Determine the type of light and draw it
		if ( KIsExactlyFromClass<SpotLight>( m_light.get() ) )
			Debug_DrawSpotLight(context);
		else if ( KIsExactlyFromClass<DirectionLight>( m_light.get() ) )	
			Debug_DrawDirectionalLight(context);
		else
			Debug_DrawPointLight(context);
	}

	return true;
}

//
// Debug_DrawPointLight
//
void VisLight::Debug_DrawPointLight(SceneContext * context)
{
	// Lights are always rendered in camera space
	context->currentRenderer->SetMatrix( MODELVIEW, STORE, context->currentCamera->getWorldMatrix() );

	// Compute the dimensions of our circles
	float radius = m_light->getRange();
	Point3 position = m_light->getPosition();

	// This geometry define the circles which represent our light
	Geometry renderGeom;
	renderGeom.m_primitiveType = LINES;
	renderGeom.m_enabledBuffers = VERTEX | COLOR;
	renderGeom.m_primitiveCount = (360 / 10) * 2;
	renderGeom.m_vertexBuffer.reset( new vector<float> );

	// Choose the highest component of the light color as the light color
	long lightColor = CreateColor( 1, 1, 1, 1 );

	ColorA rawLightColor = m_light->getDiffuse();
	if ( rawLightColor.r > rawLightColor.g && rawLightColor.r > rawLightColor.b )
		lightColor = CreateColor( 1, 0, 0, 1 );
	if ( rawLightColor.g > rawLightColor.r && rawLightColor.g > rawLightColor.b )
		lightColor = CreateColor( 0, 1, 0, 1 );
	if ( rawLightColor.b > rawLightColor.r && rawLightColor.b > rawLightColor.g )
		lightColor = CreateColor( 0, 0, 1, 1 );

	// Conver the Light Color
	float LightColor = kmath::ltof( lightColor );

	// Increment around the 360 degree circle (stepping by 10 degrees)
	// and caclculting the points.
	for( int degree = 0; degree < 360; degree += 10 )
	{
		float x = (float)cos( kmath::toRadians( (float)degree ) ) * radius;
		float y = (float)sin( kmath::toRadians( (float)degree ) ) * radius;
		float x2 = (float)cos( kmath::toRadians( (float)degree+10 ) ) * radius;
		float y2 = (float)sin( kmath::toRadians( (float)degree+10 ) ) * radius;

		renderGeom.m_vertexBuffer->push_back( x + position.x );
		renderGeom.m_vertexBuffer->push_back( 0 );
		renderGeom.m_vertexBuffer->push_back( y - position.z );
		renderGeom.m_vertexBuffer->push_back( LightColor );
		renderGeom.m_vertexBuffer->push_back( x2 + position.x );
		renderGeom.m_vertexBuffer->push_back( 0 );
		renderGeom.m_vertexBuffer->push_back( y2 - position.z );
		renderGeom.m_vertexBuffer->push_back( LightColor );

		renderGeom.m_vertexBuffer->push_back( x + position.x );
		renderGeom.m_vertexBuffer->push_back( y + position.y );
		renderGeom.m_vertexBuffer->push_back( -position.z );
		renderGeom.m_vertexBuffer->push_back( LightColor );
		renderGeom.m_vertexBuffer->push_back( x2 + position.x );
		renderGeom.m_vertexBuffer->push_back( y2 + position.y );
		renderGeom.m_vertexBuffer->push_back( -position.z );
		renderGeom.m_vertexBuffer->push_back( LightColor );
	}

	// Render the geometry
	context->currentRenderer->RenderGeometry( &renderGeom );
}

//
// Debug_DrawSpotLight
//
void VisLight::Debug_DrawSpotLight(SceneContext * context)
{
	// TODO
}

//
// Debug_DrawDirectionalLight
//
void VisLight::Debug_DrawDirectionalLight(SceneContext * context)
{
	// TODO
}

// -------------------------------------------------------


//
// OnLoadStream
//
bool VisLight::OnLoadStream( kistream & istr )
{
	// Call base class
	Visible::OnLoadStream( istr );

	// Load the refernece to the light
	istr >> m_light;

	return true;
}

//
// OnSaveStream
//
bool VisLight::OnSaveStream( kostream & ostr ) const
{
	// Call base class
	Visible::OnSaveStream( ostr );

	// Save the reference to the light
	ostr << m_light;

	return true;
}