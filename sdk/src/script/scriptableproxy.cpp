/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scriptableproxy.cpp
	Author:		Eric Bryant

	Scriptable proxy object for the Streamable class
*/

#include <map>
#include <algorithm>
#include <functional>
#include <boost/function.hpp>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "../base/kbase.h"
#include "../base/rtti.h"
#include "../base/refcount.h"
#include "../base/karray.h"
#include "../base/streamable.h"
#include "../base/kstream.h"
#include "../base/kstring.h"
#include "../base/kistream.h"
#include "../base/kostream.h"
#include "../base/log.h"
#include "scriptable.h"
#include "luaargs.h"
#include "scriptableproxy.h"
#include "../system/systemfile.h"
using namespace Katana;

//
// RTTI Definition
// 
KIMPLEMENT_ROOT_RTTI( ScriptableProxy );
KIMPLEMENT_SCRIPT( ScriptableProxy );

//
// OnCreate
//
int ScriptableProxy::OnRegister( lua_State * lua )
{
	// Call base class's register to initialize the class with the scripting environent
	InitScriptObject( lua );

	return 0;
}

//
// OnDestroy
//
int ScriptableProxy::OnUnregister( lua_State * lua )
{
	// Call the base class's unregister to unlink the class with the scripting envrionment
	UnlinkScriptObject( lua );

	return 0;
}

// ----------------------------------------------------