/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		vismesh.cpp
	Author:		Eric Bryant

	A Visible Mesh, which contains geometry data (vertices, textures, indices, etc.)
	This class is responsible for aggregated the geometry, and packing the data
	into a vertex buffer for rendering.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "visible.h"
#include "vismesh.h"
#include "scenecontext.h"
#include "engine/debugoutput.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include "render/render.h"
#include "render/vertexbuffer.h"
#include "render/shadowvolume.h"
#include "render/light.h"
#include <math.h>

//
// Constants
// 
const unsigned int LINE_LENGTH = 5;

//
// RTTI declaration
//
KIMPLEMENT_RTTI( VisMesh, Visible );

//
// Constructor
//
VisMesh::VisMesh() :
	m_meshDirty(true)
{
}

VisMesh::VisMesh( shared_ptr<Geometry> geometry) :
	m_meshDirty(true),
	m_geometry(geometry)
{
}

//
// Copy Constructor
//
VisMesh::VisMesh( shared_ptr<VisMesh> mesh ) :
	m_meshDirty(true),
	m_geometry( mesh->m_geometry )
{
}

//
// Destructor
//
VisMesh::~VisMesh()
{
}

//
// createGeometry
//
shared_ptr<Geometry> VisMesh::createGeometry()
{
	// Regardless we'll assume the mesh is dirty after this operation
	m_meshDirty = true;

	// If the geometry already exists, return this instance
	if ( m_geometry ) 
		return m_geometry;

	// Otherwise, return a new instance
	else 
	{
		m_geometry.reset( new Geometry );
		return m_geometry;
	}
}

//
// getGeometry
//
shared_ptr<Geometry> VisMesh::getGeometry(bool readOnly)
{
	if ( !readOnly ) m_meshDirty = true;
	return m_geometry;
}

//
// setGeometry
//
void VisMesh::setGeometry( shared_ptr<Geometry> geometry )
{
	m_geometry = geometry;
	m_meshDirty = true;
}

//
// OnAttach
//
bool VisMesh::OnAttach(SceneContext * context)
{
	// Call Base Class
	if ( !Visible::OnAttach( context ) )
		return false;

	// During the attach phase, if we have a valid geometry for
	// this mesh, create the corresponding vertex buffer and share
	// the vertex data
	if ( m_geometry )
	{
		// Do we have a valid vertex cache
		if ( m_geometry->m_vbcache  )
		{
			// Use it instead
			m_vb = m_geometry->m_vbcache;

			return true;
		}
		else
		{
			// Check if we have bounding information for this visible mesh and generate it if necessary
			if ( !m_localBound.isValid() )
				Geometry::createSphere( m_geometry, m_localBound.m_center, m_localBound.m_radius );

			// If normal information is needed, and doesn't exist, create it
			if ( ( ( m_geometry->m_enabledBuffers & NORMALS ) == NORMALS ) && 
				 ( !m_geometry->m_normalBuffer || m_geometry->m_normalBuffer->size() == 0 ) )
				Geometry::createNormals( m_geometry );

			// If tangent information is needed, and doesn't exist, create it
			if ( ( ( m_geometry->m_enabledBuffers & TANGENT_S ) == TANGENT_S ) && 
				( !m_geometry->m_tangentBasisS || m_geometry->m_tangentBasisS->size() == 0 ) )
				Geometry::createTangents( m_geometry );

			// Ask the render to create a vb with our parameters
			Render * render = context->currentRenderer;
			if ( !render ) return false;

			m_vb.reset( render->CreateVB( m_geometry->m_enabledBuffers,
									STATIC,
									m_geometry->m_vertexCount,
									m_geometry->m_indexCount ) );

			// Check whether we have a valid VB
			if ( !m_vb ) return false;
			
			// If we are a shadow caster, then initialize the shadow volume,
			// given the geometry, it will approximate the vertex buffers it
			// needs to create.
			if ( m_isShadowCaster ) {

				// Create a new shadow volume
				m_shadowVolume.reset( new ShadowVolume() );

				// Initialize the shadow volume
				if ( !m_shadowVolume->create( *render, m_geometry ) )
					return false;
			}

			return true;
		}
	}

	return false;
}

//
// OnDetach
//
bool VisMesh::OnDetach(SceneContext * context)
{
	// Invalidate the vertex buffer
	m_vb.reset();

	return true;
}

