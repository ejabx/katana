/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		visnode.cpp
	Author:		Eric Bryant

	A Visible Node, which contains can contain children visible objects,
	or visible nodes.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "visible.h"
#include "visnode.h"
#include "camera.h"
#include "scenecontext.h"
#include "render/rendertypes.h"
#include "render/render.h"
#include "render/renderstate.h"
#include "render/blendstate.h"

//
// RTTI declaration
//
KIMPLEMENT_RTTI( VisNode, Visible );

//
// Constructor
//
VisNode::VisNode()
{
}

//
// Destructor
//
VisNode::~VisNode()
{
	detachAllChildren();
	removeAllStates();
}

//
// attachChild
//
bool VisNode::attachChild( shared_ptr<Visible> child )
{
	// Attaching myself as a child will cause an infinite loop, disallow
	if ( child.get() == this ) return false;

	// Setup the Hierarchy
	child->setParent( this->shared_from_this() );

	/// Add it to our node array
	m_children.push_back( child );
	
	return true;
}

//
// detachChild
//
bool VisNode::detachChild( shared_ptr<Visible> child )
{
	// Find the child and remove the reference count
	vector< shared_ptr<Visible> >::iterator iter;

	for( iter = m_children.begin(); iter != m_children.end(); iter++ )
	{
		if ( child == (*iter) )
		{
			// Destroy this object
			child.reset();

			// Erase it from the children list. ERASE-REMOVE Idiom (from Item 44 Effective STL, Scott Meyers)
			m_children.erase( 
				std::remove( m_children.begin(), m_children.end(), (*iter) ),
				m_children.end() );

			// It was found
			return true;
		}
	}

	return false;
}

//
// detachAllChildren
//
void VisNode::detachAllChildren()
{
	// Iterate and release all the children
	vector< shared_ptr<Visible> >::iterator iter;

	for( iter = m_children.begin(); iter != m_children.end(); iter++ )
	{
		// Destroy this object
		(*iter).reset();
	}

	// Erase all children from the list
	m_children.clear();
}

//
// addState
//
void VisNode::addState( shared_ptr<RenderState> state )
{
	// Add it to the vector of render states
	m_renderstates.push_back( state );
}

//
// removeState
//
bool VisNode::removeState( shared_ptr<RenderState> state )
{
	// Find the render state and remove the reference count
	vector< shared_ptr<RenderState> >::iterator iter;

	for( iter = m_renderstates.begin(); iter != m_renderstates.end(); iter++ )
	{
		if ( state == (*iter) )
		{
			// Destroy this state
			state.reset();

			// Erase it from the state list. ERASE-REMOVE Idiom (from Item 44 Effective STL, Scott Meyers)
			m_renderstates.erase( 
				std::remove( m_renderstates.begin(), m_renderstates.end(), (*iter) ),
				m_renderstates.end() );

			// It was found
			return true;
		}
	}

	return false;
}

//
// removeAllStates
//
void VisNode::removeAllStates()
{
	// Iterate and release all the states
	vector< shared_ptr<RenderState> >::iterator iter;

	for( iter = m_renderstates.begin(); iter != m_renderstates.end(); iter++ )
	{
		// Destroy this object
		(*iter).reset();
	}

	// Erase all states from the list
	m_renderstates.clear();
}

//
// OnAttach
//
bool VisNode::OnAttach(SceneContext * context)
{
	for( vector< shared_ptr<Visible> >::iterator iter = m_children.begin(); iter != m_children.end(); iter++ )
		(*iter)->OnAttach( context );

	return true;
}

//
// OnDetach
//
bool VisNode::OnDetach(SceneContext * context)
{
	for( vector< shared_ptr<Visible> >::iterator iter = m_children.begin(); iter != m_children.end(); iter++ )
		(*iter)->OnDetach( context );

	return true;
}

//
// OnUpdate
//
bool VisNode::OnUpdate(SceneContext * context)
{
	for( vector< shared_ptr<Visible> >::iterator iter = m_children.begin(); iter != m_children.end(); iter++ )
		(*iter)->OnUpdate( context );

	return true;
}

//
// OnRender
//
bool VisNode::OnRender(SceneContext * context)
{
	// Iterate over the render states and call render them
	for( vector< shared_ptr<RenderState> >::iterator iterrs = m_renderstates.begin(); iterrs != m_renderstates.end(); iterrs++ )
		if ( (*iterrs)->OnPreRender( context ) )
			context->currentRenderer->SetState( (*iterrs).get() );

	return true;
}

//
// OnPreRender
//
bool VisNode::OnPreRender(SceneContext * context)
{
	// Clear our local bounds first
	m_localBound = Bound();

	// Iterate through the children and expand our visible bounds based on the children
	for( vector< shared_ptr<Visible> >::iterator iter = m_children.begin(); iter != m_children.end(); iter++ )
		m_localBound.expand( (*iter)->getLocalBound() );

	// Call the base class to determine if this object is drawable
	return Visible::OnPreRender( context );
}

//
// OnPostRender
//
bool VisNode::OnPostRender(SceneContext * context)
{
	// Iterate over the render states and call post render so they can cleanup the
	// render state if necessary
	for( vector< shared_ptr<RenderState> >::iterator iterrs = m_renderstates.begin(); iterrs != m_renderstates.end(); iterrs++ )
		(*iterrs)->OnPostRender( context );

	// Call OnPostRender() on the Base Class
	return Visible::OnPostRender( context );
}

// -------------------------------------------------------


//
// OnLoadStream
//
bool VisNode::OnLoadStream( kistream & istr )
{
	// Call base class
	Visible::OnLoadStream( istr );

	// Load the children and the render states
	loadAsRefs( istr, m_children );
	loadAsRefs( istr, m_renderstates );

	return true;
}

//
// OnSaveStream
//
bool VisNode::OnSaveStream( kostream & ostr ) const
{
	// Call base class
	Visible::OnSaveStream( ostr );

	// Save the children and the render states
	saveAsRefs( ostr, m_children );
	saveAsRefs( ostr, m_renderstates );

	return true;
}