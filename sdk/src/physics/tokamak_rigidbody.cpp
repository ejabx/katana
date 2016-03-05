/*
	Katana Engine
	Copyright © 2004 Eric Bryant, Inc.

	File:		tokamak_rigidbody.cpp
	Author:		Eric Bryant

	A Rigid Body an object within the physics simulation that undergoes rigid body dynamics,
	that is, it is effected purely by the forces acting on the body and its mass. Also, it cannot
	deform, like cloth or clay, thus, rigid body.
*/

#include "katana_config.h"

#ifdef PHYSICS_USE_TOKAMAK
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "RigidBody.h"
#include <tokamak.h>

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
	if ( isFixed() )
	{
		// Grab the Tokamak AnimatedBody
		neAnimatedBody * pTokamakAB = getInternals<neAnimatedBody *>();
		pTokamakAB->Active( active, pTokamakAB );
	}
	else
	{
		// Grab the Tokamak RigidBody
		neRigidBody * pTokamakRB = getInternals<neRigidBody *>();
		pTokamakRB->Active( active, pTokamakRB );
	}
}

//
// getActive
// Gets whether this rigid body can interact with the simulation
//
bool RigidBody::getActive() const
{
	if ( isFixed() )
	{
		// Grab the Tokamak AnimatedBody
		neAnimatedBody * pTokamakAB = getInternals<neAnimatedBody *>();
		return pTokamakAB->Active() ? true : false;
		pTokamakAB->Active() ? true : false;
	}
	else
	{
		// Grab the Tokamak RigidBody
		neRigidBody * pTokamakRB = getInternals<neRigidBody *>();
		return pTokamakRB->Active() ? true : false;
		pTokamakRB->Active() ? true : false;
	}
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

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();
	pTokamakRB->SetMass( fMass );
}

//
// getMass
// Gets the mass
//
float RigidBody::getMass() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return 0.f;

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();
	return pTokamakRB->GetMass();
}

//
// setPosition
// Sets the position of the rigid body
//
void RigidBody::setPosition( const Point3 & position )
{
	if ( isFixed() )
	{
		// Grab the Tokamak AnimatedBody
		neAnimatedBody * pTokamakAB = getInternals<neAnimatedBody *>();
		neV3 pos; pos.v[3] = 1; // W component
		pos.Set( position.x, position.y, position.z );
		pTokamakAB->SetPos( pos );
	}
	else
	{
		// Grab the Tokamak RigidBody
		neRigidBody * pTokamakRB = getInternals<neRigidBody *>();
		neV3 pos; pos.v[3] = 1; // W component
		pos.Set( position.x, position.y, position.z );
		pTokamakRB->SetPos( pos );
	}
}

//
// setRotation
// Sets the rotation of the rigid body
//
void RigidBody::setRotation( const Quaternion & rotation )
{
	if ( isFixed() )
	{
		// Grab the Tokamak AnimatedBody
		neAnimatedBody * pTokamakAB = getInternals<neAnimatedBody *>();
		neQ rot;
		rot.Set( rotation.x, rotation.y, rotation.z, rotation.w );
		pTokamakAB->SetRotation( rot );
	}
	else
	{
		// Grab the Tokamak RigidBody
		neRigidBody * pTokamakRB = getInternals<neRigidBody *>();
		neQ rot;
		rot.Set( rotation.x, rotation.y, rotation.z, rotation.w );
		pTokamakRB->SetRotation( rot );
	}
}

//
// getPosition
// Gets the position of the rigid body
//
Point3 RigidBody::getPosition() const
{
	if ( isFixed() )
	{
		// Grab the Tokamak AnimatedBody
		neAnimatedBody * pTokamakAB = getInternals<neAnimatedBody *>();
		neV3 pos = pTokamakAB->GetPos();
		return Point3( pos.v[0], pos.v[1], pos.v[2] );
	}
	else
	{
		// Grab the Tokamak RigidBody
		neRigidBody * pTokamakRB = getInternals<neRigidBody *>();
		neV3 pos = pTokamakRB->GetPos();
		return Point3( pos.v[0], pos.v[1], pos.v[2] );
	}
}

