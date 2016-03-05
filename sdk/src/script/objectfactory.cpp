/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		objectfactor.cpp
	Author:		Eric Bryant

	Scriptable agent that supports creation and loading of katana
	objects. It also binds the scriptable objects with the
	internal C++ representation.
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
#include "luaerr.h"
#include "objectfactory.h"
#include "scriptableproxy.h"
#include "../system/systemfile.h"
using namespace Katana;

//
// RTTI Definition
// 
KIMPLEMENT_ROOT_RTTI( ObjectFactory );
KIMPLEMENT_SCRIPT( ObjectFactory );

//
// OnCreate
//
int ObjectFactory::OnRegister( lua_State * _L )
{
	// Call base class's register to initialize the class with the scripting environent
	InitScriptObject( _L );

	// Add the global methods for creation and loading katana objecs.
	addGlobal( "Create", __Create );
	addGlobal( "Load", __Load );
	addGlobal( "Save", __Save );

	return 0;
}

//
// OnDestroy
//
int ObjectFactory::OnUnregister( lua_State * _L )
{
	// Call the base class's unregister to unlink the class with the scripting envrionment
	UnlinkScriptObject( _L );

	return 0;
}

// ----------------------------------------------------

//
// Create
//
KIMPLEMENT_SCRIPT_GLOBAL( ObjectFactory, Create )
{
	// Encapsulate the arguments
	LuaArgs args( lua );

	// We need at least one argument
	if ( args.getNumInArgs() == 0 ) return LUA_ERROR( "Need a object name as a parameter" );

	// Grab the object name to construct
	kstring objectName = args.getString();

	// Use the virtual constructor to create the object by name
	Streamable * object = Streamable::Construct( objectName );

	// The construction wasn't successful
	if ( !object ) return LuaErr( "ObjectFactory was unable to create object '%s'", objectName.c_str() );

	// Since the operation was valid, push the object onto the stack as a scriptable proxy
	args.putObject( object );

	// Return the number of arguments we've added to the stack
	return args.getNumOutArgs();
}

//
// Load
//
KIMPLEMENT_SCRIPT_GLOBAL( ObjectFactory, Load )
{
	// Encapsulate the arguments
	LuaArgs args( lua );

	// We need at least one argument
	if ( args.getNumInArgs() == 0 ) return LUA_ERROR( "Need a file name as a parameter" );

	// Grab the filename
	kstring fileName = args.getString();

	// Make sure we have a valid filename
	if ( fileName.empty() ) return LUA_ERROR( "The file name was invalid" );

	// Declare the stream with the filename
	kistream input( fileName.c_str() );

	// See if we had an object index as a parameter
	int objectIndex;
	if ( ( objectIndex = args.getInt() ) < 0 ) objectIndex = 0;

	// Load the object from the stream
	Streamable * object = input.getObject( objectIndex );

	// The loading wasn't successful
	if ( !object ) return LuaErr( "ObjectFactory to load object %s", fileName.c_str() );

	// Since the operation was valid, push the object onto the stack as a scriptable proxy
	args.putObject( object );

	// Return the number of arguments we've added to the stack
	return args.getNumOutArgs();
}

//
// Save
//
KIMPLEMENT_SCRIPT_GLOBAL( ObjectFactory, Save )
{
	// Encapsulate the arguments
	LuaArgs args( lua );

	// We need at least one argument
	if ( args.getNumInArgs() < 2 ) return LUA_ERROR( "Need a file name and object as a parameters" );

	// Grab the filename
	kstring fileName = args.getString();

	// Make sure we have a valid filename
	if ( fileName.empty() ) return LUA_ERROR( "The file name was invalid" );

	// Grab the target object
	Streamable * object = args.getObject();

	// Make sure we also have an object
	if ( !object ) return LUA_ERROR( "The object was invalid" );

	// Declare the stream with the filename
	kostream output( fileName.c_str() );

	// Add the object to the stream
	output.insertObject( *object );

	// Save the stream
	if ( !output.endStream() ) return LuaErr( "An error occured during ObjectFactor:Save(%s)", fileName.c_str() );

	return 0;
}