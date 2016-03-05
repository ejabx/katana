/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		visnode.h
	Author:		Eric Bryant

	A Visible Node, which contains can contain children visible objects,
	or visible nodes.
*/

#ifndef _VISNODE_H
#define _VISNODE_H

namespace Katana
{

//
// Forward Declarations
//
struct SceneContext;
class RenderState;

///
/// VisNode
/// Container for a hierarchy of children visible objects.
///
class VisNode 
	: public Visible
	, public enable_shared_from_this<VisNode>
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(VisNode);
	KDECLARE_SCRIPT;

public:
	/// Constructor
	VisNode();

	/// Destructor
	virtual ~VisNode();
	
	/// Adds a child to the visible node hierarchy and increments
	/// it's reference count.
	bool attachChild( shared_ptr<Visible> child );

	/// Removes a child from the visible node hierarchy. Note this function
	/// only decrements the reference count, so it may not be deleted.
	bool detachChild( shared_ptr<Visible> child );

	/// Removes all children from the visible node hierarchy
	void detachAllChildren();

	/// Retrieve the number of children in the hierarchy
	int getChildrenCount() const								{ return (int)m_children.size(); }

	/// Returns the constant vector of children
	vector< shared_ptr<Visible> > & getChildren() 				{ return m_children; }

	/// Adds a render state to this node and increments it's reference count
	void addState( shared_ptr<RenderState> state);

	/// Removes a render state from this node by decrementing it's refernece count
	bool removeState( shared_ptr<RenderState> state);

	/// Removes all render states
	void removeAllStates();

public:
	/// This event is specialized to iterate over the children and call the
	/// appropiate OnAttach event.
	virtual bool OnAttach(SceneContext * context);

	/// This event is specialized to iterate over the children and call the
	/// appropiate OnDetach event.
	virtual bool OnDetach(SceneContext * context);

	/// This event is specialized to iterate over the children and call the
	/// appropiate OnUpdate event.
	virtual bool OnUpdate(SceneContext * context);

	/// This event is called when the visible object needs to render itself.
	/// We use this event to setup the render states
	virtual bool OnRender(SceneContext * context);

	/// This event is called before rendering but after updating.
	/// We use this event to update the local bounds
	virtual bool OnPreRender(SceneContext * context);

	/// This event is called after rendering. We use this event to call
	/// PostRender on the render states
	virtual bool OnPostRender(SceneContext * context);

protected:
	/// Vector of children visible objects
	vector< shared_ptr<Visible> >		m_children;

	/// Vector of render states associated with this node
	vector< shared_ptr<RenderState> >	m_renderstates;
};

KIMPLEMENT_STREAM( VisNode );
KIMPLEMENT_SCRIPT( VisNode );

}; // Katana

#endif // _VISNODE_H