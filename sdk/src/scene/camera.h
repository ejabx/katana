/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		camera.h
	Author:		Eric Bryant

	A display camera. The SceneGraph can handle multiple camera (which is
	aggregated by the game engine), however, only one camera is rendered
	at a time per scene.
*/

#ifndef _CAMERA_H
#define _CAMERA_H

namespace Katana
{

///
/// Camera
/// Displayable camera.
///
class Camera : public Visible
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:
	/// Constructor
	Camera();

	/// Camera with defaults
	Camera(float fov, float nearp, float farp, float aspect);

	/// Retrieve the up vector (from the world matrix)
	Point3 getUp() const							{ return Point3( m_worldViewMatrix.up[0], m_worldViewMatrix.up[1], m_worldViewMatrix.up[2] ); }

	/// Retrieve the right vector (from the world matrix)
	Point3 getRight() const							{ return Point3( m_worldViewMatrix.right[0], m_worldViewMatrix.right[1], m_worldViewMatrix.right[2] ); }

	/// Retrieve the look at vector (from the world matrix)
	Point3 getAt() const							{ return Point3( m_worldViewMatrix.at[0], m_worldViewMatrix.at[1], m_worldViewMatrix.at[2] ); }

	/// Retrive the position vector (from the world matrix)
	Point3 getPos() const							{ return Point3( m_worldViewMatrix.pos[0], m_worldViewMatrix.pos[1], m_worldViewMatrix.pos[2] ); }

	/// Retrieve the project matrix (computed during the constructor)
	const Matrix4 & getProjection() const			{ return m_projectionMatrix; }

	/// Checks whether the bounding volume is culled by the camera's view frustum.
	/// This function returns true if the bounding volume is full or partially culled
	/// by this camera's frustum, and false if there is no intersection.
	bool Cull( const Bound & bound ) const;

	/// Checks whether the bounding volume is culled by the camera's view frustum.
	/// This function returns true if the bounding volume is full or partially culled
	/// by this camera's frustum, and false if there is no intersection. This function
	/// returns the intersection type
	bool Cull( const Bound & bound, IntersectionType & intersection ) const;

	/// Returns the given plane
	Plane getClipPlanes( FrustumPlanes planeIndex )	const { return m_planes[planeIndex]; }

public:

	/// OnPreRender will recalculate the clip planes. This operation happens during OnPreRender
	/// instead of OnUpdate because Visible::OnPreRender is when the world matrices are recalculated
	virtual bool OnPreRender(SceneContext * context);

protected:

	// Updates the clipping planes from the projective and modelview matrices
	void updateClipPlanes();

protected:
	/// Standard camera parameters
	float	m_fov, m_fovy;
	float	m_nearPlane;
	float	m_farPlane;
	float	m_aspectRatio;
	Matrix4	m_projectionMatrix;
	Plane	m_planes[MAX_FRUSTUM_PLANES];
};

KIMPLEMENT_SCRIPT( Camera );

}; // Katana

#endif // _CAMERA_H