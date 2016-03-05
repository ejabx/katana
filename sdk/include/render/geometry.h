/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		geometry.h
	Author:		Eric Bryant

	Represents any geometric primitive within the game. It is
	a collection of per vertex data (texture coordinates, color, 
	normal, and tangent basis), which is organized by some primitive
	type (triangle strip, fan, etc.).
*/

#ifndef _GEOMETRY_H
#define _GEOMETRY_H

namespace Katana
{

//
// Forward Declarations
//
class VertexBuffer;
class Streamable;

///
/// Geometry
/// Structure which represents any geometry primitive within the
/// game engine.
///
struct Geometry 
	: public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(Geometry);
	KDECLARE_SCRIPT;

	/// Constructor.
	Geometry();

	/// Which primitive is the data organized for (default is TRIANGLE_LIST)
	PrimitiveType	m_primitiveType;

	/// Which buffers are used (vertex, normals, texture, color arrays). Default is VERTEX
	BufferTypes		m_enabledBuffers;

	/// Total number of primitives
	unsigned int	m_primitiveCount;

	/// Total number of vertices
	unsigned int	m_vertexCount;

	/// Total number of indices
	unsigned int	m_indexCount;

	/// Total number of triangle strips
	unsigned int	m_triangleStripCount;

	shared_ptr< vector<float> > 			m_vertexBuffer;		/// Vertex Buffers
	shared_ptr< vector<float> > 			m_normalBuffer;		/// Normal Buffers
	shared_ptr< vector<float> > 			m_texture0Buffer;	/// Texture 0 Buffers
	shared_ptr< vector<float> > 			m_texture1Buffer;	/// Texture 1 Buffers
	shared_ptr< vector<float> > 			m_colorBuffer;		/// Color Buffers
	shared_ptr< vector<unsigned short> >	m_indexBuffer;		/// Index Buffers
	shared_ptr< vector<float> > 			m_tangentBasisS;	/// Tangent Buffer S  (for bump mapping)
	shared_ptr< vector<float> > 			m_tangentBasisT;	/// Tangent Buffer T  (for bump mapping)
	shared_ptr< vector<float> > 			m_tangentBasisST;	/// Tangent Buffer ST (for bump mapping)

	/// This is used by VisMesh to determine whether this geometry already has a
	/// vertex buffer associated with it. If so, it will not create another vb
	/// but use the existing one.
	shared_ptr<VertexBuffer>	m_vbcache;

	// Static Functions

	/// Static function which finds the minimum axis-aligned box which contains the set
	/// of points in the Geometry.
	static void createBox( shared_ptr<Geometry> spGeometry, Point3 & vMin, Point3 & vMax );

	/// Static function which finds the sphere containing the points in the Geometry using 
	/// average of the values.
	static void createSphere( shared_ptr<Geometry> spGeometry, Point3 & vCenter, float & fRadius );

	/// Static function which finds cylinder containing the points in the Geometry
	static void createCylinder( shared_ptr<Geometry> spGeometry, Point3 & vCenter, Point3 & vDirection, float & fHeight, float & fRadius );

	/// Static function which generates normals given the vertices in a Geometry
	static void createNormals( shared_ptr<Geometry> spGeometry );

	/// Static function which generates tangent coordinates given the vertices and normals in a Geometry
	static void createTangents( shared_ptr<Geometry> spGeometry );
};

KIMPLEMENT_STREAM( Geometry );
KIMPLEMENT_SCRIPT( Geometry );

}; // Katana


#endif // _GEOMETRY_H