/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		gameengine.cpp
	Author:		Eric Bryant

	This is the Katana Game Engine.
	Clients can derive from this class an instantiate their own in order
	to receive events from the game engine.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "input/inputsystem.h"
#include "physics/physicssystem.h"
#include "render/rendertypes.h"
#include "render/render.h"
#include "scene/scenecontext.h"
#include "scene/scenegraph.h"
#include "scene/visible.h"
#include "scene/visnode.h"
#include "scene/camera.h"
#include "system/systemtimer.h"
#include "gameengine.h"
#include "application.h"
#include "textdisplay.h"

//
// Extern Variables
//
extern shared_ptr<Application> katana_app;

//
// Constructor
//
GameEngine::GameEngine() 
	: m_pause( false )
	, m_scaleDeltaTime( 1 )
	, m_fixedStepTime( 0 )
	, m_deltaTimeClamp( DELTA_TIME_CLAMP )
{
}

//
// Destructor
//
GameEngine::~GameEngine()
{
}

//
// OnStartup
//
bool GameEngine::OnStartup()
{
	// Reset the game timer
	m_gameTimer.StartZero();

	// Create a new scene graph (with a default root node)
	m_scene.reset( new SceneGraph( shared_ptr<VisNode>( new VisNode ) ) );

	// Get the window information
	WindowHandle handle;
	int x, y, cx, cy;
	bool full;
	katana_app->GetWindowInfo( handle, x, y, cx, cy, full );

	// Create a default camera
	m_currentCamera.reset( new Camera(kmath::PI/4, 0.1f, 32000.f, float(cx) / cy) );
	m_currentRenderer->SetViewport( *m_currentCamera.get() );

	// Setup the context for the scene graph
	m_scene->setContext( m_currentRenderer.get(), m_currentCamera.get(), m_debug.get(), m_textdisplay.get() );

	return true;
}

// 
// OnTick
//
bool GameEngine::OnTick()
{
	// Get the delta time from the timer
	float deltaTime = m_gameTimer.GetElapsedSeconds() * m_scaleDeltaTime;

	// Reset the game timer (so the next tick, we'll have delta time)
	m_gameTimer.StartZero();

	// Update input
	m_input->Poll();

	// Zero out the delta time (if pausing is enabled)
	if ( m_pause ) deltaTime = 0.f;

	// If we have a fixed step time, increment the delta by this amount
	if ( m_fixedStepTime ) deltaTime = m_fixedStepTime;

	// Clamp the delta time if it's above the threshold
	if ( deltaTime > m_deltaTimeClamp ) deltaTime = m_deltaTimeClamp;

	// Call update on all objects and gather the renderable objects
	m_scene->beginScene( m_currentRenderer.get(), m_currentCamera.get(), deltaTime );

	// Step the physics system forward by the delta time
	m_physics->integrate( deltaTime );

	// Actually render the objects
	m_scene->endScene();

	// Notify the text display system actually draw the text within its queue
	m_textdisplay->flushTextQueue( &m_scene->getContext() );

	// Zero out the fixed step time so during the next game tick we're execute normal time
	m_fixedStepTime = 0.f;

	return true;
}