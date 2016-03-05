/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		katana_core_includes.h
	Author:		Eric Bryant

	This contains the core includes/definitions needed to compile files internal to the
	Katana Library.
*/

#ifndef _KATANA_CORE_INCLUDES_H
#define _KATANA_CORE_INCLUDES_H

// Include files for standard template library
#include <vector>
#include <map>
using namespace std;

// Include files for boost
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
using namespace boost;

// Turn off various warnings
#pragma warning( disable : 4244 )	// '=' : conversion from 'X' to 'Y', possible loss of data
#pragma warning( disable : 4311 )	// 'type cast' : pointer truncation from
#pragma warning( disable : 4312 )	// conversion from 'X' to 'Y' of greater size
#pragma warning( disable : 4267 )	// 'argument' : conversion from 'size_t' to 'int', possible loss of data
#pragma warning( disable : 4251 )	// 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
#pragma warning( disable : 4275 )	// non dll-interface class '_classA_' used as base for dll-interface class '_classB_'
#pragma warning( disable : 4355 )	// 'this' : used in base member initializer list

// Include files for Lua
// Lua headers
extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

// Common Typedefs
typedef void * AppHandle;
typedef void * WindowHandle;
typedef void * DialogHandle;
typedef void * FileHandle;

// Use the katana namespace by default
namespace Katana {};
using namespace Katana;

#endif // _KATANA_CORE_INCLUDES_H