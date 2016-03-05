/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		visible.cpp
	Author:		Eric Bryant

	A Visible object in the Game Engine (which can be displayed by the
	scene manager). It contains visibility information (such as local space,
	textures, etc.)
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "render/rendertypes.h"
#include "render/render.h"
#include "render/shader.h"
#include "render/light.h"
#include "render/material.h"
#include "physics/collidable.h"
#include "physics/rigidbody.h"
#include "system/systemfile.h"
#include "engine/debugoutput.h"
#include "animation/animation.h"
#include "scenecontext.h"
#include "visible.h"
#include "visnode.h"
#include "camera.h"

//
// RTTI declaration
//
KIMPLEMENT_RTTI( Visible, Streamable );

//
// Constructor
//
Visible::Visible() 
	: m_isVisible( true )
	, m_isDirty( true )
	, m_frameCount( 0 )
	, m_renderPass( 0 )
	, m_scale( 1 )
	, m_isShadowCaster( false )
	, m_isBillboard( false )
{
	m_worldViewMatrix.setIdentity();
}

Visible::Visible( shared_ptr<VisNode> parent) 
	: m_parent( parent )
	, m_isVisible( true )
	, m_isDirty( true )
	, m_frameCount( 0 )
	, m_renderPass( 0 )
	, m_scale( 1 )
	, m_isShadowCaster( false )
	, m_isBillboard( false )
{
	m_worldViewMatrix.setIdentity();
}

//
// setTransform
//
void Visible::setTransform(const Point3 & trans, const Quaternion & rot)
{
	// Store the new translation, rotation
	m_translation = trans;
	m_rotation = rot;
	m_isDirty = true;
}

//
// OnAttach
//
bool Visible::OnAttach(SceneContext * context)
{
	// If we have a Rigid Body, set its starting position and orientation to our transform
	if ( m_spRigidBody )
	{
		m_spRigidBody->setPosition( m_translation );
		m_spRigidBody->setRotation( m_rotation );
	}

	return true;
}

//
// OnUpdate
//
bool Visible::OnUpdate( SceneContext * context )
{
	// If this Visible Object has a Rigid Body associate with it,
	// synchronize our position and orientation from the rigid body
	if ( m_spRigidBody && !m_spRigidBody->isFixed() )
	{
		m_translation	= m_spRigidBody->getPosition();
		m_rotation		= m_spRigidBody->getRotation();
		m_isDirty		= true;
	}
	// Otherwise, if we have an animation tell it to advance the animation
	// forward and apply the resultant keyframe to our position, orientation
	else if ( m_animation && m_animation->getEnabled() )
	{
		// The advance function will return TRUE if there was a 
		// positional/rotational change, or false otherwise
		m_isDirty = m_animation->advance( context->deltaTime, m_translation, m_rotation );
	}

	// If the visible object is billboarded, then orientate it towards the camera
	// in all axis
	if ( m_isBillboard && context->currentCamera )
	{
		// Grab the world orientation of the camera
		Matrix4 cameraWorldMatrix = context->currentCamera->getWorldMatrix();

		// Determine the at vector. It is pointing it the oppositive direction the camera is facing
		Matrix4 billboardRotation;
		billboardRotation.at = -cameraWorldMatrix.at;

		// Calculate the right and up directions
		billboardRotation.right = billboardRotation.at.getCross( Point3( 0, 1, 0 ) ).getNormalized(); // Up
		billboardRotation.up = billboardRotation.at.getCross( billboardRotation.right ).getNormalized();

		// Update our orientation by this transformation
		m_rotation.fromMatrix( billboardRotation );
	}

	return true;
}