//
// OnPreRender
//
bool VisMesh::OnPreRender(SceneContext * context)
{
	// Call the base method to make sure we're renderable
	if ( !Visible::OnPreRender( context ) )
		return false;

	if ( m_meshDirty && m_geometry && m_vb )
	{
		// During PreRender, if the mesh is dirty, upload
		// the geometry to the vertex buffer by sharing the
		// vertices and explicitly dirtying the vertex buffer also
		m_vb->shareBuffers( *m_geometry.get() );

		// Setup the primitive count and type of the vb
		m_vb->setPrimitveType( m_geometry->m_primitiveType );
		m_vb->setPrimitveCount( m_geometry->m_primitiveCount );

		// Now the vb is dirty. Set it to upload during the next render pass.
		m_vb->setDirty();

		// But our mesh isn't considered dirty anymore
		m_meshDirty = false;
	}

	return true;
}

//
// OnRender
//
bool VisMesh::OnRender(SceneContext * context)
{
	// Render the vertex buffer if it's available
	if ( m_vb )
	{
		Render * render = context->currentRenderer;

		if ( render )
		{
			render->RenderVB( m_vb.get() );

			return true;
		}
	}

	return false;
}

//
// OnPostRender
//
bool VisMesh::OnPostRender(SceneContext * context)
{
	// Check whether we have any active lights and a shadow volume
	if ( context->currentLights.size() && m_shadowVolume )
	{
		// We only generate shadows for the primary light
		shared_ptr<Light> primaryLight = context->currentLights[0];

		// Has this light moved, if so, update the shadow volume
		if ( primaryLight->getMoved() ) 
		{
			// Determine the light direction in world space
			Point3 lightDirectionInWorldSpace = primaryLight->getPosition() - m_worldViewMatrix.pos;

			// Transform the light from world space to object space
			Point3 lightDirectionInObjectSpace = lightDirectionInWorldSpace * Matrix4( m_worldViewMatrix ).inverse().transpose();

			// Update the shadow volume
			m_shadowVolume->update( lightDirectionInObjectSpace );
		}
	}

	// Debug Drawing

	// Display the normals for the mesh
	if ( context->debugOutput->getDrawNormals() ) Debug_DrawNormals( context );

	// Display the tangents for the mesh
	if ( context->debugOutput->getDrawTangents() ) Debug_DrawTangents( context );

	// Display the bounds for the visible object
	if ( context->debugOutput->getDrawBounds() ) Debug_DrawBounds( context );

	/// Draw the shadow volume for the mesh
	if ( context->debugOutput->getDrawShadowVolumes() ) Debug_DrawShadowVolume( context );

	return true;
}

//
// OnRenderShadow
//
bool VisMesh::OnRenderShadow( SceneContext * context )
{
	// Make sure we have a renderer and a shadow volume
	Render * render = context->currentRenderer;
	if ( !render || !m_shadowVolume ) return false;

	// Delegate rendering of the shadow volume to itself
	m_shadowVolume->render( render );

	return true;
}

//
// Debug_DrawNormals
//
void VisMesh::Debug_DrawNormals(SceneContext * context)
{
	// Normals are always displayed in world space
	context->currentRenderer->SetMatrix( MODELVIEW, STORE, getWorldMatrix() );

	// Grab the geometry
	shared_ptr<Geometry> geom( getGeometry( true ) );
	if ( !geom ) return;

	// Make sure we have the vertex and normal information
	if ( !geom->m_vertexBuffer || !geom->m_normalBuffer ) return;

	// This is the definition of our renderable geometry
	Geometry renderGeom; 
	renderGeom.m_primitiveType = LINES;
	renderGeom.m_enabledBuffers = VERTEX | COLOR;
	renderGeom.m_primitiveCount = (unsigned int)geom->m_vertexBuffer->size() / 3;
	renderGeom.m_vertexBuffer.reset( new vector<float> );

	// Colors for rendering
	float YELLOW = kmath::ltof( CreateColor( 1, 1, 0, 1 ) );

	// Fill the line list with the normal information
	for( unsigned int vertex = 0; vertex < geom->m_vertexBuffer->size(); vertex += 3 )
	{
		Point3 p(geom->m_vertexBuffer->at(vertex+0),
				 geom->m_vertexBuffer->at(vertex+1),
				 geom->m_vertexBuffer->at(vertex+2));

		Point3 n(geom->m_normalBuffer->at(vertex+0),
				 geom->m_normalBuffer->at(vertex+1),
				 geom->m_normalBuffer->at(vertex+2));

		// Transform the point into world space coordinates
		p += getWorldMatrix().at;

		// Scale it so it looks good
		n *= LINE_LENGTH;

		// Push the vertex and scaled normal onto the line list
		renderGeom.m_vertexBuffer->push_back( p.x );
		renderGeom.m_vertexBuffer->push_back( p.y );
		renderGeom.m_vertexBuffer->push_back( p.z );
		renderGeom.m_vertexBuffer->push_back( YELLOW );
		renderGeom.m_vertexBuffer->push_back( p.x + n.x );
		renderGeom.m_vertexBuffer->push_back( p.y + n.y );
		renderGeom.m_vertexBuffer->push_back( p.z + n.z );
		renderGeom.m_vertexBuffer->push_back( YELLOW );
	}

	// Render the geometry
	context->currentRenderer->RenderGeometry( &renderGeom );
}

