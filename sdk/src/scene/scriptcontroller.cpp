/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scriptcontroller.cpp
	Author:		Eric Bryant

	Per every game tick in the scene graph, a scripting function is called.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "visible.h"
#include "controller.h"
#include "scenecontext.h"
#include "scenegraph.h"
#include "scriptcontroller.h"
#include "base/kstring.h"

//
// RTTI declaration
//
KIMPLEMENT_RTTI( ScriptController, Controller );

//
// Externally defined structures
// 
extern shared_ptr<ScriptEngine> katana_script;


//
// OnUpdate
//
void ScriptController::OnUpdate( SceneContext * context )
{
	// If we have a valid scripting function, execute the function
	if ( !m_scriptFunction.empty() )
		katana_script->execFunction( m_scriptFunction.c_str(), 1, kstring(context->deltaTime).c_str() );
}
