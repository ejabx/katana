/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		bspgeometry.h
	Author:		Eric Bryant

	Geometry specific to a BSP Scene
*/

#ifndef _BSPGEOMETRY_H
#define _BSPGEOMETRY_H

namespace Katana
{

///
/// BSPVertex
///
struct BSPVertex
{
	Point3	vPosition;		/// (x, y, z) position. 
	Point2	vTextureCoord;	/// (u, v) texture coordinate
	Point2	vLightmapCoord;	/// (u, v) lightmap coordinate
	Point3	vNormal;		/// (x, y, z) normal vector
	long	color;			/// (r, g, b, a) color
};

///
/// BSPFace
///
struct BSPFace
{
	int		textureID;				/// The index into the texture array 
	int		effect;					/// The index for the effects (or -1 = n/a) 
	int		type;					/// 1=polygon, 2=patch, 3=mesh, 4=billboard 
	int		startVertIndex;			/// The starting index into this face's first vertex 
	int		numOfVerts;				/// The number of vertices for this face 
	int		meshVertIndex;			/// The index into the first meshvertex 
	int		numMeshVerts;			/// The number of mesh vertices 
	int		lightmapID;				/// The texture index for the lightmap 
	int		lMapCorner[2];			/// The face's lightmap corner in the image 
	int		lMapSize[2];			/// The size of the lightmap section 
	Point3	lMapPos;				/// The 3D origin of lightmap. 
	Point3	lMapVecs[2];			/// The 3D space for s and t unit vectors. 
	Point3	vNormal;				/// The face normal. 
	int		reserved[2];			/// The bezier patch dimensions. 
};

///
/// BSPGeometry
///
struct BSPGeometry
	: public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(BSPGeometry);

	/// Constructor
	BSPGeometry();

	/// Total number of vertices
	unsigned int	m_vertexCount;

	/// Total number of faces
	unsigned int	m_faceCount;

	shared_ptr< vector< BSPVertex > >	m_vertexBuffer;		/// Vertex Buffer
	shared_ptr< vector< BSPFace > >		m_faceBuffer;		/// Face Information
};

KIMPLEMENT_STREAM( BSPGeometry );


//
// Inline
//

//
// BSPGeometry::constructor
//
inline BSPGeometry::BSPGeometry()
	: m_vertexCount( 0 )
	, m_faceCount( 0 )
{}

};

#endif // _BSPGEOMETRY_H