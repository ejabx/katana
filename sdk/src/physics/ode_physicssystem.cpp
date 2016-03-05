/*
Katana Engine
Copyright © 2001-2004 Eric Bryant, Inc.

File:		ode_physicssystem.h
Author:		Eric Bryant

	Responsible for resolving rigid bodies within the physics sytem.
	This implementation used the ODE (Open Dynamics Engine), an open sourced
	physics rigid body dynamics engine. For more information, see http://q12.org/ode/.

	In addition, this library uses OPCODE to handle collisions between arbritrary triangle
	meshes. For more information, see http://www.codercorner.com/Opcode.htm.
*/

#include "katana_config.h"

#ifdef PHYSICS_USE_ODE
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include "physicssystem.h"
#include "rigidbody.h"
#include <ode/ode.h>

// --------------------------------------------------------------------

// Make sure client includes ODE libaries
#ifdef _DEBUG
	#pragma comment(lib, "ode_d.lib")
	#pragma comment(lib, "opcode_d.lib")
#else
	#pragma comment(lib, "ode.lib")
	#pragma comment(lib, "opcode.lib")
#endif

// --------------------------------------------------------------------
// Static Variables
// --------------------------------------------------------------------

static dWorldID			ODE_WORLD;
static dSpaceID			ODE_SPACE;
static dJointGroupID	ODE_CONTACT_GROUP;
static float			DEFAULT_TOTAL_MASS = 1;

// --------------------------------------------------------------------
// Static Functions
// --------------------------------------------------------------------

static void frictionModel( void * data, dGeomID o1, dGeomID o2 );
static void raycastModel( void * data, dGeomID o1, dGeomID o2 );

// --------------------------------------------------------------------
// PhysicsSystem
// --------------------------------------------------------------------

//
// initialize
//
bool PhysicsSystem::initialize()
{
	// Create our ODE World
	ODE_WORLD = dWorldCreate();

	// Create our default ODE Space. All Rigid Body objects are contained 
	// within this space, and it generally optimized for fast collision detection.
	// We can generate multiple spaces for physics LOD, but currently we only require one.
	ODE_SPACE = dHashSpaceCreate( 0 );

	// Create the default contact group. When using the default friction model, we need
	// this contact group to generate contacts for us.
	ODE_CONTACT_GROUP = dJointGroupCreate( 0 );

	// Setup Newtonian Gravity
	dWorldSetGravity( ODE_WORLD, 0, -10, 0 );

	return true;
}

//
// terminate
//
bool PhysicsSystem::terminate()
{
	// Destroy our contact group
	dJointGroupDestroy( ODE_CONTACT_GROUP );

	// Destroy our default ODE space
	dSpaceDestroy( ODE_SPACE );

	// Destroy the ODE World
	dWorldDestroy( ODE_WORLD );

	// Explicitly close all memory allocated by ODE
	dCloseODE();

	return true;
}

//
// integrate
//
void PhysicsSystem::integrate( float deltaTime )
{
	// Resolve all collisions within the ODE Space
	dSpaceCollide( ODE_SPACE, 0, &frictionModel );

	// Advance the simulation by the delta time 
	// (NOTE: Advance takes milliseconds, convert from seconds)
	if ( deltaTime ) dWorldStep( ODE_WORLD, deltaTime * 10 );	

	// Remove all contacts joints from the contact group
	dJointGroupEmpty( ODE_CONTACT_GROUP );
}

//
// addRigidBody
// Adds a rigid body to the simulation and returns a reference to it
//
shared_ptr<RigidBody> PhysicsSystem::addRigidBody( bool bFixed )
{
	// Create a new rigid body
	shared_ptr<RigidBody> rb( new RigidBody( bFixed ) );

	// If it is a movable rigid body....
	if ( !bFixed )
	{
		// Create a rigid body via the Physics System. This will tie it to
		// an internal RigidBody (via RigidBody::m_pvInternal).
		rb->setInternals( static_cast<void *>( dBodyCreate( ODE_WORLD ) ) );
	}

	return rb;
}

