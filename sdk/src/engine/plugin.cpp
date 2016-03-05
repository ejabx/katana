/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		plugin.cpp
	Author:		Eric Bryant

	Enables clients to register "plugins" into the game engine, which
	take part in the normal game update/render routine.
*/

#include "../base/rtti.h"
#include "../base/refcount.h"
#include "../base/kstring.h"
#include "plugin.h"
using namespace Katana;

//
// RTTI declaration
//
KIMPLEMENT_ROOT_RTTI( Plugin );