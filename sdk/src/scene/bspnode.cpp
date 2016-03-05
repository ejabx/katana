/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		bspnode.h
	Author:		Eric Bryant
	Credits:	Dan Ginsburg, 2000, "Game Gems 1: Octree Construction"
				for linear style Octree data structures.

	BSP Nodes
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include "math/intersect.h"
#include "bspnode.h"
#include <math.h>

// --------------------------------------------------------------
// Macros
// --------------------------------------------------------------

#define MIN_POINT3( point )	point.x < point.y ? point.x : point.y < point.z ? point.y : point.z
#define MAX_POINT3( point )	point.x > point.y ? point.x : point.y > point.z ? point.y : point.z

// --------------------------------------------------------------
// BSPNodeConstructor 
// --------------------------------------------------------------

//
// Constructor
//
BSPNodeConstructor::BSPNodeConstructor( int zone, shared_ptr<Geometry> geometry, vector<BSPNode> & bspNodes )
	: m_zone( zone )
	, m_geometry( geometry )
	, m_bspNodes( bspNodes )
	, m_maximumNodeDepth( MAXIMUM_NODE_DEPTH )
	, m_minimumTriangleCount( MINIMUM_TRIANGLE_COUNT )
	, m_allowDuplicateTris( false )
{
}

//
// makeBSP
//
bool BSPNodeConstructor::makeBSP()
{
	// Reset the node parameters
	m_currentNodeDepth = 0;

	// Reset the reordered triangle index list
	m_reorderedIndexList.reset( new vector< unsigned short > );

	// Generate a sphere which encompasses the geometry. This will be our
	// root bounding volume
	AxisAlignedBox initialBox;
	Geometry::createBox( m_geometry, initialBox.m_minimum, initialBox.m_maximum );

	// Octree has to be square, so we set the box to the minimum/maximum value
	float fMinimum = MIN_POINT3( initialBox.m_minimum );
	float fMaximum = MAX_POINT3( initialBox.m_maximum );

	// Create a point which is the minimum and maximum extents and replace that in the box
	const Point3 vMinimum = Point3( fMinimum, fMinimum, fMinimum );
	const Point3 vMaximum = Point3( fMaximum, fMaximum, fMaximum );
	initialBox.m_minimum = vMinimum;
	initialBox.m_maximum = vMaximum;

	// Initialize the root of the BSP with this bound and zone
	BSPNode root;
	root.zone = m_zone;
	root.box = initialBox;
	root.bound = Bound( initialBox ); // Convert the AABB to a sphere for visibility testing
	root.faceCount = m_geometry->m_indexCount / 3;
	root.faceIndex = m_triangleIndexList.size();

	// We will allocate enough room for the maximum node depth.
	// TODO: This is a hack. I need to understand why this is needed.
	unsigned int uiMaxNodeCount = ( pow( 8, m_maximumNodeDepth ) - 1 ) / 7;
	m_bspNodes.reserve( uiMaxNodeCount );

	// Reserve space in the triangle index list in order to avoid reallocations (which are expensive)
	// TODO: We overestimate the index count because of duplicates. We need to make sure a triangle
	// can only belong to one node.
	m_triangleIndexList.reserve( m_geometry->m_indexCount * 2 );

	// Fill the triangle index list table
	for( unsigned int i = 0; i < root.faceCount; i++ )
		m_triangleIndexList.push_back( i );

	// Add the root node to the collection of BSP nodes
	m_bspNodes.push_back( root );

	// Allocate space for the triangle reference list and fill it
	m_triangleReferenceList.resize( root.faceCount );
	for( unsigned int i = 0; i < root.faceCount; i++ )
		m_triangleReferenceList[i] = false;

	// Recursively fill the additional nodes until we reach the target level
	// We start at the root which is index 0.
	recursiveBuildTree( 0 );

	// Replace the triangle index list with our reordered list
	m_geometry->m_indexBuffer = m_reorderedIndexList;

	// Update the index count
	m_geometry->m_indexCount = m_reorderedIndexList->size();

	return true;
}

