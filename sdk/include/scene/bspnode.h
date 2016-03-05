/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		bspnode.h
	Author:		Eric Bryant

	BSP Nodes
*/

#ifndef _BSPNODE_H
#define _BSPNODE_H

namespace Katana
{

// Enumeration which aids in determine front or back child index
enum { BSP_FRONT_CHILD = 0, BSP_BACK_CHILD = 1 };

///
/// BSPNode
///
struct BSPNode
{
	short			children[8];	/// Because this is an octree-style BSP node, these are the eight children.
									/// If this is a BSP tree, only the first members are set: 0 = front, 1 = back
	short			neighbors[6];	/// Because this is an octree-style BSP node, these are the six neighbors.
	Plane			plane;			/// Plane which divides the BSP node in half-space
	AxisAlignedBox	box;			/// The box is used for occlusion culling and triangle intersection testing. It is more accurate
	Bound			bound;			/// This is the bounding volume (sphere) used for visibility testing
	short			material;		/// Index referencing the material to use to render this node
	unsigned short	faceIndex;		/// The first index of this leaf's faces. -1 if this node is not a leaf
	unsigned short  faceCount;		/// The number of faces belonging to this leaf
	short			zone;			/// The zone index this node belongs to

	bool isLeaf() const;	/// Function returns true if this node is a leaf
	bool isBSP() const;		/// Function returns true if this node is acting as a BSP
};

///
/// BSPNodeBuilder
/// Helper class which recursively constructs BSP nodes given geometry
/// and a collection of BSP nodes to fill.
///
struct BSPNodeConstructor
{
	/// Constructor which takes all the inputs to the BSP generator
	BSPNodeConstructor( int zone, shared_ptr<Geometry> geometry, vector<BSPNode> & bspNodes );

	/// Sets how the constructor will limit the nodes of the BSP. By default we set a maximum triangle count of 20
	void setMaximumNodeDepthLimit( unsigned int uiMaxNodeDepth );
	void setMinimumTriangleCountPerLeaf( unsigned int uiMinTrianglePerLeaf );

	/// Execute the generator
	bool makeBSP();

public:
	enum { MAXIMUM_NODE_DEPTH = 5, MINIMUM_TRIANGLE_COUNT = 100 };

private:

	/// Recursive function which fills the BSP nodes until the node limit is reached
	void recursiveBuildTree( unsigned int uiNodeIndex );

	/// Determines whether the box (specified by the bounds) intersects with the triangle starting
	/// at the given index.
	bool triangleBoxIntersection( shared_ptr<Geometry> geometry, const AxisAlignedBox & box, unsigned short uiStartTriIdx );

private:
	int										m_zone;					/// Parent zone
	shared_ptr<Geometry>					m_geometry;				/// Geometry used for construction
	vector<BSPNode> &						m_bspNodes;				/// Reference to output BSP nodes
	vector<unsigned short>					m_triangleIndexList;	/// Reference to output triangle index list
	shared_ptr< vector<unsigned short> >	m_reorderedIndexList;	/// This index list has been reordered for the octree nodes
	unsigned int							m_maximumNodeDepth;		/// Maximum Node Depth
	unsigned int							m_minimumTriangleCount;	/// Minimum Triangle Count
	unsigned int							m_currentNodeDepth;		/// The current node count
};


//
// Inline
//

//
// BSPNode::constructor
//
inline BSPNode::BSPNode()
	: faceIndex( -1 )
	, material( -1 )
	, faceCount( 0 )
	, zone( 0 )
{
	children[0] = children[1] = children[2] = children[3] =
	children[4] = children[5] = children[6] = children[7] =
	neighbors[0] = neighbors[1] = neighbors[2] =
	neighbors[3] = neighbors[4] = neighbors[5] = -1;
}

//
// BSPNode::isLeaf
//
inline bool BSPNode::isLeaf() const
{
	return children[0] == -1;
}

//
// BSPNode::isBSP
//
inline bool BSPNode::isBSP() const
{
	return children[2] == -1;
}

//
// BSPNodeConstructor::setMaximumNodeLimit
//
inline void BSPNodeConstructor::setMaximumNodeDepthLimit( unsigned int uiMaxNodeDepth )
{
	m_maximumNodeDepth = uiMaxNodeDepth;
}

//
// BSPNodeConstructor::setMaximumTriangleCountPerLeaf
//
inline void BSPNodeConstructor::setMinimumTriangleCountPerLeaf( unsigned int uiMinTrianglePerLeaf )
{
	m_minimumTriangleCount = uiMinTrianglePerLeaf;
}

}; // Katana

#endif // _BSPNODE_H