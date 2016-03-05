/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		camera.cpp
	Author:		Eric Bryant

	A display camera. The SceneGraph can handle multiple camera (which is
	aggregated by the game engine), however, only one camera is rendered
	at a time per scene.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "visible.h"
#include "camera.h"

//
// RTTI declaration
//
KIMPLEMENT_RTTI( Camera, Visible );

//
// Constructor
//
Camera::Camera()
{
	m_worldViewMatrix.setIdentity();
}

//
// Constructor with defaults
//
Camera::Camera(float fov, float nearp, float farp, float aspect) :
	m_fov(fov), m_nearPlane(nearp), m_farPlane(farp), m_aspectRatio(aspect)
{
	m_fovy = fov * aspect;
	kmath::createProjection( fov, nearp, farp, aspect, m_projectionMatrix );
}

//
// Cull
//
bool Camera::Cull( const Bound & bound ) const
{
	int iPositivePlanes = 0;
	for( int i = 0; i < MAX_FRUSTUM_PLANES; i++ )
	{
		if ( m_planes[i].whichSide( bound ) != Plane::SIDE_BACK ) 
			iPositivePlanes++;
	}

	if ( iPositivePlanes != MAX_FRUSTUM_PLANES ) 
		return false;
	else
		return true;
}

bool Camera::Cull( const Bound & bound, IntersectionType & intersection ) const
{
	intersection = INTERSECTION_NONE;
	int iPositivePlanes = 0;
	for( int i = 0; i < MAX_FRUSTUM_PLANES; i++ )
	{
		if ( m_planes[i].whichSide( bound ) != Plane::SIDE_BACK ) 
			iPositivePlanes++;
		else
			return false;
	}

	if ( iPositivePlanes == MAX_FRUSTUM_PLANES ) 
		intersection = INTERSECTION_FULL;
	else
		intersection = INTERSECTION_PARTIAL;

	return true;
}

//
// OnPreRender
//
bool Camera::OnPreRender(SceneContext * context)
{
	// Cache whether the camera's position was dirty. Visible::OnPreRender
	// will set the camera to undirty.
	bool wasDirty = m_isDirty;

	// Call the base class. This will update the worldview matrix.
	Visible::OnPreRender( context );

	// If the camera has moved, update the clip planes
	if ( wasDirty ) updateClipPlanes();

	return true;
}

//
// updateClipPlanes
//
void Camera::updateClipPlanes()
{
	// Concatenate the modelview and projection matrices
//	Matrix4 clipMatrix = m_projectionMatrix * m_worldViewMatrix;
	Matrix4 clipMatrix = m_projectionMatrix;

	// Extract the Frustum Planes from the Concatenated Projection and WorldView Matrix
	// Using the notation Mi to represent row i of the Matrix M of the Projection matrix, we have
	// the following formaulae to determine the planes:
	//		Near	= M4 + M3
	//		Far		= M4 - M3
	//		Left	= M4 + M1
	//		Right	= M4 - M1
	//		Bottom	= M4 + M2
	//		Top		= M4 - M2

	m_planes[FRUSTUM_RIGHT].set(	clipMatrix.s[ 3] - clipMatrix.s[ 0],
									clipMatrix.s[ 7] - clipMatrix.s[ 4],
									clipMatrix.s[11] - clipMatrix.s[ 8],
									clipMatrix.s[15] - clipMatrix.s[12] );

	m_planes[FRUSTUM_LEFT].set(		clipMatrix.s[ 3] + clipMatrix.s[ 0],
									clipMatrix.s[ 7] + clipMatrix.s[ 4],
									clipMatrix.s[11] + clipMatrix.s[ 8],
									clipMatrix.s[15] + clipMatrix.s[12] );

	m_planes[FRUSTUM_BOTTOM].set(	clipMatrix.s[ 3] + clipMatrix.s[ 1],
									clipMatrix.s[ 7] + clipMatrix.s[ 5],
									clipMatrix.s[11] + clipMatrix.s[ 9],
									clipMatrix.s[15] + clipMatrix.s[13] );

	m_planes[FRUSTUM_TOP].set(		clipMatrix.s[ 3] - clipMatrix.s[ 1],
									clipMatrix.s[ 7] - clipMatrix.s[ 5],
									clipMatrix.s[11] - clipMatrix.s[ 9],
									clipMatrix.s[15] - clipMatrix.s[13] );

	m_planes[FRUSTUM_FAR].set( 		clipMatrix.s[ 3] - clipMatrix.s[ 2],
									clipMatrix.s[ 7] - clipMatrix.s[ 6],
									clipMatrix.s[11] - clipMatrix.s[10],
									clipMatrix.s[15] - clipMatrix.s[14] );

	m_planes[FRUSTUM_NEAR].set(		clipMatrix.s[ 3] + clipMatrix.s[ 2],
									clipMatrix.s[ 7] + clipMatrix.s[ 6],
									clipMatrix.s[11] + clipMatrix.s[10],
									clipMatrix.s[15] + clipMatrix.s[14] );

	// Normalize the Frustum Planes
	m_planes[FRUSTUM_RIGHT]	.normalize();
	m_planes[FRUSTUM_LEFT]	.normalize();
	m_planes[FRUSTUM_BOTTOM].normalize();
	m_planes[FRUSTUM_TOP]	.normalize();
	m_planes[FRUSTUM_FAR]	.normalize();
	m_planes[FRUSTUM_NEAR]	.normalize();
}