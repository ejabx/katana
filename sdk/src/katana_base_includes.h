/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		katana_core_includes.h
	Author:		Eric Bryant

	This contains the minumum included needed for modules build on top of the Katana Library.
	They include the miniumum Base, Math, Scripting, and System includes.
*/

#ifndef _KATANA_BASE_INCLUDES_H
#define _KATANA_BASE_INCLUDES_H

// Core Libraries
#include "base/rtti.h"
#include "base/color.h"
#include "base/karray.h"
#include "base/kstring.h"
#include "base/streamable.h"
#include "base/kstream.h"
#include "base/kistream.h"
#include "base/kostream.h"
#include "base/listener.h"
#include "base/message.h"
#include "base/log.h"
#include "base/messagerouter.h"

// Math Libraries
#include "math/kmath.h"
#include "math/point.h"
#include "math/plane.h"
#include "math/matrix.h"
#include "math/quaternion.h"
#include "math/bound.h"
#include "math/box.h"

// Scripting Libraries
#include "script/scriptengine.h"

// Physics Libraries
#include "physics/collidable.h"

#endif // _KATANA_BASE_INCLUDES_H