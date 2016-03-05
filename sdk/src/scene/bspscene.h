/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		bspscene.h
	Author:		Eric Bryant

	BSP Scene is used for rendering indoor environments, and to an lesser
	extent, outdoor environments. 

	Although at the heart are BSP nodes/leaves, the BSPScene has a higher
	level of occlusion which consist of "Zones" and "Portals".

	A "Zone" is essentially an octree which handles frustum culling for the
	geometry. The leaves of the octree are further divided into BSP nodes
	for collision detection. The octree depth is determined by the number
	of triangles in model.

	"Portals" are planes which connect the zones to each other. A Zone 
	cannot view another Zone unless they are connected by a Portal and the 
	Portal is visible.
*/

#ifndef _BSPSCENE_H
#define _BSPSCENE_H

namespace Katana
{

// Forward Declarations
class Zone;
class Portal;

///
/// BspScene
///
class BSPScene
	: public VisNode
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(BSPScene)
	KDECLARE_SCRIPT;

public:
	/// Constructor
	BSPScene();

	/// Constructor which will construct BSP data from a geometry object by 
	/// created a default zone for the geometry.
	BSPScene( shared_ptr<Geometry> geometry );

	/// Constructor which will construct BSP data from a geometry object by 
	/// created a default zone for the geometry. The depth of the BSP data
	/// is bounded by the specified maximum depth and minimum triangle count
	BSPScene( shared_ptr<Geometry> geometry, unsigned int uiMaximumDepth, unsigned int uiMinimumTriCount );

public:

	/// BSPScene will attempt to create the VB specific to the device when it
	/// is first attached to the scene.
	virtual bool OnAttach(SceneContext * context);

	/// BSPScene will release it's reference count to the VB, which may delete
	/// it if it's reference count is zero.
	virtual bool OnDetach(SceneContext * context);

	/// BSPScene will upload the mesh geometry to the VB during this event.
	virtual bool OnPreRender(SceneContext * context);

	/// BSPScene will render the VB during this event.
	virtual bool OnRender(SceneContext * context);

	/// BSPScene will render debug information during this event
	virtual bool OnPostRender(SceneContext * context);

public:

	/// Adds a zone to the BSP Scene
	void addZone( shared_ptr<Zone> zone );

	/// Creates a zone from the geometry and adds the zone to the BSP Scene
	void createZone( shared_ptr<Geometry> geometry, unsigned int uiMaximumDepth, unsigned int uiMinimumTriCount );

protected:

	/// Retrieves the next available zone identifier
	unsigned short getNextZoneID() const;

protected:

	/// The collection of zones in this BSP Scene
	vector< shared_ptr<Zone> > m_zones;

};

//
// Inline
//

//
// BSPScene::addZone
//
inline void BSPScene::addZone( shared_ptr<Zone> zone )
{
	m_zones.push_back( zone );
}

inline unsigned short BSPScene::getNextZoneID() const
{
	return (unsigned short)m_zones.size() + 1;
}

KIMPLEMENT_STREAM( BSPScene );
KIMPLEMENT_SCRIPT( BSPScene );

}; // Katana

#endif // _BSPSCENE_H