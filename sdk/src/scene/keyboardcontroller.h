/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		keyboardcontroller.h
	Author:		Eric Bryant

	Implements default keypresses within the game engine,
	such as the escape button, etc.
*/

#ifndef _KEYBOARDCONTROLLER_H
#define _KEYBOARDCONTROLLER_H

namespace Katana
{

//
// Forward Declarations
//
class WireframeState;

//
// Constants
//
const float STEP_FORWARD_TIME	= +0.003f;
const float STEP_BACKWARDS_TIME	= -0.003f;

///
/// KeyboardMappedFunction
///
typedef bool (*KeyboardMappedFunction)();

///
/// KeyboardController
///
class KeyboardController
	: public Controller, public Listener
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:
	/// Constructor
	KeyboardController();

	/// Setters for the keyboard mappings
	void setEscape( char key )						{ m_escapeChar = key; }
	void setWireframe( char key )					{ m_wireChar = key; }
	void setPause( char key )						{ m_pauseChar = key; }
	void setStepForward( char key )					{ m_stepFwdChar = key; }
	void setStepBack( char key )					{ m_stepBackChar = key; }
	void setShowNormals( char key )					{ m_showNormalsChar = key; }
	void setShowTangents( char key )				{ m_showTangentsChar = key; }
	void setShowLights( char key )					{ m_showLightsChar = key; }
	void setShowConsole( char key )					{ m_showConsoleChar = key; }
	void setShowBounds( char key )					{ m_showBoundsChar = key; }
	void setShowStatistics( char key )				{ m_showStatsChar = key; }
	void setEnableCull( char key )					{ m_enableCullChar = key; }
	void setShowShadow( char key )					{ m_showShadowChar = key; }

	/// Manage custom keyboard callbacks
	void addKeyCallback( char key, KeyboardMappedFunction funct );
	bool removeKeyCallback( char key );

	/// Called by the scene graph when it is first attached to the scene graph
	virtual bool OnAttach( SceneContext * context );

	/// Called by the scene graph when it is detached from the scene graph
	virtual bool OnDetach( SceneContext * context );

	/// Called when we receive input updates from the mouse device
	virtual bool OnMessage( Message * message );

protected:
	/// Wireframe state
	shared_ptr<WireframeState>		m_wireState;

	/// Keyboard mapping
	char m_escapeChar;		/// Keypress which exits the program (default is ESC under Win32)
	char m_wireChar;		/// Keypress which sets the scene to wireframe mode (default is 'W')
	char m_pauseChar;		/// Keypress which pauses the game (default is 'P')
	char m_stepFwdChar;		/// Keypress which steps the game forward (default is ']')
	char m_stepBackChar;	/// Keypress which steps the game backwards (default is '[')
	char m_showNormalsChar;	/// Keypress which displays all normals per mesh (default is 'N')
	char m_showTangentsChar;/// Keypress which displays all tangents per mesh (default is 'T')
	char m_showLightsChar;	/// Keypress which displays all lights (default is 'L')
	char m_showConsoleChar;	/// Keypress which displays the console (default is '`')
	char m_showBoundsChar;	/// Keypress which displays the bounds (default is 'B')
	char m_showStatsChar;	/// Keypress which displays the scene statistics (default is 'F')
	char m_enableCullChar;	/// Keypress which enabled frustum culling (default is 'C')
	char m_showShadowChar;	/// Keypress which displays all shadow volumes (default is 'S')

	/// Custom keyboard mappings between keys and their correspinding callbacks
	std::map<char,KeyboardMappedFunction> m_customCharMaps;
};

KIMPLEMENT_SCRIPT( KeyboardController );

}; // Katana

#endif // _KEYBOARDCONTROLLER_H