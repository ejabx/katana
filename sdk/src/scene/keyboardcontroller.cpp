/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		keyboardcontroller.cpp
	Author:		Eric Bryant

	Implements default keypresses within the game engine,
	such as the escape button, etc.
*/

#include <bitset>

#include <windows.h>
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "visible.h"
#include "controller.h"
#include "scenecontext.h"
#include "scenegraph.h"
#include "keyboardcontroller.h"
#include "input/inputmessages.h"
#include "input/inputsystem.h"
#include "engine/application.h"
#include "render/renderstate.h"
#include "system/systemtimer.h"
#include "render/wireframestate.h"
#include "scene/visnode.h"
#include "engine/gameengine.h"
#include "engine/debugoutput.h"
#include "engine/console.h"

//
// RTTI declaration
//
KIMPLEMENT_ROOT_RTTI( Controller );
KIMPLEMENT_RTTI( KeyboardController, Controller );

//
// Externally defined structures
// 
extern shared_ptr<InputSystem>	katana_input;
extern shared_ptr<Application>	katana_app;
extern shared_ptr<SceneGraph>	katana_scene;
extern shared_ptr<GameEngine>	katana_game;
extern shared_ptr<DebugOutput>	katana_debug;
extern shared_ptr<Console>		katana_console;

//
// Constructor
//
KeyboardController::KeyboardController() 
	: m_escapeChar		( 0x01 )
	, m_wireChar		( 0x11 )
	, m_pauseChar		( 0x19 )
	, m_stepFwdChar		( 0x1b )
	, m_stepBackChar	( 0x1a )
	, m_showNormalsChar	( 0x31 )
	, m_showTangentsChar( 0x14 )
	, m_showLightsChar	( 0x26 )
	, m_showConsoleChar	( 0x29 )
	, m_showBoundsChar	( 0x30 )
	, m_showStatsChar	( 0x21 )
	, m_enableCullChar	( 0x2e )
	, m_showShadowChar	( 0x1f )
{
}

//
// addKeyCallback
//
void KeyboardController::addKeyCallback( char key, KeyboardMappedFunction funct )
{
	// Because the DInput returns scan keys, we need to convert our ASCII character
	// into a scan key. Because there is no direct way to translate a ASCII character
	// into a scan key, its a two step process, ASCII -> virtual key -> scan key
	short scanKey = MapVirtualKey( VkKeyScan( key ), 0 );

	// Insert the scan key into our mapping
	m_customCharMaps.insert( std::pair<char, KeyboardMappedFunction>(scanKey, funct) );
}

//
// removeKeyCallback
//
bool KeyboardController::removeKeyCallback( char key )
{
	// Because the DInput returns scan keys, we need to convert our ASCII character
	// into a scan key. Because there is no direct way to translate a ASCII character
	// into a scan key, its a two step process, ASCII -> virtual key -> scan key
	short scanKey = MapVirtualKey( VkKeyScan( key ), 0 );

	// Attempt to find the key within our mapping
	std::map<char,KeyboardMappedFunction>::const_iterator iter = m_customCharMaps.find( scanKey );

	// The mapping wasn't found
	if ( iter == m_customCharMaps.end() )
		return false;

	// Otherwise remove it
	m_customCharMaps.erase( key );

	return true;
}

//
// OnAttach
//
bool KeyboardController::OnAttach( SceneContext * context )
{
	// Register the controller as a listener for keyboard messages
	katana_input->addListener( this );

	// Create a wireframe state and attach it to the root node
	m_wireState.reset( new WireframeState );
	katana_scene->getRoot()->addState( m_wireState );

	return true;
}

//
// OnDetach
//
bool KeyboardController::OnDetach( SceneContext * context )
{
	// Unregister the controller as a listener for keyboard messages
	if ( katana_input )
		katana_input->removeListener( this );

	// Remove the wireframe state
	if ( katana_scene ) 
		katana_scene->getRoot()->removeState( m_wireState );

	return true;
}