//
// Debug_DrawTangents
//
void VisMesh::Debug_DrawTangents(SceneContext * context)
{
	// Tangents are always displayed in world space
	context->currentRenderer->SetMatrix( MODELVIEW, STORE, getWorldMatrix() );

	// Grab the geometry
	shared_ptr<Geometry> geom = getGeometry( true );
	if ( !geom ) return;

	// Make sure we have the vertex and tangent information
	if ( !geom->m_vertexBuffer || !geom->m_tangentBasisS ||
		 !geom->m_tangentBasisT || !geom->m_tangentBasisST ) return;

	// This is the definition of our renderable geometry
	Geometry renderGeom; 
	renderGeom.m_primitiveType = LINES;
	renderGeom.m_enabledBuffers = VERTEX | COLOR;
	renderGeom.m_primitiveCount = (unsigned int)geom->m_vertexBuffer->size() / 3;
	renderGeom.m_vertexBuffer.reset( new vector<float> );

	// Colors for rendering
	float RED	= kmath::ltof( CreateColor( 1, 0, 0, 1 ) );
	float BLUE	= kmath::ltof( CreateColor( 0, 0, 1, 1 ) );
	float GREEN = kmath::ltof( CreateColor( 0, 1, 0, 1 ) );

	// Fill the line list with the normal information
	for( unsigned int vertex = 0; vertex < geom->m_vertexBuffer->size(); vertex += 3 )
	{
		Point3 p(geom->m_vertexBuffer->at(vertex+0),
				 geom->m_vertexBuffer->at(vertex+1),
				 geom->m_vertexBuffer->at(vertex+2));

		Point3 s(geom->m_tangentBasisS->at(vertex+0),
				 geom->m_tangentBasisS->at(vertex+1),
				 geom->m_tangentBasisS->at(vertex+2));

		Point3 t(geom->m_tangentBasisT->at(vertex+0),
				 geom->m_tangentBasisT->at(vertex+1),
				 geom->m_tangentBasisT->at(vertex+2));

		Point3 st(geom->m_tangentBasisST->at(vertex+0),
				  geom->m_tangentBasisST->at(vertex+1),
				  geom->m_tangentBasisST->at(vertex+2));

		// Transform the point into world space coordinates
		p += getWorldMatrix().at;

		// Scale it so it looks good
		s *= LINE_LENGTH;
		t *= LINE_LENGTH;
		st *= LINE_LENGTH;

		// Push the vertex and scaled tangents onto the line list
		renderGeom.m_vertexBuffer->push_back( p.x );
		renderGeom.m_vertexBuffer->push_back( p.y );
		renderGeom.m_vertexBuffer->push_back( p.z );
		renderGeom.m_vertexBuffer->push_back( GREEN );
		renderGeom.m_vertexBuffer->push_back( p.x + s.x );
		renderGeom.m_vertexBuffer->push_back( p.y + s.y );
		renderGeom.m_vertexBuffer->push_back( p.z + s.z );
		renderGeom.m_vertexBuffer->push_back( GREEN );

		renderGeom.m_vertexBuffer->push_back( p.x );
		renderGeom.m_vertexBuffer->push_back( p.y );
		renderGeom.m_vertexBuffer->push_back( p.z );
		renderGeom.m_vertexBuffer->push_back( RED );
		renderGeom.m_vertexBuffer->push_back( p.x + t.x );
		renderGeom.m_vertexBuffer->push_back( p.y + t.y );
		renderGeom.m_vertexBuffer->push_back( p.z + t.z );
		renderGeom.m_vertexBuffer->push_back( RED );

		renderGeom.m_vertexBuffer->push_back( p.x );
		renderGeom.m_vertexBuffer->push_back( p.y );
		renderGeom.m_vertexBuffer->push_back( p.z );
		renderGeom.m_vertexBuffer->push_back( BLUE );
		renderGeom.m_vertexBuffer->push_back( p.x + st.x );
		renderGeom.m_vertexBuffer->push_back( p.y + st.y );
		renderGeom.m_vertexBuffer->push_back( p.z + st.z );
		renderGeom.m_vertexBuffer->push_back( BLUE );
	}

	// Render the geometry
	context->currentRenderer->RenderGeometry( &renderGeom );
}