//
// removeRigidBody
// Removes a rigid body from the simulation
//
void PhysicsSystem::removeRigidBody( shared_ptr<RigidBody> & spRigidBody )
{
	// Only destroy unfixed Rigid Bodies because then have internal ODE RigidBodies
	if ( !spRigidBody->isFixed() )
	{
		// Grab the ODE RigidBody and destroy it
		dBodyID odeRB = spRigidBody->getInternals<dBodyID>();
		dBodyDestroy( odeRB );
	}

	// Reset the shared pointer
	spRigidBody.reset();
}

//
// castRay
// Casts a ray and invokes the callback when the ray intersects with a RigidBody
//
void PhysicsSystem::castRay( const Point3 & rayStart, const Point3 & rayEnd, RaycastCallback pfRayCB )
{
	// Create the ray
	dGeomID ray = dCreateRay( ODE_SPACE, ( rayEnd - rayStart ).getLength() );

	// Setup the ray origin and direction
	const Point3 vRayDirection = Point3( rayEnd - rayStart ).getNormalized();
	dGeomRaySet( ray, rayStart.x, rayStart.y, rayStart.z, vRayDirection.x, vRayDirection.y, vRayDirection.z );

	// Associate the callback function with this ray
	dGeomSetData( ray, pfRayCB );

	// Perform the raycast
	dSpaceCollide( ODE_SPACE, 0, &raycastModel );
}

//
// createGeometry*
// Creation functions for various geometries. It will associate the created geometry with the RigidBody
//
void PhysicsSystem::createBoxGeometry( shared_ptr<RigidBody> & spRigidBody, float fExtentX, float fExtentY, float fExtentZ )
{
	// Construct the box geometry and associate it with our ODE RigidBody
	dGeomID odeGeom = dCreateBox( ODE_SPACE, fExtentX, fExtentY, fExtentZ );

	// If this is fixed, store the ODE Geometry in the RB
	if ( spRigidBody->isFixed() ) 
		spRigidBody->setInternals( odeGeom );

	// Otherwise, associate the Geometry with the ODE RigidBody
	else
		dGeomSetBody( odeGeom, spRigidBody->getInternals<dBodyID>() );

	// Store the resultant Mass/Inertia in the Rigid Body
	if ( !spRigidBody->isFixed() ) 
	{
		// Set this box's size in the mass (for the inertia tensor)
		dMass odeMass;
		dMassSetZero( &odeMass );
		dMassSetBoxTotal( &odeMass, DEFAULT_TOTAL_MASS, fExtentX, fExtentY, fExtentZ );
		dBodySetMass( spRigidBody->getInternals<dBodyID>(), &odeMass );
	}
}

void PhysicsSystem::createSphereGeometry( shared_ptr<RigidBody> & spRigidBody, float fRadius )
{
	// Construct the sphere geometry and associate it with our ODE RigidBody
	dGeomID odeGeom = dCreateSphere( ODE_SPACE, fRadius );

	// If this is fixed, store the ODE Geometry in the RB
	if ( spRigidBody->isFixed() ) 
		spRigidBody->setInternals( odeGeom );

	// Otherwise, associate the Geometry with the ODE RigidBody
	else
		dGeomSetBody( odeGeom, spRigidBody->getInternals<dBodyID>() );

	// Store the resultant Mass/Inertia in the Rigid Body
	if ( !spRigidBody->isFixed() ) 
	{
		// Set this sphere's radius in the mass (for the inertia tensor) 
		dMass odeMass;
		dMassSetZero( &odeMass );
		dMassSetSphereTotal( &odeMass, DEFAULT_TOTAL_MASS, fRadius );
		dBodySetMass( spRigidBody->getInternals<dBodyID>(), &odeMass );
	}
}

