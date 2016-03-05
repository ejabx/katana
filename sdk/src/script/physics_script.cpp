/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		physics_script.cpp
	Author:		Eric Bryant

	Registers all classes in physics library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>
#include <luabind/discard_result_policy.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "scriptengine.h"
#include "physics/physicssystem.h"
#include "physics/collidable.h"
#include "physics/rigidbody.h"

// --------------------------------------------------------------------
// External References
// --------------------------------------------------------------------

extern shared_ptr<PhysicsSystem>	katana_physics;

// --------------------------------------------------------------------
// Extern Function
// --------------------------------------------------------------------

bool Base_Register( lua_State * env );

// --------------------------------------------------------------------
// Local Functions
// --------------------------------------------------------------------

shared_ptr<PhysicsSystem>	getPhysics()	{ return katana_physics; }

// --------------------------------------------------------------------
// Registration
// --------------------------------------------------------------------

//
// PhysicsSystem Registration
//
bool PhysicsSystem::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
		[
			class_< PhysicsSystem, shared_ptr<PhysicsSystem> >( "PhysicsSystem" )
			,
			def( "getPhysics", getPhysics )
		];

	return true;
}

//
// Collidable Registration
//
bool Collidable::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	Base_Register( env );
	module( env )
		[
			class_< RigidBody, shared_ptr<RigidBody> >( "RigidBody" )
				.def( "setActive",			&RigidBody::setActive )
				.def( "getActive",			&RigidBody::getActive )
				.def( "isFixed",			&RigidBody::isFixed )
				.def( "setMass",			&RigidBody::setMass )
				.def( "getMass",			&RigidBody::getMass )
				.def( "setPosition",		&RigidBody::setPosition )
				.def( "setRotation", 		&RigidBody::setRotation )
				.def( "getPosition", 		&RigidBody::getPosition )
				.def( "getRotation", 		&RigidBody::getRotation )
				.def( "setInertiaTensor",	&RigidBody::setInertiaTensor )
				.def( "setEnableGravity", 	&RigidBody::setEnableGravity )
				.def( "getEnableGravity",	&RigidBody::getEnableGravity )
				.def( "setForce", 			(void(RigidBody::*)(const Point3 &))&RigidBody::setForce )
				.def( "setForce", 			(void(RigidBody::*)(const Point3 &, const Point3 &))&RigidBody::setForce )
				.def( "getForce", 			&RigidBody::getForce )
				.def( "setTorque", 			&RigidBody::setTorque )
				.def( "getTorque", 			&RigidBody::getTorque )
				.def( "applyImpulse", 		(void(RigidBody::*)(const Point3 &))&RigidBody::applyImpulse )
				.def( "applyImpulse", 		(void(RigidBody::*)(const Point3 &, const Point3 &))&RigidBody::applyImpulse )
				.def( "applyTwist",			&RigidBody::applyTwist )
				.def( "getVelocity",		&RigidBody::getVelocity )
				.def( "getAngularVelocity", &RigidBody::getAngularVelocity )
			,
			class_< Collidable, Streamable, shared_ptr<Collidable> >( "Collidable" )
				.def( "setCollideShape",	&setCollideShape )
				.def( "createRigidBody",	&createRigidBody )
				.def( "getRigidBody",		&getRigidBody )
				.enum_( "CollideShape" )
					[
						value( "BOX",		COLLIDE_BOX ),
						value( "SPHERE",	COLLIDE_SPHERE ),
						value( "CYLINDER",	COLLIDE_CYLINDER ),
						value( "MESH",		COLLIDE_MESH ),
						value( "PLANE",		COLLIDE_PLANE )
					]
		];

	return true;
}
