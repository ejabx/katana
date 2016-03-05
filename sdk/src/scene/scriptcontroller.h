/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scriptcontroller.h
	Author:		Eric Bryant

	Per every game tick in the scene graph, a scripting function is called.
*/

#ifndef _SCRIPTCONTROLLER_H
#define _SCRIPTCONTROLLER_H

namespace Katana
{

//
// Forward Declaration
//
struct Message;

///
/// MouseController
///
class ScriptController : public Controller
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:
	/// Constructor
	ScriptController( const char * szFunct )						{ m_scriptFunction = szFunct; }

	/// Called by the scene graph every game update
	virtual void OnUpdate( SceneContext * context );

private:
	/// This is the function name to call during scene graph updates
	string		m_scriptFunction;
};

KIMPLEMENT_SCRIPT( ScriptController );

}; // Katana

#endif // _SCRIPTCONTROLLER_H