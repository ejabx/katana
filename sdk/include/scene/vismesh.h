/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		vismesh.h
	Author:		Eric Bryant

	A Visible Mesh, which contains geometry data (vertices, textures, indices, etc.)
	This class is responsible for aggregated the geometry, and packing the data
	into a vertex buffer for rendering.
*/

#ifndef _VISMESH_H
#define _VISMESH_H


namespace Katana
{

//
// Forward Declarations
//
struct Geometry;
class VertexBuffer;
class ShadowVolume;

///
/// VisMesh
/// A Visible Mesh, which contains geometry data (vertices, textures, indices, etc.)
///
class VisMesh : public Visible
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(VisMesh)
	KDECLARE_SCRIPT;

public:
	/// Constructor
	VisMesh();

	/// Copy constructor will share the geometry of another VisMesh
	VisMesh( shared_ptr<VisMesh> mesh);

	/// Shares existing geometry
	VisMesh( shared_ptr<Geometry> geometry);

	/// Destructor
	virtual ~VisMesh();

	/// Creates an internal instance of the geometry. This geometry
	/// is empty (it contains no vertices/indices), but you can fill
	/// via the returned Geometry pointer. This automatically dirties
	/// the mesh
	/// If the geometry was already created, it will return the existing
	/// instance.
	shared_ptr<Geometry> createGeometry();

	/// Retrieves the internal geometry. This function assumes you're
	/// retriving it for the purpose of changing the data, so it sets
	/// the dirty flag to true (unless readOnly is true)
	shared_ptr<Geometry> getGeometry(bool readOnly = false);

	/// Replace the internal instance of the geometry with another object
	void setGeometry( shared_ptr<Geometry> geometry );

protected:

	/// VisMesh will attempt to create the VB specific to the device when it
	/// is first attached to the scene.
	virtual bool OnAttach(SceneContext * context);

	/// VisMesh will release it's reference count to the VB, which may delete
	/// it if it's reference count is zero.
	virtual bool OnDetach(SceneContext * context);

	/// VisMesh will upload the mesh geometry to the VB during this event.
	virtual bool OnPreRender(SceneContext * context);

	/// VisMesh will render the VB during this event.
	virtual bool OnRender(SceneContext * context);

	/// VisMesh will optionally draw the normals and/or tangents
	virtual bool OnPostRender(SceneContext * context);

	/// VisMesh will draw the shadow volume computed for this mesh
	virtual bool OnRenderShadow( SceneContext * context );

private:

	/// Draw the normals for the mesh
	void Debug_DrawNormals(SceneContext * context);

	/// Draw the tangents for the mesh
	void Debug_DrawTangents(SceneContext * context);

	/// Draw the bounds for this visible object
	void Debug_DrawBounds( SceneContext * context );

	/// Draw the shadow volume for the mesh
	void Debug_DrawShadowVolume( SceneContext * context );

protected:
	/// Reference to the underlying geometry of the mesh. This is the serialized,
	/// offline version of the mesh, whose vertex data is shared with the vb.
	shared_ptr<Geometry>		m_geometry;

	/// Reference to the vertex buffer (which is sharable). Note it is
	/// the implementation specific (based on the renderer).
	shared_ptr<VertexBuffer>	m_vb;

	/// The shadow volume. This represents the renderable portion of the shadow.
	shared_ptr<ShadowVolume>	m_shadowVolume;

	/// Flag which indicates whether the mesh is dirty, that is,
	/// whether the mesh geometry must be uploaded to the VB again
	bool m_meshDirty;
};

KIMPLEMENT_STREAM( VisMesh );
KIMPLEMENT_SCRIPT( VisMesh );

}; // Katana

#endif // _VISMESH_H 