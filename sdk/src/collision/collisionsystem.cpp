/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		collisionsystem.cpp
	Author:		Eric Bryant

	Collision System handles collision detection in addition to
	rigid body movement.
*/

#include "../base/refcount.h"
#include "../base/karray.h"
#include "../base/messagerouter.h"
#include "collisionsystem.h"
using namespace Katana;