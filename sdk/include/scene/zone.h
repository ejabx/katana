/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		zone.h
	Author:		Eric Bryant

	A "Zone" is essentially an octree which handles frustum culling for the
	geometry. The leaves of the octree are further divided into BSP nodes
	for collision detection. The octree depth is determined by the number
	of triangles in model.
*/

#ifndef _ZONE_H
#define _ZONE_H

namespace Katana
{

// Forward Declarations
struct BSPNode;
struct Geometry;
struct SceneContext;
class VertexBuffer;
class IndexBuffer;

///
/// Zone
///
class Zone
	: public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(Zone)

public:

	/// Default constructor
	Zone();

	/// Constructor which takes a zone identifier
	Zone( unsigned short zoneID );

	/// Constructor which takes geometry
	Zone( unsigned short zoneID, shared_ptr<Geometry> geometry, unsigned int uiMaximumDepth, unsigned int uiMinimumTriCount  );

	/// Creates a zone given the geometry
	bool createZone( shared_ptr<Geometry> geometry, unsigned int uiMaximumDepth, unsigned int uiMinimumTriCount  );

	/// Initializes the Zone. This prepares it for rendering
	/// by creating a vertex buffer for our geometry
	bool preRender( SceneContext * context );

	/// Renders the Zone.
	bool render( SceneContext * context );

protected:

	/// Recursively check all the nodes for visibility and add their indices
	/// to our destination index buffer if so
	void checkAndRenderNodes( SceneContext * context, unsigned int uiCurrentNodeIndex, 
							  unsigned short * pSrcIndexData, unsigned short * pDestIndexData, 
							  unsigned int & uiTotalIndexCount );

protected:

	unsigned short				m_zoneID;				/// Zone identifier
	vector<BSPNode>				m_bspNodes;				/// Collection of BSP Nodes	
	shared_ptr<Geometry>		m_geometry;				/// Reference to renderable geometry
	shared_ptr<VertexBuffer>	m_vb;					/// Reference to the vertex buffer
	shared_ptr<IndexBuffer>		m_ib;					/// Reference to the index buffer

};

KIMPLEMENT_STREAM( Zone );

}; // Katana

#endif // _ZONE_H
