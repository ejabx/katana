/*
	Katana Engine
	Copyright © 2004 Eric Bryant, Inc.

	File:		rigidbody.cpp
	Author:		Eric Bryant

	A Rigid Body an object within the physics simulation that undergoes rigid body dynamics,
	that is, it is effected purely by the forces acting on the body and its mass. Also, it cannot
	deform, like cloth or clay, thus, rigid body.
*/

#include "katana_config.h"

#ifdef PHYSICS_USE_ODE
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "RigidBody.h"
#include <ode/ode.h>

//
// Constructor
//
RigidBody::RigidBody( bool bFixed, bool bParticle )
	: m_pvInternals( NULL )
	, m_bIsFixed( bFixed )
	, m_bIsParticle( bParticle )
{
}

//
// setActive
// Sets whether this rigid body can interact with the simulation
//
void RigidBody::setActive( bool active )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Enable or disable it
	if ( active )
		dBodyEnable( odeRB );
	else
		dBodyDisable( odeRB );
}

//
// getActive
// Gets whether this rigid body can interact with the simulation
//
bool RigidBody::getActive() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return false;

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Return whether it's enabled
	return dBodyIsEnabled( odeRB ) ? true : false;
}

// 
// isFixed
// Gets whether this rigid body is fixed
//
inline bool RigidBody::isFixed() const
{
	return m_bIsFixed;
}

//
// isParticle
//
bool RigidBody::isParticle() const
{
	return m_bIsParticle;
}

//
// setMass
// Sets the mass
//
void RigidBody::setMass( float fMass )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Grab the mass and adjust the total mass of the RigidBody
	dMass odeMass;
	dBodyGetMass( odeRB, &odeMass );
	dMassAdjust( &odeMass, fMass );
}

//
// getMass
// Gets the mass
//
float RigidBody::getMass() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return 0.f;

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Grab the mass and return the density
	dMass odeMass;
	dBodyGetMass( odeRB, &odeMass );
	return odeMass.mass;
}

//
// setPosition
// Sets the position of the rigid body
//
void RigidBody::setPosition( const Point3 & position )
{
	// Set the Geometry's position if this a fixed rigid body
	if ( isFixed() )
	{
		// Grab the ODE Geometry
		dGeomID odeGeom = getInternals<dGeomID>();

		// Set the Position
		dGeomSetPosition( odeGeom, position.x, position.y, position.z );
	}
	else
	{
		// Grab the ODE RigidBody
		dBodyID odeRB = getInternals<dBodyID>();

		// Set the Position
		dBodySetPosition( odeRB, position.x, position.y, position.z );
	}
}

//
// setRotation
// Sets the rotation of the rigid body
//
void RigidBody::setRotation( const Quaternion & rotation )
{
	// Set the Geometry's rotation if this a fixed rigid body
	if ( isFixed() )
	{
		// Grab the ODE Geometry
		dGeomID odeGeom = getInternals<dGeomID>();

		// Set the Rotation
		dQuaternion odeRot;
		odeRot[0] = rotation.w; odeRot[1] = rotation.x; odeRot[2] = rotation.y; odeRot[3] = rotation.z;
		dGeomSetQuaternion( odeGeom, odeRot );
	}
	else
	{
		// Grab the ODE RigidBody
		dBodyID odeRB = getInternals<dBodyID>();

		// Set the Rotation
		dQuaternion odeRot;
		odeRot[0] = rotation.w; odeRot[1] = rotation.x; odeRot[2] = rotation.y; odeRot[3] = rotation.z;
		dBodySetQuaternion( odeRB, odeRot );
	}
}

//
// getPosition
// Gets the position of the rigid body
//
Point3 RigidBody::getPosition() const
{
	// Get the position from the Geometry if this is a fixed rigid body
	if ( isFixed() )
	{
		// Grab the ODE Geometry
		dGeomID odeGeom = getInternals<dGeomID>();

		// Return the position
		const dReal * pos = dGeomGetPosition( odeGeom );
		return Point3( pos[0], pos[1], pos[2] );
	}
	else
	{
		// Grab the ODE RigidBody
		dBodyID odeRB = getInternals<dBodyID>();

		// Return the position
		const dReal * pos = dBodyGetPosition( odeRB );
		return Point3( pos[0], pos[1], pos[2] );
	}
}

