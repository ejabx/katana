/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		visible.h
	Author:		Eric Bryant

	A Visible object in the Game Engine (which can be displayed by the
	scene manager). It contains visibility information (such as local space,
	textures, etc.)
*/

#ifndef _VISIBLE_H
#define _VISIBLE_H


namespace Katana
{

//
// Forward Declarations
//
class VisNode;
struct SceneContext;
class kistream;
class kostream;
struct Material;
class Light;
class Animation;

///
/// Visible
/// Represents a Visible object (which can have optional dynamic Physics properties,
/// via its Collidable class) in the Scene Graph.
///
class Visible 
	: public Collidable
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(Visible)
	KDECLARE_SCRIPT;

public:
	/// Constructor
	Visible();

	/// Constructor which takes a parent
	Visible( shared_ptr<VisNode> parent);

	/// Sets the parent
	void setParent( shared_ptr<VisNode> parent )		{ m_parent = parent; }

	/// Sets whether the object is visible
	void setVisible(bool bOn)							{ m_isVisible = bOn; }

	/// Sets whether the object is dirty (update of the world matrix is necessary)
	void setDirty()										{ m_isDirty = true; }

	/// Sets the translation
	void setTranslation(const Point3 & trans)			{ m_translation = trans; m_isDirty = true; }

	/// Sets the rotation
	void setRotation(const Quaternion & rot)			{ m_rotation = rot; m_isDirty = true; }

	/// Sets the transformation
	void setTransform(const Point3 & trans, const Quaternion & rot);

	/// Sets the scale
	void setScale(float scale)							{ m_scale = scale; m_isDirty = true; }

	/// Sets the local bounds
	void setBound(const Bound & bv)						{ m_localBound = bv; }

	/// Is this object visible?
	bool isVisible() const								{ return m_isVisible; }

	/// Is the bounds dirty (used to recalculate the world matrix)
	bool isDirty() const								{ return m_isDirty; }

	/// Returns the parent
	shared_ptr<VisNode> getParent()						{ return m_parent.lock(); }

	/// Returns the local translation
	const Point3& getTranslation() const				{ return m_translation; }

	/// Returns the local rotation
	const Quaternion & getRotation() const				{ return m_rotation; }

	/// Returns the local scale
	float getScale() const								{ return m_scale; }

	/// Returns the world matrix
	const Matrix4 & getWorldMatrix() const				{ return m_worldViewMatrix; }

	/// Returns the local bounds
	const Bound & getLocalBound() const					{ return m_localBound; }

	/// Returns the world bounds
	const Bound & getWorldBound() const					{ return m_worldBound; }

	/// Sets the material
	void setMaterial( shared_ptr<Material> material )	{ m_material = material; }

	/// Gets the material
	shared_ptr<Material> getMaterial()					{ return m_material; }

	/// Sets the light source
	void setLight( shared_ptr<Light> light )			{ m_light = light; }

	/// Gets the light source
	shared_ptr<Light> getLight()						{ return m_light; }

	/// Sets the animation. This is keyframed animation which is applied to this visible object every frame.
	void setAnimation( shared_ptr<Animation> animation) { m_animation = animation; }

	/// Sets whether this visible object is a shadow caster
	void setCastsShadows( bool enable )					{ m_isShadowCaster = enable; }

	/// Gets whether this visible object is a shadow caster
	bool getCastsShadows() const						{ return m_isShadowCaster; }

	/// Sets whether the visible object is billboarded
	void setBillboard( bool enable )					{ m_isBillboard = enable; }

	/// Gets whether the visible object is billboarded
	bool getBillboard() const							{ return m_isBillboard; }

public:
	/// This event is called when the visible object is initially attached to
	/// the scene. Use this event to process one-time initialization, like
	/// create vertex buffers.
	virtual bool OnAttach(SceneContext * context);

	/// This event is called once the object is detached from the scene. Use
	/// to release resources.
	virtual bool OnDetach(SceneContext * context)								{ return true; }

	/// This event is called when the visible object needs to update itself.
	/// Usually, this involves transforming the bounds and coordinates from local
	/// into world space. Note, this happens before the render.
	virtual bool OnUpdate(SceneContext * context);

	/// This event is called before rendering but after updating.
	/// Generally, OnPreRender determines whether the object is infact 
	/// within the frustum (for culling), and preforms other preprocessing.
	virtual bool OnPreRender(SceneContext * context);

	/// This event is called when the visible object needs to render itself
	virtual bool OnRender(SceneContext * context)								{ return true; }

	/// This event is called after rendering.
	/// Generally, OnPostRender is called after the visible object is rendered,
	/// it's usually used to display debug information such as normals, tangents, lights, etc.
	virtual bool OnPostRender(SceneContext * context);

	/// This event is called by the scene to render the visible object's shadow volume.
	/// During the shadow rendering pass, the scene will accumulate all shadow casters
	/// and call this method.
	/// The scene has already setup the appropate render states (stencil buffer, etc.),
	/// the only action this function needs to do is draw the shadow volume
	virtual bool OnRenderShadow( SceneContext * context )						{ return true; }

protected:
	/// Parential relationship
	weak_ptr<VisNode>		m_parent;

	/// Determine the visiblity of the object
	bool					m_isVisible;

	/// Flag used to determine whether the world matrix must be recalculated
	/// (usually due to translation or rotation of the object).
	bool					m_isDirty;

	/// Incremental frame count which is used so that the object isn't
	/// rendered multiple times. This number is seeded by the Scene Graph.
	int						m_frameCount;

	/// Current render pass. This is to support render pass render.
	/// The frame count will disallow rendering this object twice,
	/// unless it's a different render pass
	int						m_renderPass;

	/// Local Translation (is concatenated into the world matrix)
	Point4					m_translation;

	/// Local Rotation (is concatenated into the world matrix)
	Quaternion				m_rotation;

	/// Local Scale (is mutiplied into the world matrix)
	float					m_scale;

	/// Matrix which is calculated using local transform and parent relationships.
	/// It is a concatenation of this modified local matrix, multiplied by the view matrix
	/// (taken from the active camera)
	Matrix4					m_worldViewMatrix;

	/// Local Bounds of the Visible Object (used for culling).
	Bound					m_localBound;

	/// World Bounds of the Visible Object (used for culling). It was transformed
	/// by the Visible Object's world transformations.
	Bound					m_worldBound;

	/// Material which determines the material properties of this visible object
	shared_ptr<Material>	m_material;

	/// Light source associated with this Visible Object.
	shared_ptr<Light>		m_light;

	/// Keyframed animation which is applied to this visible object every frame.
	shared_ptr<Animation>	m_animation;

	/// Specifies whether this visible object is a shadow caster
	bool					m_isShadowCaster;

	/// Determines whether the visible object is billboarded each frame
	bool					m_isBillboard;
};

KIMPLEMENT_STREAM( Visible );
KIMPLEMENT_SCRIPT( Visible );

} // Katana

#endif // _VISIBLE_H