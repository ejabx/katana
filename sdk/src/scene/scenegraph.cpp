/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scenegraph.h
	Author:		Eric Bryant

	Manager responsible for the scene graph functionality within
	the game engine. It aggregates all renderable (i.e., visible) objects
	for a given frame, sorts them based on texture, render state, and
	renders them to the current render device.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "engine/debugoutput.h"
#include "render/rendertypes.h"
#include "render/render.h"
#include "render/material.h"
#include "render/shader.h"
#include "render/hardwarelitshader.h"
#include "render/stencilshadowshader.h"
#include "visible.h"
#include "camera.h"
#include "visnode.h"
#include "controller.h"
#include "scenecontext.h"
#include "scenegraph.h"
#include "system/systemtimer.h"

//
// Constructor
//
SceneGraph::SceneGraph( shared_ptr<VisNode> root )
{
	// Store the root
	m_rootNode = root;

	// Seed the context
	m_context.currentViewMatrix = NULL;
	m_context.currentCamera = NULL;
	m_context.currentRenderer = NULL;
	m_context.currentScene = this;
	m_context.debugOutput = NULL;
	m_context.deltaTime = 0.f;
	m_context.gameTime = 0.f;
	m_context.renderPass = 0;
	m_context.frameCount = 0;

	// Zero out the scene statistics
	memset( &m_statistics, 0, sizeof( SceneStatistics ) );

	// Setup the default shaders
	m_defaultShader.reset( new HardwareLitShader() );
	m_stencilShadowShader.reset( new StencilShadowShader() );
}

//
// Destructor
//
SceneGraph::~SceneGraph()
{
	removeAllControllers();
}

//
// addNode
//
void SceneGraph::addNode( shared_ptr<Visible> node )
{ 
	// If the attachment was successful
	if ( node->OnAttach( &m_context ) ) 
	{
		// Add the node to the root node
		m_rootNode->attachChild( node ); 
	}
}

//
// removeNode
//
void SceneGraph::removeNode( shared_ptr<Visible> node )
{
	// Inform the node that is going to be detached
	node->OnDetach( &m_context ); 

	// Detach the node from the root node
	m_rootNode->detachChild( node ); 
}

//
// removeAllNodes
//
void SceneGraph::removeAllNodes()
{
	// Detach the root node, which recursively detaches
	// the children nodes
	m_rootNode->OnDetach( &m_context );

	// Detach all children from the root
	m_rootNode->detachAllChildren(); 
}

//
// addController
//
void SceneGraph::addController( shared_ptr<Controller> controller )
{
	// Can we attach the controller?
	if ( controller->OnAttach( &m_context ) )
	{
		/// Add it to our controller collection
		m_controllers.push_back( controller );
	}
}

//
// removeController
//
bool SceneGraph::removeController( shared_ptr<Controller> controller )
{
	// Find the controller and remove the reference count
	vector< shared_ptr<Controller> >::iterator iter;

	for( iter = m_controllers.begin(); iter != m_controllers.end(); iter++ )
	{
		if ( controller == (*iter) )
		{
			// Can we detach the controller?
			if ( controller->OnDetach( &m_context ) )
			{
				// Erase it from the controller list. ERASE-REMOVE Idiom (from Item 44 Effective STL, Scott Meyers)
				m_controllers.erase( 
					std::remove( m_controllers.begin(), m_controllers.end(), (*iter) ),
					m_controllers.end() );

				// It was found
				return true;
			}
		}
	}

	return false;
}

//
// removeAllControllers
//
void SceneGraph::removeAllControllers()
{
	// Iterate and release all the controller
	vector< shared_ptr<Controller> >::iterator iter;

	for( iter = m_controllers.begin(); iter != m_controllers.end(); iter++ )
	{
		// Detach the controller. Note this is not optional, the DecRef()
		// occurs regardless of the return value of this function. This is
		// only to notify the controller that it's being detached from the scene
		(*iter)->OnDetach( &m_context );
	}

	// Erase all controllers from the list
	m_controllers.clear();
}

int g_frames = 0;
int g_frames2 = 0;
SystemTimer							m_statsTimer;
float g_fTime = 0;

//
// beginScene
//
void SceneGraph::beginScene( Render * render, Camera * camera, float deltaTime )
{
	// Update the context
	m_context.currentCamera = camera;
	m_context.currentRenderer = render;
	m_context.deltaTime = deltaTime;
	m_context.gameTime += deltaTime;
	m_context.currentViewMatrix = &camera->getProjection();
	m_context.renderPass = 0;

	if ( m_statsTimer.GetElapsedSeconds() > 1 )
	{
		m_statistics.framesPerSecond = g_frames / m_statsTimer.GetElapsedSeconds();
		g_frames = 0;
		m_statsTimer.StartZero();
	}
	{
		// Start the stats timer to determine
		m_statsTimer.Start();
		g_frames++;
	}

	// Because the camera is a Visible object, it needs updating and rendering
	if ( camera )
	{
		camera->OnUpdate( &m_context );
		if ( camera->OnPreRender( &m_context ) )
			camera->OnRender( &m_context );
	}

	// Update ALL objects within the scene (regardless of whether they're visible).
	m_rootNode->OnUpdate( &m_context );

	// Iterate through all the controllers and update them
	for( vector< shared_ptr<Controller> >::iterator	iter = m_controllers.begin(); iter != m_controllers.end(); iter++ )
		(*iter)->OnUpdate( &m_context );

	// Recursively iterate over the nodes in our scene graph, adding them to the
	// render queue for rendering in endScene()

	// Clear the render queue to prepare it for render
	m_renderQueue.clear();
	m_shadowCasterQueue.clear();

	// Clear the set of active lights
	// TODO: Keep track of which lights effect specific nodes
	m_context.currentLights.clear();

	// "Flatten" the scene graph by iterating throught all the nodes, call OnPreRender()
	// to determine whether they want to be render (also updating their world matrices).
	// If so, adding them to the render queue
	RecursiveFillQueue( m_rootNode );

	// TODO: sort the render queue to minimize the render states changes
}

