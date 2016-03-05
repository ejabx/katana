/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		streamable.h
	Author:		Eric Bryant

	Interface for classes which can interact with the Physics System
*/

#include "katana_config.h"

#ifdef PHYSICS_USE_ODE
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include "collidable.h"
#include "physicssystem.h"
#include "rigidbody.h"

// --------------------------------------------------------------------
// External References
// --------------------------------------------------------------------

extern shared_ptr<PhysicsSystem>	katana_physics;

// --------------------------------------------------------------------
// Collidable
// --------------------------------------------------------------------

//
// RTTI declaration
//
KIMPLEMENT_RTTI( Collidable, Streamable );

//
// Destructor
//
Collidable::~Collidable()
{
	// Remove my rigid body from the simulation
	if ( m_spRigidBody ) katana_physics->removeRigidBody( m_spRigidBody );
}

//
// setCollideShape
// Generates a Physics representation of the Geometry
//
void Collidable::setCollideShape( shared_ptr<Geometry> spGeometry, CollideShape shapeType )
{
	// Only continue if we have a valid rigid body OR we're generating a triangle mesh
	if ( !m_spRigidBody ) return;

	// Determine how to calculate the shape from the Geometry
	switch( shapeType )
	{
		case COLLIDE_PLANE:
		{
			// TODO: Allow planes not at the origin
			katana_physics->createPlaneGeometry( m_spRigidBody, Point3( 0, 1, 0 ), 0 );

			break;
		}
		case COLLIDE_BOX:
		{
			// Determine the dimensions of the bounding box
			Point3 vMin, vMax, vExtents;
			Geometry::createBox( spGeometry, vMin, vMax );
			vExtents = vMax - vMin;

			// Create a box geometry for collision detection
			katana_physics->createBoxGeometry( m_spRigidBody, vExtents.x, vExtents.y, vExtents.z );

			break;
		}
		case COLLIDE_SPHERE:
		{
			// Determine the dimensions of the bounding sphere
			Point3 vCenter; float fRadius;
			Geometry::createSphere( spGeometry, vCenter, fRadius );

			// Create a sphere geometry for collision detection
			katana_physics->createSphereGeometry( m_spRigidBody, fRadius );

			break;
		}
		case COLLIDE_CYLINDER:
		{
			// Determine the dimensions of the bounding cylinder
			Point3 vCenter, vDirection; float fHeight, fRadius;
			Geometry::createCylinder( spGeometry, vCenter, vDirection, fHeight, fRadius );

			// Create a cylinder geometry for collision detection
			katana_physics->createCylinderGeometry( m_spRigidBody, fRadius, fHeight );

			break;
		}
		case COLLIDE_MESH:
		{
			// Create a mesh geometry for collision detection
			katana_physics->createMeshGeometry( m_spRigidBody, spGeometry );

			break;
		}
	}
}

//
// createRigidBody
// Creates an internal instance of the RigidBody, which defines the Physics representation
// of this object (mass, inertia, phyics geometry, etc.)
//
void Collidable::createRigidBody( bool bFixed )
{
	m_spRigidBody = katana_physics->addRigidBody( bFixed );
}

//
// getRigidBody
// Retrieves the internal RigidBody
//
shared_ptr<RigidBody> Collidable::getRigidBody()
{
	return m_spRigidBody;
}

//
// setRigidBody
// Replaces the internal instance of the RigidBody
//
void Collidable::setRigidBody( shared_ptr<RigidBody> spRigidBody )
{
	m_spRigidBody = spRigidBody;
}

#endif // PHYSICS_USE_ODE