//
// getRotation
// Gets the rotation of the rigid body
//
Quaternion RigidBody::getRotation() const
{
	// Get the rotation from the Geometry if this a fixed rigid body
	if ( isFixed() )
	{
		// Grab the ODE Geometry
		dGeomID odeGeom = getInternals<dGeomID>();

		// Return the position
		dReal rot[4];
		dGeomGetQuaternion( odeGeom, rot );
		return Quaternion( rot[1], rot[2], rot[3], rot[0] );
	}
	else
	{
		// Grab the ODE RigidBody
		dBodyID odeRB = getInternals<dBodyID>();

		// Return the position
		const dReal * rot = dBodyGetQuaternion( odeRB );
		return Quaternion( rot[1], rot[2], rot[3], rot[0] );
	}
}

//
// setInertiaTensor
// Sets the inertia tensor
//
void RigidBody::setInertiaTensor( const Matrix4 & tensor )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Grab the mass and adjust the total mass of the RigidBody
	dMass odeMass;
	dBodyGetMass( odeRB, &odeMass );

	// Set the inertial tensor
	dMassSetParameters( &odeMass, odeMass.mass, 
						odeMass.c[0],	odeMass.c[1],	odeMass.c[2],
						tensor.m[1][1], tensor.m[2][2], tensor.m[3][3],
						tensor.m[1][2], tensor.m[1][2], tensor.m[2][3]
	);
}

//
// setForce
// Sets a constant force on the rigid body
//
void RigidBody::setForce( const Point3 & force )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Set the Force
	dBodyAddForce( odeRB, force.x, force.y, force.z );
}

void RigidBody::setForce( const Point3 & force, const Point3 & position )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Set the Force
	dBodyAddForceAtPos( odeRB, force.x, force.y, force.z, position.x, position.y, position.z );
}

//
// setEnableGravity
// Sets if the rigid body is affected by gravity
// 
void RigidBody::setEnableGravity( bool enable )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the ODE RigidBody and set the gravity mode (non-zero is enabled)
	dBodyID odeRB = getInternals<dBodyID>();
	dBodySetGravityMode( odeRB, enable );
}

//
// getEnableGravity
// Gets if the rigid body is affected by gravity
//
bool RigidBody::getEnableGravity() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return false;

	// Grab the ODE RigidBody and set the gravity mode (non-zero is enabled)
	dBodyID odeRB = getInternals<dBodyID>();

	// Return the gravity mode
	return dBodyGetGravityMode( odeRB ) ? true : false;
}

//
// getForce
// Gets the constant force on the rigid body
//
Point3 RigidBody::getForce() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return Point3();

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Return the force
	const dReal * force = dBodyGetForce( odeRB );
	return Point3( force[0], force[1], force[2] );
}

//
// setTorque
// Sets a constant torque on the rigid body
//
void RigidBody::setTorque( const Point3 & torque )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Set the Torque
	dBodySetTorque( odeRB, torque.x, torque.y, torque.z );
}

//
// getTorque
// Gets the constant torque on the rigid body
//
Point3 RigidBody::getTorque() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return Point3();

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Return the torque
	const dReal * torque = dBodyGetTorque( odeRB );
	return Point3( torque[0], torque[1], torque[2] );
}


//
// applyImpulse
// Apply an impulse on the rigid body
//
void RigidBody::applyImpulse( const Point3 & impulse )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// TODO
}

void RigidBody::applyImpulse( const Point3 & impulse, const Point3 & position )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// TODO
}

//
// applyTwist
// Apply a twist on the rigid body
//
void RigidBody::applyTwist( const Point3 & twist )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// TODO
}

//
// getVelocity
// Gets the velocity of the rigid body
//
Point3 RigidBody::getVelocity() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return Point3();

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Return the linear velocity
	const dReal * linear_velocity = dBodyGetLinearVel( odeRB );
	return Point3( linear_velocity[0], linear_velocity[1], linear_velocity[2] );
}

//
// getAngularVelocity
// Gets the angular velocity of the rigid body
//
Point3 RigidBody::getAngularVelocity() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return Point3();

	// Grab the ODE RigidBody
	dBodyID odeRB = getInternals<dBodyID>();

	// Return the angular velocity
	const dReal * angular_velocity = dBodyGetAngularVel( odeRB );
	return Point3( angular_velocity [0], angular_velocity [1], angular_velocity [2] );
}

//
// setInternals
// Sets the internal RigidBody. WARNING: This should be be used by the PhysicsSystem
//
void RigidBody::setInternals( void * pvInternals )
{
	m_pvInternals = pvInternals;
}

//
// getInternals
// Helper Template function which statically casts the internals to type T
//
template <typename T> 
inline T RigidBody::getInternals() const
{
	return static_cast<T>( m_pvInternals );
}

#endif // PHYSICS_USE_ODE