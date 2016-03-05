/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		gameengine.h
	Author:		Eric Bryant

	This is the Katana Game Engine.
	Clients can derive from this class an instantiate their own in order
	to receive events from the game engine.
*/

#ifndef _GAMEENGINE_H
#define _GAMEENGINE_H

namespace Katana
{

//
// Forward Declarations
//
class SceneGraph;
class GameContext;
class Render;
class Camera;
class InputSystem;
class DebugOutput;
class ScriptEngine;
class SystemTimer;
class PhysicsSystem;
class TextDisplay;

//
// Constants
//
const float DELTA_TIME_CLAMP = 0.3f;

///
/// GameEngine
/// 
class GameEngine
{
	KDECLARE_SCRIPT;

public:
	/// Constructor
	GameEngine();

	/// Destructor
	virtual ~GameEngine();

	/// Retrieves the scene graph manager
	shared_ptr<SceneGraph> getScene()				{ return m_scene; }

	/// Pauses the game engine by passing a 0 delta time to the scene
	void setPause( bool pause )						{ m_pause = pause; }

	/// Returns whether the game is paused or not
	bool getPause() const							{ return m_pause; }

	/// Scales the game time to game run slower or faster
	void scaleTime( float scale )					{ m_scaleDeltaTime = scale; }

	/// Returns the scaled time factor
	float getScaleTime() const						{ return m_scaleDeltaTime; }

	/// For a single game tick, will step the game simulation time by a fixed amount
	void stepTime( float step )						{ m_fixedStepTime = step; }

	/// Returns the current active camera
	shared_ptr<Camera> getCurentCamera()			{ return m_currentCamera; }

	/// Returns the current renderer
	shared_ptr<Render> getCurrentRender()			{ return m_currentRenderer; }

public:

	/// Event is called when the game engine is initially started
	virtual bool OnStartup();

	/// Event is called when the game needs to update itself
	virtual bool OnTick();

public:
	/// Scene Graph Manager
	shared_ptr<SceneGraph>		m_scene;

	/// The active renderer
	shared_ptr<Render>			m_currentRenderer;

	/// The active camera
	shared_ptr<Camera>			m_currentCamera;

	/// The scripting environment
	shared_ptr<ScriptEngine>	m_scripting;

	/// The input system
	shared_ptr<InputSystem>		m_input;

	/// The debug output device
	shared_ptr<DebugOutput>		m_debug;

	/// The physics system
	shared_ptr<PhysicsSystem>	m_physics;

	/// The text display system
	shared_ptr<TextDisplay>		m_textdisplay;

	/// High resolution game timer which keeps track of game time
	SystemTimer					m_gameTimer;

	/// Flags whether the game engine is paused
	bool						m_pause;

	/// Factor to scale time
	float						m_scaleDeltaTime;

	/// Fixed step. The game time will be incremented by this amount for a single game tick.
	float						m_fixedStepTime;

	/// Clamp time. When the delta time is above this time, it will be clamped to this time
	float						m_deltaTimeClamp;
};

KIMPLEMENT_SCRIPT( GameEngine );

}

#endif // _GAMEENGINE_H