/*
	Katana Engine
	Copyright © 2001-2003 Eric Bryant, Inc.

	File:		terrain.h
	Author:		Eric Bryant

	Terrain Node.
*/

#ifndef _TERRAIN_H
#define _TERRAIN_H

namespace Katana
{

//
// Forward Declarations
//
class TerrainSettings;
class TerrainPatch;
class VertexBuffer;
class IndexBuffer;
struct SceneContext;

///
/// Terrain
///
class Terrain : public VisNode
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(Terrain);
	KDECLARE_SCRIPT;

public:
	/// Default Constructor
	Terrain();

	/// Constructor which takes a height map and world dimensions
	Terrain( TerrainSettings & settings );

	/// Destructor
	virtual ~Terrain();

	/// Constructs the terrain from a height map and world dimensions
	bool construct( TerrainSettings & settings );

	/// Returns whether the terrain was constructed properly
	bool isInitialized() const								{ return m_isInitialized; }

	/// Returns the maximum projected screen error for rendering patches
	float getMaximumScreenError() const						{ return m_maximumScreenError; }

	/// Returns the number of active patches
	unsigned int getActivePatchCount() const				{ return (unsigned int)m_activePatches.size(); }

	/// Returns the number of triangles rendered in the last pass
	unsigned int getTriangleCount() const					{ return m_requiredPatchIndices - 2; }

	/// Returns the patches
	std::vector<TerrainPatch*> getTerrainPatches()			{ return m_patches; }

	/// Called before the terrain is attached to the scene, it will create the vertex
	/// and index buffers and load the shaders.
	virtual bool OnAttach( SceneContext * context );

	/// Called when the terrain is detached from the scene, it will release resources from memory.
	virtual bool OnDetach( SceneContext * context );

	/// Called before actual rendering. Patches are culling and renderable patches are added to the active list of patches.
	/// Also the vertex and pixel shaders are enabled during this call.
	virtual bool OnPreRender( SceneContext * context );

	/// Renders the terrain. It routes the call to either renderUnififed() or renderSplit().
	virtual bool OnRender( SceneContext * context );

protected:

	/// Creates a terrain patch during construct(). Override this method to create your own terrain behavior
	virtual TerrainPatch * createPatch( unsigned int px, unsigned int pz );

	/// Generates the vertex and index buffers
	virtual bool createBuffers( SceneContext * context );

	/// Renders all the terrain patches in a unified vertex buffer. This method assumed each patch as the same lod level.
	virtual void renderUnified( SceneContext * context );

	/// Renders the terrain in separater buffers. This method assuming each patch may have different lod's levels are require stitching.
	virtual void renderSplit( SceneContext * context );

protected:

	enum RenderMethod { RENDER_UNIFIED, RENDER_SPLIT };				/// Enumeration to determine whether the terrain is rendered in one vertex buffer,
																	/// on in seperate buffers with separate lods.

	bool						m_isInitialized;					/// Flags whether the terrain has been constructed/loaded successfully
	std::vector<TerrainPatch*>	m_patches;							/// The collection of patches
	unsigned int				m_patchesX, m_patchesZ;				/// Maximum number of patches in the (X,Z) directions
	unsigned int				m_patchSizeX, m_patchSizeZ;			/// Size of each individual patch in (X,Z) directions
	unsigned int				m_worldHeight;						/// The height of the world
	RenderMethod				m_renderMethod;						/// Method to render the terrain
	std::list<TerrainPatch *>	m_activePatches;					/// List of active patches for rendering
	shared_ptr<VertexBuffer>	m_vb;								/// Vertex buffer used for rendering the terrain
	shared_ptr<IndexBuffer>		m_ib;								/// Index buffer used for rendering the terrain
	float						m_maximumScreenError;				/// This is the maximum allowable projected screen error for patch rendering

	unsigned int				m_requiredPatchVertices, m_requiredPatchIndices;	/// Calculated during OnPreRender(), this is the number of 
																					/// vertices and indices needed to render the terrain for this pass
};

KIMPLEMENT_STREAM( Terrain );
KIMPLEMENT_SCRIPT( Terrain );

}; // Katana

#endif // _TERRAIN_H