//
// recursiveBuildTree
// Recursive function which fills the BSP nodes until the node limit is reached
//
void BSPNodeConstructor::recursiveBuildTree( unsigned int uiNodeIndex )
{
	// Increment the node depth
	m_currentNodeDepth++;

	// Grab the node by index
	BSPNode & currentNode = m_bspNodes[uiNodeIndex];

	// If this node has more than the threshold of triangles,
	// and we are not at the maximum node depth, create children
	if ( ( currentNode.faceCount > m_minimumTriangleCount ) && ( m_currentNodeDepth < m_maximumNodeDepth ) )
	{
		// Iterate over the children and initialize them
		for( unsigned int uiChildIdx = 0; uiChildIdx < MAX_OCTANTS; uiChildIdx++ )
		{
			BSPNode childNode;

			// Set this node's child reference
			currentNode.children[uiChildIdx] = m_bspNodes.size();

			// Partition a new bounding box for the child
			childNode.box = currentNode.box.getOctant( (Octant)uiChildIdx );

			// Convert the AABB to a sphere for visibility testing
			childNode.bound = Bound( childNode.box );

			// Enlarge the radius of the bound to reduce inaccuracies from converting
			// the box to a sphere
//			childNode.bound.m_radius *= 1.1f;

			// Determine which of the parent's triangles lie within this new node
			childNode.faceCount = 0;
			childNode.faceIndex = m_triangleIndexList.size();
			for( unsigned int j = 0; j < currentNode.faceCount; j++ )
			{
				unsigned short uiTriIdx = m_triangleIndexList[currentNode.faceIndex + j];
				if ( triangleBoxIntersection( m_geometry, childNode.box, uiTriIdx ) )
				{
					m_triangleIndexList.push_back( uiTriIdx );
					childNode.faceCount++;
				}
			}

			// Add the new node to the collection of BSP nodes
			m_bspNodes.push_back( childNode );

			// Recurse on this child node's children
			recursiveBuildTree( m_bspNodes.size() - 1 );
		}
	}
	// This is a leaf within the octree. Add the indices to the reordered index list
	else
	{
		// This is the new face index
		unsigned short newFaceIndex = m_reorderedIndexList->size() / 3;

		// Get the triangle index
		unsigned short * pTriangleIndex = &m_triangleIndexList[currentNode.faceIndex];

		// Keep track of how many indices were already referenced. We need to subtract
		// this from our face count
		unsigned short uiDuplicateTriangleCount = 0;

		// Iterate over all faces
		for( unsigned int j = 0; j < currentNode.faceCount; j++ )
		{
			// Grab the triangle index
			unsigned short uiTriIdx = pTriangleIndex[j];

			// Has this triangle been referenced already in another node?
			if ( m_allowDuplicateTris || !m_triangleReferenceList[uiTriIdx] )
			{
				// Grab the indices of our triangle
				unsigned short uiIndex1 = m_geometry->m_indexBuffer->at( uiTriIdx*3+0 );
				unsigned short uiIndex2 = m_geometry->m_indexBuffer->at( uiTriIdx*3+1 );
				unsigned short uiIndex3 = m_geometry->m_indexBuffer->at( uiTriIdx*3+2 );

				// Store them in the reordered list
				m_reorderedIndexList->push_back( uiIndex1 );
				m_reorderedIndexList->push_back( uiIndex2 );
				m_reorderedIndexList->push_back( uiIndex3 );

				// Set this triangle index as already referenced
				m_triangleReferenceList[uiTriIdx] = true;
			}
			else
			{
				uiDuplicateTriangleCount++;
			}
		}

		// Remove the duplicate triangles from our face count
		currentNode.faceCount -= uiDuplicateTriangleCount;

		// Modify our face index
		currentNode.faceIndex = newFaceIndex;
	}

	// Decrement the recursion depth
	m_currentNodeDepth--;
}

//
// triangleBoxIntersection
// Determines whether the box (specified by the bounds) intersects with the triangle starting
// at the given index.
//
bool BSPNodeConstructor::triangleBoxIntersection( shared_ptr<Geometry> geometry, const AxisAlignedBox & box, unsigned short uiStartTriIdx )
{
	// Grab the indices of our triangle
	unsigned short uiIndex1 = geometry->m_indexBuffer->at( uiStartTriIdx*3+0 );
	unsigned short uiIndex2 = geometry->m_indexBuffer->at( uiStartTriIdx*3+1 );
	unsigned short uiIndex3 = geometry->m_indexBuffer->at( uiStartTriIdx*3+2 );

	// Reinterpret out vertex buffer points (which are floats) to Point3s
	const Point3 * pPoints = reinterpret_cast<const Point3 *>( &geometry->m_vertexBuffer->front() );

	// Grab the corresponding vertices
	const Point3 & vert0 = pPoints[uiIndex1], vert1 = pPoints[uiIndex2], vert2 = pPoints[uiIndex3];

	// Perform the intersection test
	return kmath::testIntersect( box, vert0, vert1, vert2 );
}