//
// getRotation
// Gets the rotation of the rigid body
//
Quaternion RigidBody::getRotation() const
{
	if ( isFixed() )
	{
		// Grab the Tokamak AnimatedBody
		neAnimatedBody * pTokamakAB = getInternals<neAnimatedBody *>();
		neQ rot = pTokamakAB->GetRotationQ();
		return Quaternion( rot.X, rot.Y, rot.Z, rot.W );
	}
	else
	{
		// Grab the Tokamak RigidBody
		neRigidBody * pTokamakRB = getInternals<neRigidBody *>();
		neQ rot = pTokamakRB->GetRotationQ();
		return Quaternion( rot.X, rot.Y, rot.Z, rot.W );
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

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neM3 neTensor;
	neTensor.M[1].Set( tensor.right[0], tensor.right[1], tensor.right[2] );
	neTensor.M[2].Set( tensor.up[0],	tensor.up[1],	 tensor.up[2] );
	neTensor.M[3].Set( tensor.at[0],	tensor.at[1],	 tensor.at[2] );

	pTokamakRB->SetInertiaTensor( neTensor );
}

//
// setForce
// Sets a constant force on the rigid body
//
void RigidBody::setForce( const Point3 & force )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neV3 newton;
	newton.Set( force.x, force.y, force.z );
	pTokamakRB->SetForce( newton );
}

void RigidBody::setForce( const Point3 & force, const Point3 & position )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neV3 newton, pos;
	newton.Set( force.x, force.y, force.z );
	pos.Set( position.x, position.y, position.z );
	pTokamakRB->SetForce( newton, pos );
}

//
// setEnableGravity
// Sets if the rigid body is affected by gravity
// 
void RigidBody::setEnableGravity( bool enable )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();
	pTokamakRB->GravityEnable( enable );
}

//
// getEnableGravity
// Gets if the rigid body is affected by gravity
//
bool RigidBody::getEnableGravity() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return false;

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();
	return pTokamakRB->GravityEnable() ? true : false;
}

//
// getForce
// Gets the constant force on the rigid body
//
Point3 RigidBody::getForce() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return Point3();

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neV3 newton = pTokamakRB->GetForce();
	return Point3( newton.v[0], newton.v[1], newton.v[2] );
}

//
// setTorque
// Sets a constant torque on the rigid body
//
void RigidBody::setTorque( const Point3 & torque )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neV3 neTorque;
	neTorque.Set( torque.x, torque.y, torque.z );
	pTokamakRB->SetTorque( neTorque );
}

//
// getTorque
// Gets the constant torque on the rigid body
//
Point3 RigidBody::getTorque() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return Point3();

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neV3 torque = pTokamakRB->GetTorque();
	return Point3( torque.v[0], torque.v[1], torque.v[2] );
}


//
// applyImpulse
// Apply an impulse on the rigid body
//
void RigidBody::applyImpulse( const Point3 & impulse )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neV3 neImpulse;
	neImpulse.Set( impulse.x, impulse.y, impulse.z );
	pTokamakRB->ApplyImpulse( neImpulse );
}

void RigidBody::applyImpulse( const Point3 & impulse, const Point3 & position )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neV3 neImpulse, pos;
	neImpulse.Set( impulse.x, impulse.y, impulse.z );
	pos.Set( position.x, position.y, position.z );
	pTokamakRB->ApplyImpulse( neImpulse, pos );
}

//
// applyTwist
// Apply a twist on the rigid body
//
void RigidBody::applyTwist( const Point3 & twist )
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return;

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neV3 neTwist;
	neTwist.Set( twist.x, twist.y, twist.z );
	pTokamakRB->ApplyTwist( neTwist );
}

//
// getVelocity
// Gets the velocity of the rigid body
//
Point3 RigidBody::getVelocity() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return Point3();

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neV3 velocity = pTokamakRB->GetVelocity();
	return Point3( velocity.v[0], velocity.v[1], velocity.v[2] );
}

//
// getAngularVelocity
// Gets the angular velocity of the rigid body
//
Point3 RigidBody::getAngularVelocity() const
{
	// Ignore if this is a Fixed Rigid Body
	if ( isFixed() ) return Point3();

	// Grab the Tokamak RigidBody
	neRigidBody * pTokamakRB = getInternals<neRigidBody *>();

	neV3 angular_velocity = pTokamakRB->GetAngularVelocity();
	return Point3( angular_velocity.v[0], angular_velocity.v[1], angular_velocity.v[2] );
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

#endif // PHYSICS_USE_TOKAMAK