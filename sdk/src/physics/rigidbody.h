/*
	Katana Engine
	Copyright © 2004 Eric Bryant, Inc.

	File:		rigidbody.h
	Author:		Eric Bryant

	A Rigid Body an object within the physics simulation that undergoes rigid body dynamics,
	that is, it is effected purely by the forces acting on the body and its mass. Also, it cannot
	deform, like cloth or clay, thus, rigid body.
*/

#ifndef _RIGIDBODY_H
#define _RIGIDBODY_H

namespace Katana
{

///
/// CollisionGeometry
/// A lightweight version of Geometry, it is used strictly for collision detection,
/// and doesn't contain other visible buffers like color, texture coordinates, etc.
///
/// CollisionGeometry stores vertices and a 3 floats and triangles and 3 indices.
/// Note the index buffer is a vector of int's instead of shorts, this for compatibility
/// purposes, most use ints to specific indices, instead of shorts (see Geometry).
///
/// Finally, because these are shared pointers to vectors, they can be shared with the
/// visible representation to save memory
///
struct CollisionGeometry
{
	shared_ptr< vector<float> > m_vertexBuffer;		/// Vertex Buffers
	shared_ptr< vector<float> > m_normalBuffer;		/// Normal Buffers
	shared_ptr< vector<int> >	m_indexBuffer;		/// Index Buffers
};

///
/// RigidBody
/// Wrapper for RigidBody. The functions implementations are deferred to the physics library.
///
class RigidBody
{
public:
	/// Constructor which takes whether this RigidBody is Fixed. By default, it is FALSE
	RigidBody( bool bFixed = false, bool bParticle = false );

	/// Sets whether this rigid body can interact with the simulation
	void setActive( bool active );

	/// Gets whether this rigid body can interact with the simulation
	bool getActive() const;

	/// Gets whether this rigid body is fixed
	bool isFixed() const;

	/// Gets whether this rigid body is a particle (is TRUE, then it CANNOT interact with other RigidBodies)
	bool isParticle() const;

	/// Sets the mass
	void setMass( float fMass );

	/// Gets the mass
	float getMass() const;

	/// Sets the position of the rigid body
	void setPosition( const Point3 & position );

	/// Sets the rotation of the rigid body
	void setRotation( const Quaternion & rotation );

	/// Gets the position of the rigid body
	Point3 getPosition() const;

	/// Gets the rotation of the rigid body
	Quaternion getRotation() const;

	/// Sets the inertia tensor
	void setInertiaTensor( const Matrix4 & tensor );

	/// Sets if the rigid body is affected by gravity
	void setEnableGravity( bool enable );

	/// Gets if the rigid body is affected by gravity
	bool getEnableGravity() const;

public:

	/// Sets a constant force on the rigid body
	void setForce( const Point3 & force );
	void setForce( const Point3 & force, const Point3 & position );

	/// Gets the constant force on the rigid body
	Point3 getForce() const;

	/// Sets a constant torque on the rigid body
	void setTorque( const Point3 & torque );

	/// Gets the constant torque on the rigid body
	Point3 getTorque() const;

	/// Apply an impulse on the rigid body
	void applyImpulse( const Point3 & impulse );
	void applyImpulse( const Point3 & impulse, const Point3 & position );

	/// Apply a twist on the rigid body
	void applyTwist( const Point3 & twist );

	/// Gets the velocity of the rigid body
	Point3 getVelocity() const;

	/// Gets the angular velocity of the rigid body
	Point3 getAngularVelocity() const;

public:

	/// Sets the internal RigidBody. WARNING: This should be be used by the PhysicsSystem
	void setInternals( void * pvInternals );

	/// Helper Template function which statically casts the internals to type T
	template <typename T> T getInternals() const;

public:

	/// Geometry used for the purposes of collision detection. Because the buffers can
	/// be shared, genererally this Geometry will not own the vertex, index and normal buffers,
	/// but shared them with the visible geometry. However, the client may want them to be
	/// different to have a lower detail geometry for collision detection.
	shared_ptr<CollisionGeometry> m_collisionGeometry;

private:

	/// Implementation specific version of the RigidBody (based on the physics library in use).
	void * m_pvInternals;

	/// Flags whether this RigidBody is fixed or movable. By default, this is FALSE.
	bool m_bIsFixed;

	/// Flags whether this RigidBody can interact with other RigidBodies (or has particle like-behavior). By default, this is FALSE.
	bool m_bIsParticle;
};

}; // Katana

#endif // _RIGIDBODY_H