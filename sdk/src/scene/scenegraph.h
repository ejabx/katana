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

#ifndef _SCENEGRAPH_H
#define _SCENEGRAPH_H

namespace Katana
{

//
// Forward Declaration
//
class Controller;
class Render;
class VisNode;
class Camera;
class DebugOutput;
class TextDisplay;
class Visible;
class Shader;

///
/// SceneGraph
/// Manager responsible for the scene graph functionality. It aggregates all
/// visible objects, and for a given frame, sorts them based on render state,
/// and then renders them.
///
class SceneGraph
{
	KDECLARE_SCRIPT;

public:
	/// Constructor
	SceneGraph( shared_ptr<VisNode> root);

	/// Destructor
	virtual ~SceneGraph();

	/// Adds a node for rendering to the scene graph
	void addNode( shared_ptr<Visible> node );

	/// Removes a node from the scene graph
	void removeNode( shared_ptr<Visible> node );

	/// Clears all nodes from the scene graph
	void removeAllNodes();

	/// Adds a controller to the scene graph.
	void addController( shared_ptr<Controller> controller );

	/// Removes a controller from the scene graph.
	bool removeController( shared_ptr<Controller> controller );

	/// Removes all controllers
	void removeAllControllers();

	/// Returns an array of all the controllers
	vector< shared_ptr<Controller> > & getControllers()			{ return m_controllers; }

	/// Sets the default shader. When a Visible object is being render and it doesn't have a material
	/// or its material does not have a shader, this is the shader that is used. By default, it is
	/// HardwareLitShader.
	void setDefaultShader( shared_ptr<Shader> shader )			{ m_defaultShader = shader; }

	/// Retrieves the default shader
	shared_ptr<Shader> getDefaultShader()						{ return m_defaultShader; }

	/// Sets the stencil shadow shader. This shader prepares the scene for rendering stencil
	/// shadow volumes. Replacing this shadow allows the client to use a different technique,
	/// like using vertex shaders or other optimizations.
	void setStencilShadowShader( shared_ptr<Shader> shader )	{ m_stencilShadowShader = shader; }

	/// Retrieves the root node
	shared_ptr<VisNode> getRoot()								{ return m_rootNode; }

public:

	/// Sets the context with its essential parameters
	void setContext( Render * render, Camera * camera, DebugOutput * debug, TextDisplay * textDisplay );

	/// Retrieves the context
	SceneContext & getContext()									{ return m_context; }

	/// Retrieve the statistics
	const SceneStatistics & getStatistics() const				{ return m_statistics; }

	/// Preprocess the scene by iterating through the scene graph nodes, update them,
	/// sorts them to minimize state changes, and pushes them in the render queue.
	void beginScene( Render * render, Camera * camera, float deltaTime );

	/// Flushes the render queue by calling render on all objects.
	void endScene();

private:

	/// Recursively adds the children of the root node into the render queue
	/// if their OnPreRender() returns true.
	void RecursiveFillQueue( weak_ptr<VisNode> wpNode );

	/// Adds a node to the render queue
	void AddNodeToQueue( weak_ptr<Visible> wpNode );

	/// Renders the shadow casters using stencil shadow volumes
	void renderStencilShadowCasters();

private:
	/// This is the context used to render all object. It stores state information
	/// like the current camera, texture pass, references to the renderer,
	/// and other information needed for rendering/updating
	SceneContext						m_context;

	/// This is the statistics object. It stores information like fps, tps, etc.
	SceneStatistics						m_statistics;

	/// The render queue, which is the flattened array of nodes to render.
	/// This queue is filled during BeginScene()
	vector< weak_ptr<Visible> >			m_renderQueue;

	// Collection of shadow casters accumated during the beginScene
	vector< weak_ptr<Visible> >			m_shadowCasterQueue;

	/// This is the root node of the scene graph. Renders starts at
	/// this node and works itself recursively down the scene graph.
	shared_ptr<VisNode>					m_rootNode;

	/// The collection of controllers which are applied to the scene every game update
	vector< shared_ptr<Controller> >	m_controllers;

	/// The default shader used for rendering when a Visible object does not specify
	/// its shader. By default, it is HardwareLitShader().
	shared_ptr<Shader>					m_defaultShader;

	/// The stencil shadow shader. This shader prepares the scene for rendering stencil
	/// shadow volumes. Replacing this shadow allows the client to use a different technique,
	/// like using vertex shaders or other optimizations.
	shared_ptr<Shader>					m_stencilShadowShader;
};

KIMPLEMENT_SCRIPT( SceneGraph );

//
// Inline
//

//
// SceneGraph::setContext
//
inline void SceneGraph::setContext( Render * render, Camera * camera, DebugOutput * debug, TextDisplay * textDisplay )
{ 
	m_context.currentRenderer = render; 
	m_context.currentCamera = camera; 
	m_context.debugOutput = debug; 
	m_context.textDisplay = textDisplay;
}

}; // Katana

#endif // _SCENEGRAPH_H