//
// endScene
//
void SceneGraph::endScene()
{
	// Iterate over all the objects in the render queue and render them
	for( vector< weak_ptr<Visible> >::iterator iter = m_renderQueue.begin();
		 iter != m_renderQueue.end();
		 iter++ )
	{
		shared_ptr<Shader> spShader;

		// Obtain a shared point to the visible object
		shared_ptr<Visible> spVisible = (*iter).lock();
		if ( !spVisible ) continue;

		// Grab the material from the visible object
		shared_ptr<Material> spMaterial = spVisible->getMaterial();

		// Determine the shader to use for this visible object from its material.
		// If the material does not have a shader, use the default shader
		if ( spMaterial && spMaterial->shader )	spShader = spMaterial->shader;
		else									spShader = m_defaultShader;

		// Setup the current visible object. The shader will need this to determine
		// the target of its operations
		m_context.currentVisibleObject = spVisible.get();
		
		// Allow the shader to perform its pre-processing
		if ( spShader->OnPreRender( &m_context ) )
		{
			// Render the visible object
			spVisible->OnRender( &m_context );

			// Allow the shader to do post-processing
			spShader->OnPostRender( &m_context );

			// Post-Render the visible object
			spVisible->OnPostRender( &m_context );
		}
	}

	// Renders the shadow casters
	renderStencilShadowCasters();

	// Stop the stats timer
	m_statsTimer.Stop();

	// Display debug output
	m_context.debugOutput->OnDebugOutput( &m_context );

	// The number of objects rendered this frame is the number of visible objects
	// in the render queue
	m_statistics.objectsRenderedLastFrame = (int)m_renderQueue.size();

	// Increment the total objects rendered
	m_statistics.totalObjectsRendered += m_statistics.objectsRenderedLastFrame;

	// Increment the frame count
	m_context.frameCount++;
}

//
// RecursiveFillQueue
//
void SceneGraph::RecursiveFillQueue( weak_ptr<VisNode> wpNode )
{
	if ( shared_ptr<VisNode> node = wpNode.lock() )
	{
		// Call OnPreRender() on this visible node
		if ( node->OnPreRender( &m_context ) )
		{
			// Add this node to the render queue
			AddNodeToQueue( node );

			// Now, iterate through its children, add them to the render queue
			// and add their iterate through its children's children (if they're
			// visible nodes).

			vector< shared_ptr<Visible> >::iterator iter;
			vector< shared_ptr<Visible> > & children = node->getChildren();

			for( iter = children.begin(); iter != children.end(); iter++ )
			{
				// Can we even render the object?
				if ( (*iter)->OnPreRender( &m_context ) )
				{
					shared_ptr<VisNode> visnode = boost::dynamic_pointer_cast<VisNode>( (*iter) );

					if ( visnode )
						// Next, if this is also a VisNode, recurse into its children
						RecursiveFillQueue( visnode );
					else
						// Otherwise, just add it to the queue for rendering
						AddNodeToQueue( (*iter) );
				}
			}
		}
	}
}

//
// AddNodeToQueue
// Adds a node to the render queue
//
void SceneGraph::AddNodeToQueue( weak_ptr<Visible> wpNode )
{
	if ( wpNode.lock() )
	{
		// Push this node onto the queue
		m_renderQueue.push_back( wpNode );

		// If this node is a shadow caster, then add it to the shadow caster queue
		if ( wpNode.lock()->getCastsShadows() )
			m_shadowCasterQueue.push_back( wpNode );

		// If this node has a light, push it onto the list of lights to render
		if ( wpNode.lock()->getLight() ) 
			m_context.currentLights.push_back( wpNode.lock()->getLight() );
	}
}

//
// renderStencilShadowCasters
// Renders the shadow casters using stencil shadow volumes
// 
void SceneGraph::renderStencilShadowCasters()
{
	// Do we have any lights, if not, exit
	if ( m_context.currentLights.size() == 0 ) return;

	// Do we have any shadow casters, if not, exit
	if ( m_shadowCasterQueue.size() == 0 ) return;

	// Reset the render pass
	m_context.renderPass = 0;

	// Allow the stencil shadow shader to setup the render states.
	// Because this may be a multiple pass shader, we iterate until it indicates the shader
	// is complete
	while ( !m_stencilShadowShader->OnPreRender( &m_context ) )
	{
		// Iterate over the shadow casters and tell them to render their shadow volumes
		for( vector< weak_ptr<Visible> >::iterator iter = m_shadowCasterQueue.begin();
			iter != m_shadowCasterQueue.end();
			iter++ )
		{
			// Obtain a shared point to the visible object
			shared_ptr<Visible> spVisible = (*iter).lock();
			if ( !spVisible ) continue;

			// Transform the shadow volume into world space
			m_context.currentRenderer->SetMatrix( MODELVIEW, STORE, spVisible->getWorldMatrix() );

			// Render the shadow volume
			spVisible->OnRenderShadow( &m_context );
		}
	}

	// Draw the actual shadow, which is all full screen quad render with stencil testing
	StencilShadowShader::renderShadows( &m_context );

	// Allow the stencil shadow shader to restore the render states
	m_stencilShadowShader->OnPostRender( &m_context );
}