//
// OnMessage
//
bool KeyboardController::OnMessage( Message * message )
{
	MESSAGE_SWITCH_BEGIN( message )

		MESSAGE_CASE( InputMessage_Keyboard, keyboard )
			
			// Check for escape character
			if ( keyboard->keys.at( m_escapeChar ) )
			{
				katana_app->Terminate();
			}
			
			// Check for wireframe mode
			if ( keyboard->keys.at( m_wireChar ) )
			{
				m_wireState->setWireframe( !m_wireState->getWireframe() );
			}

			// Check for pause
			if ( keyboard->keys.at( m_pauseChar ) )
			{
				katana_game->setPause( !katana_game->getPause() );
			}

			// Check for stepping the game forwards
			if ( keyboard->keys.at( m_stepFwdChar ) )
			{
				katana_game->stepTime( STEP_FORWARD_TIME );
				katana_game->setPause( true );
			}

			// Check for stepping the game backwards
			if ( keyboard->keys.at( m_stepBackChar ) )
			{
				katana_game->stepTime( STEP_BACKWARDS_TIME );
				katana_game->setPause( true );
			}

			// Check for time scaling (note the numeric keys are equal to the virtual keys + 1)
			if ( keyboard->keys.at( 2 ) )	{ katana_game->scaleTime( 0.1f ); katana_game->setPause( false ); } // 1
			if ( keyboard->keys.at( 3 ) )	{ katana_game->scaleTime( 0.2f ); katana_game->setPause( false ); } // 2
			if ( keyboard->keys.at( 4 ) )	{ katana_game->scaleTime( 0.3f ); katana_game->setPause( false ); } // 3
			if ( keyboard->keys.at( 5 ) )	{ katana_game->scaleTime( 0.4f ); katana_game->setPause( false ); } // 4
			if ( keyboard->keys.at( 6 ) )	{ katana_game->scaleTime( 0.5f ); katana_game->setPause( false ); } // 5
			if ( keyboard->keys.at( 7 ) )	{ katana_game->scaleTime( 0.6f ); katana_game->setPause( false ); } // 6
			if ( keyboard->keys.at( 8 ) )	{ katana_game->scaleTime( 0.7f ); katana_game->setPause( false ); } // 7
			if ( keyboard->keys.at( 9 ) )	{ katana_game->scaleTime( 0.8f ); katana_game->setPause( false ); } // 8
			if ( keyboard->keys.at( 10 ) )	{ katana_game->scaleTime( 0.9f ); katana_game->setPause( false ); } // 9
			if ( keyboard->keys.at( 11 ) )	{ katana_game->scaleTime( 1.0f ); katana_game->setPause( false ); } // 0

			// Check for displaying normals
			if ( keyboard->keys.at( m_showNormalsChar ) )
			{
				katana_debug->setDrawNormals( !katana_debug->getDrawNormals() );
			}

			// Check for displaying tangents
			if ( keyboard->keys.at( m_showTangentsChar ) )
			{
				katana_debug->setDrawTangents( !katana_debug->getDrawTangents() );
			}

			// Check for displaying lights
			if ( keyboard->keys.at( m_showLightsChar ) )
			{
				katana_debug->setDrawLights( !katana_debug->getDrawLights() );
			}

			// Check for displaying bounds
			if ( keyboard->keys.at( m_showBoundsChar ) )
			{
				katana_debug->setDrawBounds( !katana_debug->getDrawBounds() );
			}

			// Check for displaying statistics
			if ( keyboard->keys.at( m_showStatsChar ) )
			{
				katana_debug->setDrawStatistics( !katana_debug->getDrawStatistics() );
			}

			// Check for frustum culling
			if ( keyboard->keys.at( m_enableCullChar ) )
			{
				katana_debug->setEnableFrustumCulling( !katana_debug->getEnableFrustumCulling() );
			}

			// Check for displaying shadow volumes
			if ( keyboard->keys.at( m_showShadowChar ) )
			{
				katana_debug->setDrawShadowVolumes( !katana_debug->getDrawShadowVolumes() );
			}

			// Check for console
			if ( keyboard->keys.at( m_showConsoleChar ) )	
			{
				if ( katana_console->isVisible() )	katana_console->closeConsole();
				else								katana_console->openConsole();
			}

			// Check for custom key mappings
			for( std::map<char,KeyboardMappedFunction>::iterator iter = m_customCharMaps.begin();
				 iter != m_customCharMaps.end();
				 iter++ )
			{
				// If there exists a mapping for this key, execute the associated function
				if ( keyboard->keys.at( (*iter).first ) ) (*iter).second();
			}

		MESSAGE_CASE_END()

	MESSAGE_SWITCH_END()

	return true;
}