//
// OnPreRender
//
bool Visible::OnPreRender(SceneContext * context)
{
	// If this object isn't visible, exit
	if ( !m_isVisible )
		return false;
/*
	// If this object has already been rendering this frame,
	// don't bother to render it again
	if ( m_frameCount == context->frameCount &&
		 m_renderPass == context->renderPass )
		return false;
*/

	// Only recalculate the world view matrix if we're dirty
	if ( m_isDirty || ( !m_parent.expired() && m_parent.lock()->isDirty() ) || context->currentCamera->isDirty() )
	{
		// If we have a parent, transform our WTM relative to our parent's
		if ( !m_parent.expired() )
		{
			// Get the parent
			shared_ptr<VisNode> spParent = m_parent.lock();

			// Multiply this World View Matrix by the parent's matrix
			// to get the concatenated version
			Matrix4 parentRotationMatrix;
			spParent->getRotation().toMatrix( parentRotationMatrix );
			Matrix4 localRotationMatrix;
			m_rotation.toMatrix( localRotationMatrix );

			m_worldViewMatrix = parentRotationMatrix.transpose() * localRotationMatrix;
			m_worldViewMatrix.pos = spParent->m_worldViewMatrix.pos + 
				( m_translation * spParent->m_worldViewMatrix );
		}
		// Otherwise store our translation and rotation in the WTM
		else
		{
			// Store the rotation matrix
			m_rotation.toMatrix( m_worldViewMatrix );

			// Add the translation to the matrix
			m_worldViewMatrix.pos = m_translation;

			// Apply the uniform local scale to the matrix
			// TODO: Currently, the scale skews the rotation, we're figure it out and fix it later.
			//		m_worldViewMatrix.m[0][0] = m_scale;
			//		m_worldViewMatrix.m[1][1] = m_scale;
			//		m_worldViewMatrix.m[2][2] = m_scale;
		}

		// Now that this object is in world space, transform it into
		// view (camera) space.
		m_worldViewMatrix *= context->currentCamera->getWorldMatrix();

		// Update the world bounds, which is basically the local bounds
		// transformed by our world matrix
		m_worldBound = m_localBound;
		m_worldBound.transform( m_worldViewMatrix );

		// If this Visible Object has a light associated with it,
		// update it's position with our world position
		if ( m_light ) {
			m_light->setPosition( m_worldViewMatrix.pos );

			// Also, if necessary, enlarge the radius of the world bounds
			// to incorporate the light range. This is because the light
			// will not affect the scene if this object is culled out.
			if ( m_worldBound.getRadius() < m_light->getRange() )
				m_worldBound.m_radius += m_light->getRange();
		}
	}

	// Perform frustum intersection culling. This must be done after the world bounds has been transformed
	if ( context->debugOutput->getEnableFrustumCulling() && 
		 m_worldBound.getRadius() &&							// If this visible object's radius is 0, assume it is always visible
		 !context->currentCamera->Cull( m_worldBound ) )
		return false;

	// Store our frame index
	m_frameCount = context->frameCount;

	return true;
}

//
// OnPostRender
//
bool Visible::OnPostRender(SceneContext * context)
{
	// Undirty ourselves. Note this is done AFTER OnPreRender(), otherwise,
	// children Visible objects will not know to recalculate the WTM because
	// their parent is dirty.
	m_isDirty = false;

	return true;
}

// -------------------------------------------------------


//
// OnLoadStream
//
bool Visible::OnLoadStream( kistream & istr )
{
	// Load the reference to the parent
	istr >> m_parent.lock();

	// Load the revelant member variables
	istr >> m_isVisible;
	istr >> m_translation;
	istr >> m_rotation;
	istr >> m_scale;
	istr >> m_localBound;
	istr >> m_animation;
	istr >> m_material;
	istr >> m_light;
	istr >> m_isShadowCaster;
	istr >> m_isBillboard;

	return true;
}

//
// OnSaveStream
//
bool Visible::OnSaveStream( kostream & ostr ) const
{
	// Save the reference to the parent, ONLY if isn't non-null and
	// this is a root object (see kostream.h for the definition)
	if ( !m_parent.expired() && ostr.isRoot( *this ) )	ostr << m_parent.lock();
	else												ostr << 0L;

	// Save the revelant member variables
	ostr << m_isVisible;
	ostr << m_translation;
	ostr << m_rotation;
	ostr << m_scale;
	ostr << m_localBound;
	ostr << m_animation;
	ostr << m_material;
	ostr << m_light;
	ostr << m_isShadowCaster;
	ostr << m_isBillboard;

	return true;
}