void PhysicsSystem::createCylinderGeometry( shared_ptr<RigidBody> & spRigidBody, float fRadius, float fHeight )
{
	// Construct the cylinder geometry and associate it with our ODE RigidBody
	dGeomID odeGeom = dCreateCCylinder( ODE_SPACE, fRadius, fHeight );

	// If this is fixed, store the ODE Geometry in the RB
	if ( spRigidBody->isFixed() ) 
		spRigidBody->setInternals( odeGeom );

	// Otherwise, associate the Geometry with the ODE RigidBody
	else
		dGeomSetBody( odeGeom, spRigidBody->getInternals<dBodyID>() );

	// Store the resultant Mass/Inertia in the Rigid Body
	if ( !spRigidBody->isFixed() ) 
	{
		// Set this cylinders's height/radius in the mass (for the inertia tensor)
		dMass odeMass;
		dMassSetZero( &odeMass );
		dMassSetCylinderTotal( &odeMass, DEFAULT_TOTAL_MASS, 3, fRadius, fHeight );
		dBodySetMass( spRigidBody->getInternals<dBodyID>(), &odeMass );
	}
}

void PhysicsSystem::createPlaneGeometry( shared_ptr<RigidBody> & spRigidBody, const Point3 & normal, float fDistance )
{
	// Construct the plane geometry and associate it with our ODE RigidBody
	dGeomID odeGeom = dCreatePlane( ODE_SPACE, normal.x, normal.y, normal.z, fDistance );

	// If this is fixed, store the ODE Geometry in the RB
	if ( spRigidBody->isFixed() ) 
		spRigidBody->setInternals( odeGeom );

	// Otherwise, associate the Geometry with the ODE RigidBody
	else
		dGeomSetBody( odeGeom, spRigidBody->getInternals<dBodyID>() );
}

void PhysicsSystem::createMeshGeometry( shared_ptr<RigidBody> & spRigidBody, shared_ptr<Geometry> & spGeometry )
{
	// Create a new collision geometry for the rigid body. This is because while we can shared
	// the vertex information, the index information must be stored as ints, not as shorts. Also,
	// ODE needs the triangles indices defined in COUNTERCLOCKWISE order, while we use CLOCKWISE order
	// for DirectX/OpenGL
	spRigidBody->m_collisionGeometry.reset( new CollisionGeometry );

	// Share the vertex and normal information
	spRigidBody->m_collisionGeometry->m_vertexBuffer = spGeometry->m_vertexBuffer;
	spRigidBody->m_collisionGeometry->m_normalBuffer = spGeometry->m_normalBuffer;

	// Create a new index buffer
	spRigidBody->m_collisionGeometry->m_indexBuffer.reset( new vector< int >() );
	vector<int> & indexBuffer = *spRigidBody->m_collisionGeometry->m_indexBuffer;

	// Push the triangle indices in COUNTERCLOCKWISE order
	for( unsigned int index = 0; index < spGeometry->m_indexCount; index+=3 )
	{		
		indexBuffer.push_back( spGeometry->m_indexBuffer->at( index+2 ) );
		indexBuffer.push_back( spGeometry->m_indexBuffer->at( index+1 ) );
		indexBuffer.push_back( spGeometry->m_indexBuffer->at( index+0 ) );
	}

	// Construct the trimesh data and store the vertex and indices pointers within
	dTriMeshDataID triMeshData = dGeomTriMeshDataCreate();
	dGeomTriMeshDataBuildSingle( triMeshData, 
								  &spGeometry->m_vertexBuffer->at(0),	// Vertices
								  3 * sizeof( float ),					// Vertex Stride
								  spGeometry->m_vertexCount,			// Vertex Count
								  &indexBuffer[0],						// Triangle Indices
								  spGeometry->m_indexCount,				// Triangle Count
								  3 * sizeof( int )					// Triangle Stride
//								  &spGeometry->m_normalBuffer->at(0)	// Normals
	);

	// Construct the trimesh geometry and associate it with our ODE RigidBody
	dGeomID odeGeom = dCreateTriMesh( ODE_SPACE, triMeshData, 0, 0, 0 );

	// If this is fixed, store the ODE Geometry in the RB
	if ( spRigidBody->isFixed() ) 
		spRigidBody->setInternals( odeGeom );

	// Otherwise, associate the Geometry with the ODE RigidBody
	else
		dGeomSetBody( odeGeom, spRigidBody->getInternals<dBodyID>() );
}

