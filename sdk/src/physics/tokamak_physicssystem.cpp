/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		tokamak_physicssystem.h
	Author:		Eric Bryant

	Responsible for resolving rigid bodies within the physics sytem.
	This implementation used the Tokamak physics engine, version 1.0.10.
*/

#include "katana_config.h"

#ifdef PHYSICS_USE_TOKAMAK
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "physicssystem.h"
#include "rigidbody.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include <tokamak.h>

// --------------------------------------------------------------------

// Make sure client includes Tokamak libaries
#ifdef _DEBUG
	#pragma comment(lib, "tokamak_d.lib")
#else
	#pragma comment(lib, "tokamak.lib")
#endif

// --------------------------------------------------------------------
// Static Variables
// --------------------------------------------------------------------

static shared_ptr<neSimulator>	TOKAMAK_SIMULATION;

// --------------------------------------------------------------------
// PhysicsSystem
// --------------------------------------------------------------------

//
// initialize
//
bool PhysicsSystem::initialize()
{
	// Setup the simulation parameters
	neSimulatorSizeInfo sizeInfo;

	// Default Gravity
	neV3 gravity; gravity.Set(0.0f, -10.0f, 0.0f);

	// Create the simulation
	TOKAMAK_SIMULATION.reset( neSimulator::CreateSimulator( sizeInfo, NULL, &gravity ) );

	return true;
}

//
// terminate
//
bool PhysicsSystem::terminate()
{
	// Destroy the simulation
	neSimulator::DestroySimulator( TOKAMAK_SIMULATION.get() );

	return true;
}

//
// integrate
//
void PhysicsSystem::integrate( float deltaTime )
{
	// Advance the simulation by the delta time 
	// (NOTE: Advance takes milliseconds, convert from seconds)
	TOKAMAK_SIMULATION->Advance( deltaTime * 10 );
}

//
// addRigidBody
// Adds a rigid body to the simulation and returns a reference to it
//
shared_ptr<RigidBody> PhysicsSystem::addRigidBody( bool bFixed )
{
	// Create a new rigid body
	shared_ptr<RigidBody> rb( new RigidBody( bFixed ) );

	// If fixed, create a neAnimatedBody and store it within the RigidBody
	if ( bFixed )
		rb->setInternals( static_cast<void *>( TOKAMAK_SIMULATION->CreateAnimatedBody() ) );

	// Otherwise, create a neRigidBody and store it within the RigidBody
	else
		rb->setInternals( static_cast<void *>( TOKAMAK_SIMULATION->CreateRigidBody() ) );

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
		neRigidBody * tokamakRB = spRigidBody->getInternals<neRigidBody *>();
		TOKAMAK_SIMULATION->FreeRigidBody( tokamakRB );
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
	// TODO
}

//
// createGeometry*
// Creation functions for various geometries. It will associate the created geometry with the RigidBody
//
void PhysicsSystem::createBoxGeometry( shared_ptr<RigidBody> & spRigidBody, float fExtentX, float fExtentY, float fExtentZ )
{
	neRigidBody * pRB = spRigidBody->getInternals<neRigidBody *>();
	neAnimatedBody * pAB = spRigidBody->getInternals<neAnimatedBody *>();

	// Adds geometry to the animated/rigid body and reference it
	neGeometry * pGeometry = 0;
	if ( !spRigidBody->isFixed() ) pGeometry = pRB->AddGeometry();
	if (  spRigidBody->isFixed() ) pGeometry = pAB->AddGeometry();

	// Set this box's size in the geometry
	pGeometry->SetBoxSize( fExtentX, fExtentY, fExtentZ );

	// Update the Rigid Body or Animated Body's bounding info (required after we set the Geometry size)
	if ( !spRigidBody->isFixed() ) pRB->UpdateBoundingInfo();
	if (  spRigidBody->isFixed() ) pAB->UpdateBoundingInfo();

	// Setup the inertia tensor
	if ( !spRigidBody->isFixed() )
		pRB->SetInertiaTensor( neBoxInertiaTensor( fExtentX, fExtentY, fExtentZ, spRigidBody->getMass() ) );
}

