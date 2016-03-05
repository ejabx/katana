/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		streamable.h
	Author:		Eric Bryant

	Interface for classes which can interact with the PhysicsSystem
*/

#ifndef _COLLIDABLE_H
#define _COLLIDABLE_H

namespace Katana
{

// Forward Declarations
struct Geometry;
class RigidBody;

///
/// CollideShape
///
enum CollideShape
{
	COLLIDE_BOX,
	COLLIDE_SPHERE,
	COLLIDE_CYLINDER,
	COLLIDE_MESH,
	COLLIDE_PLANE,
};

///
/// Collidable
/// Base class for all physics objects.
///
class Collidable 
	: public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:
	/// Destructor
	virtual ~Collidable();

	/// Generates a Physics representation of the Geometry
	void setCollideShape( shared_ptr<Geometry> spGeometry, CollideShape shapeType );

public:

	/// Creates an internal instance of the RigidBody, which defines the Physics representation
	/// of this object (mass, inertia, phyics geometry, etc.).
	///
	/// If bFixed, then we will create an AnimatedBody, which is keyframed and does NOT undergo
	/// rigid body motion. Otherwise, we will create a RigidBody
	void createRigidBody( bool bFixed );

	/// Retrieves the internal RigidBody
	shared_ptr<RigidBody> getRigidBody();

	/// Replaces the internal instance of the RigidBody
	void setRigidBody( shared_ptr<RigidBody> spRigidBody );

protected:
	shared_ptr<RigidBody>		m_spRigidBody;		/// Reference to the Internal Rigid Body

};

KIMPLEMENT_SCRIPT( Collidable );

} // Katana

#endif // _COLLIDABLE_H