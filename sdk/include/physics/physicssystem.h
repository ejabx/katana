/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		physicssystem.h
	Author:		Eric Bryant

	Responsible for resolving rigid bodies within the physics sytem
*/

#ifndef _PHYSICSSYSTEM_H
#define _PHYSICSSYSTEM_H

namespace Katana
{

// Forward Declarations
class RigidBody;
struct Geometry;
class Collidable;

///
/// RaycastCallback
/// Callback invoked when a ray hits a target
///
typedef void (*RaycastCallback)( const Point3 & hitPoint, const Point3 & hitNormal, shared_ptr<Collidable> hitRB );

///
/// PhysicsSystem
///
class PhysicsSystem 
	: public MessageRouter
{
	KDECLARE_SCRIPT;

public:
	/// Initialize the physics system
	bool initialize();

	/// Terminates the physics system
	bool terminate();

	/// Steps the simulations forward by deltaTime
	void integrate( float deltaTime );

public:

	/// Casts a ray and invokes the callback when the ray intersects with a RigidBody
	void castRay( const Point3 & rayStart, const Point3 & rayEnd, RaycastCallback pfRayCB );

public:
	/// Adds a rigid body to the simulation and returns a reference to it
	shared_ptr<RigidBody> addRigidBody( bool bFixed );

	/// Removes a rigid body from the simulation
	void removeRigidBody( shared_ptr<RigidBody> & spRigidBody );

	/// Creation functions for various geometries. It will associate the created geometry with the RigidBody
	void createBoxGeometry( shared_ptr<RigidBody> & spRigidBody, float fExtentX, float fExtentY, float fExtentZ );
	void createSphereGeometry( shared_ptr<RigidBody> & spRigidBody, float fRadius );
	void createCylinderGeometry( shared_ptr<RigidBody> & spRigidBody, float fRadius, float fHeight );
	void createMeshGeometry( shared_ptr<RigidBody> & spRigidBody, shared_ptr<Geometry> & spGeometry );
	void createPlaneGeometry( shared_ptr<RigidBody> & spRigidBody, const Point3 & normal, float fDistance );
};

KIMPLEMENT_SCRIPT( PhysicsSystem );

}; // Katana

#endif // _PHYSICSSYSTEM_H