void PhysicsSystem::createSphereGeometry( shared_ptr<RigidBody> & spRigidBody, float fRadius )
{
	neRigidBody * pRB = spRigidBody->getInternals<neRigidBody *>();
	neAnimatedBody * pAB = spRigidBody->getInternals<neAnimatedBody *>();

	// Adds geometry to the animated/rigid body and reference it
	neGeometry * pGeometry = 0;
	if ( !spRigidBody->isFixed() ) pGeometry = pRB->AddGeometry();
	if (  spRigidBody->isFixed() ) pGeometry = pAB->AddGeometry();

	// Set this sphere's radius in the geometry 
	pGeometry->SetSphereDiameter( fRadius * 2 );

	// Update the Rigid Body or Animated Body's bounding info (required after we set the Geometry size)
	if ( !spRigidBody->isFixed() ) pRB->UpdateBoundingInfo();
	if (  spRigidBody->isFixed() ) pAB->UpdateBoundingInfo();

	// Setup the inertia tensor
	if ( !spRigidBody->isFixed() )
		pRB->SetInertiaTensor( neSphereInertiaTensor( fRadius, spRigidBody->getMass() ) );
}

void PhysicsSystem::createCylinderGeometry( shared_ptr<RigidBody> & spRigidBody, float fRadius, float fHeight )
{
	neRigidBody * pRB = spRigidBody->getInternals<neRigidBody *>();
	neAnimatedBody * pAB = spRigidBody->getInternals<neAnimatedBody *>();

	// Adds geometry to the animated/rigid body and reference it
	neGeometry * pGeometry = 0;
	if ( !spRigidBody->isFixed() ) pGeometry = pRB->AddGeometry();
	if (  spRigidBody->isFixed() ) pGeometry = pAB->AddGeometry();

	// Set this cylinders's height/radius in the geometry
	pGeometry->SetCylinder( fRadius, fHeight - fRadius );

	// Update the Rigid Body or Animated Body's bounding info (required after we set the Geometry size)
	if ( !spRigidBody->isFixed() ) pRB->UpdateBoundingInfo();
	if (  spRigidBody->isFixed() ) pAB->UpdateBoundingInfo();

	// TODO: Setup the transform of the geometry. This is needed because the orientation of the cylinder
	// is specified by vDirection.

	// Setup the inertia tensor
	if ( !spRigidBody->isFixed() )
		pRB->SetInertiaTensor( neCylinderInertiaTensor( fRadius, fHeight - fRadius, spRigidBody->getMass() ) );
}

void PhysicsSystem::createPlaneGeometry( shared_ptr<RigidBody> & spRigidBody, const Point3 & normal, float fDistance )
{
	// TODO
}

void PhysicsSystem::createMeshGeometry( shared_ptr<RigidBody> & spRigidBody, shared_ptr<Geometry> & spGeometry )
{
	// Generate a Triangle Mesh from the Geometry
	neTriangleMesh * pTriMesh = new neTriangleMesh;

	// Reinterpret out vertex buffer point (which are float) to Point3s
	const Point3 * pPoints = reinterpret_cast<const Point3 *>( &spGeometry->m_vertexBuffer->front() );

	// Create Tokamak's vertices and copy them from the geometry
	neV3 * neVertices = new neV3[ spGeometry->m_vertexCount ];

	for( unsigned int uiVertex = 0; uiVertex < spGeometry->m_vertexCount; uiVertex++ )
		neVertices[ uiVertex ].Set( pPoints[ uiVertex ].x, pPoints[ uiVertex ].y, pPoints[ uiVertex ].z );

	// Create Tokamak's triangles and populate them with the geometry's index information
	neTriangle * neTriangles = new neTriangle[ spGeometry->m_primitiveCount ];

	for( unsigned int uiTriangle = 0; uiTriangle < spGeometry->m_primitiveCount; uiTriangle++ )
	{
		neTriangles[uiTriangle].indices[0] = spGeometry->m_indexBuffer->at( uiTriangle * 3 + 0 );
		neTriangles[uiTriangle].indices[1] = spGeometry->m_indexBuffer->at( uiTriangle * 3 + 1 );
		neTriangles[uiTriangle].indices[2] = spGeometry->m_indexBuffer->at( uiTriangle * 3 + 2 );

		neTriangles[uiTriangle].materialID = 0;
		neTriangles[uiTriangle].flag = neTriangle::NE_TRI_TRIANGLE;
	}

	// Setup the Triangle Mesh vertices and triangles
	pTriMesh->vertexCount = spGeometry->m_vertexCount;
	pTriMesh->triangleCount = spGeometry->m_primitiveCount;
	pTriMesh->vertices = neVertices;
	pTriMesh->triangles = neTriangles;

	TOKAMAK_SIMULATION->SetTerrainMesh( pTriMesh );
}

#endif // PHYSICS_USE_TOKAMAK