//
// Debug_DrawBounds
// Draw the bounds for this visible object
//
void VisMesh::Debug_DrawBounds( SceneContext * context )
{
	// Check if the bounds are valid
	if ( m_worldBound.isValid() )
	{
		// Compute the dimensions of our circles
		float radius = m_worldBound.getRadius();
		Point3 position = m_worldBound.getCenter();

		// This geometry define the circles which represent our light
		Geometry renderGeom;
		renderGeom.m_primitiveType = LINES;
		renderGeom.m_enabledBuffers = VERTEX | COLOR;
		renderGeom.m_primitiveCount = (360 / 10) * 2;
		renderGeom.m_vertexBuffer.reset( new vector<float> );

		// The color is Yellow for bounding spheres
		long lightColor = CreateColor( 0, 1, 1, 1 );

		// Conver the Light Color
		float LightColor = kmath::ltof( lightColor );

		// Increment around the 360 degree circle (stepping by 10 degrees)
		// and caclculting the points.
		for( int degree = 0; degree < 360; degree += 10 )
		{
			float x = (float)cos( kmath::toRadians( (float)degree ) ) * radius;
			float y = (float)sin( kmath::toRadians( (float)degree ) ) * radius;
			float x2 = (float)cos( kmath::toRadians( (float)degree+10 ) ) * radius;
			float y2 = (float)sin( kmath::toRadians( (float)degree+10 ) ) * radius;

			renderGeom.m_vertexBuffer->push_back( x + position.x );
			renderGeom.m_vertexBuffer->push_back( 0 );
			renderGeom.m_vertexBuffer->push_back( y - position.z );
			renderGeom.m_vertexBuffer->push_back( LightColor );
			renderGeom.m_vertexBuffer->push_back( x2 + position.x );
			renderGeom.m_vertexBuffer->push_back( 0 );
			renderGeom.m_vertexBuffer->push_back( y2 - position.z );
			renderGeom.m_vertexBuffer->push_back( LightColor );

			renderGeom.m_vertexBuffer->push_back( x + position.x );
			renderGeom.m_vertexBuffer->push_back( y + position.y );
			renderGeom.m_vertexBuffer->push_back( -position.z );
			renderGeom.m_vertexBuffer->push_back( LightColor );
			renderGeom.m_vertexBuffer->push_back( x2 + position.x );
			renderGeom.m_vertexBuffer->push_back( y2 + position.y );
			renderGeom.m_vertexBuffer->push_back( -position.z );
			renderGeom.m_vertexBuffer->push_back( LightColor );
		}

		// Render the geometry
		context->currentRenderer->RenderGeometry( &renderGeom );
	}
}

//
// Debug_DrawShadowVolume
// Draw the shadow volume for the mesh
//
void VisMesh::Debug_DrawShadowVolume( SceneContext * context )
{
	// Shadows are always displayed in world space
	context->currentRenderer->SetMatrix( MODELVIEW, STORE, getWorldMatrix() );

	// Simply draw the shadow volume buffer
	OnRenderShadow( context );
}

// -------------------------------------------------------


//
// OnLoadStream
//
bool VisMesh::OnLoadStream( kistream & istr )
{
	// Call base class
	Visible::OnLoadStream( istr );

	// Load the geometry
	istr >> m_geometry;

	return true;
}

//
// OnSaveStream
//
bool VisMesh::OnSaveStream( kostream & ostr ) const
{
	// Call base class
	Visible::OnSaveStream( ostr );

	// Save the geometry
	ostr << m_geometry;

	return true;
}