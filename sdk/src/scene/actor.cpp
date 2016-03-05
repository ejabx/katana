/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		actor.cpp
	Author:		Eric Bryant

	An Actor is any object which exists in the game engine that has two elements:
	a visible representation (such as a mesh, or biped), and a controller for
	the actor (which could come from AI, or a controller). The Controller can be
	any type of controller, the only onus is the it has to send the appropiate
	messages to the actor (such as movement, jump, etc.)
*/

#include <map>
#include "../base/kbase.h"
#include "../base/rtti.h"
#include "../base/refcount.h"
#include "../base/karray.h"
#include "../base/streamable.h"
#include "../base/kstream.h"
#include "../base/kistream.h"
#include "../base/kostream.h"
#include "../base/listener.h"
#include "../math/point.h"
#include "../math/matrix.h"
#include "../math/quaternion.h"
#include "../math/bound.h"
#include "../script/scriptengine.h"
#include "../render/geometry.h"
#include "../render/render.h"
#include "../render/vertexbuffer.h"
#include "../system/systemfile.h"
#include "visible.h"
#include "controller.h"
#include "actor.h"
using namespace Katana;

//
// RTTI Definition
//
KIMPLEMENT_RTTI( Actor, Visible );

//
// Constructor
//
Actor::Actor()
{
}

Actor::Actor( Visible * visObject, Controller * actorController )
{
}

//
// OnMessage
//
bool Actor::OnMessage( Message * message )
{
	return false;
}