// --------------------------------------------------------------------

//
// frictionModel
// Standard friction model
//
static void frictionModel( void *data, dGeomID o1, dGeomID o2 )
{
	// Maximum number of contact points per body
	const int MAX_CONTACTS_PER_BODY = 4;

	// exit without doing anything if the two bodies are connected by a joint
	dBodyID b1 = dGeomGetBody( o1 );
	dBodyID b2 = dGeomGetBody( o2 );
	if ( b1 && b2 && dAreConnectedExcluding( b1, b2, dJointTypeContact ) )
		return;

	dContact contact[ MAX_CONTACTS_PER_BODY ];   // up to MAX_CONTACTS_PER_BODY contacts per box-box
	for (int i = 0; i < MAX_CONTACTS_PER_BODY; i++ )
	{
		contact[i].surface.mode = dContactBounce | dContactSoftCFM;
		contact[i].surface.mu = dInfinity;
		contact[i].surface.mu2 = 0;
		contact[i].surface.bounce = 0.1f;
		contact[i].surface.bounce_vel = 0.1f;
		contact[i].surface.soft_cfm = 0.01f;
	}
	if ( int contacts = dCollide( o1, o2, MAX_CONTACTS_PER_BODY, &contact[0].geom, sizeof( dContact ) ) )
	{
		dMatrix3 RI;
		dRSetIdentity( RI );
		const dReal ss[3] = { 0.02f, 0.02f, 0.02f };
		for ( int i = 0; i < contacts; i++ )
		{
			dJointID c = dJointCreateContact( ODE_WORLD, ODE_CONTACT_GROUP, contact + i );
			dJointAttach( c, b1, b2 );
		}
	}
}

//
// raycastModel
// Only used for raycast collisions
//
static void raycastModel( void * data, dGeomID o1, dGeomID o2 )
{
	// Did the ray collide with anything?
	if ( dGeomGetClass( o1 ) == dRayClass || dGeomGetClass( o2 ) == dRayClass )
	{
		int contacts;
		dContactGeom contact[1];

		// Determine the single contact point
		if ( dGeomGetClass( o1 ) == dRayClass )
			contacts = dCollide( o2, o1, 1, &contact[0], sizeof( dContactGeom ) );
		else
			contacts = dCollide( o1, o2, 1, &contact[0], sizeof( dContactGeom ) );

		// Did we have any contacts?
		if ( contacts )
		{
			// Select the first contact point
			const Point3 position = Point3( contact[0].pos[0], contact[0].pos[1], contact[0].pos[2] );
			const Point3 normal = Point3( contact[0].normal[0], contact[0].normal[1], contact[0].normal[2] );

			// Retrieve the callback function from the ray
			RaycastCallback pfRayCB;
			shared_ptr<RigidBody> otherRB;
			if ( dGeomGetClass( o1 ) == dRayClass ) pfRayCB = static_cast<RaycastCallback>( dGeomGetData( o1 ) );
			if ( dGeomGetClass( o2 ) == dRayClass ) pfRayCB = static_cast<RaycastCallback>( dGeomGetData( o2 ) );

			// Invoke the callback function
			// TODO: Retrieve the original Collidable object
			pfRayCB( position, normal, shared_ptr<Collidable>() );
		}
	}

	// Destroy the ray
	if ( dGeomGetClass( o1 ) == dRayClass ) dGeomDestroy( o1 );
	if ( dGeomGetClass( o2 ) == dRayClass ) dGeomDestroy( o2 );
}

#endif // PHYSICS_USE_ODE