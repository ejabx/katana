/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		actor.h
	Author:		Eric Bryant

	An Actor is any object which exists in the game engine that has two elements:
	a visible representation (such as a mesh, or biped), and a controller for
	the actor (which could come from AI, or a controller). The Controller can be
	any type of controller, the only onus is the it has to send the appropiate
	messages to the actor (such as movement, jump, etc.)
*/

#ifndef _ACTOR_H
#define _ACTOR_H

namespace Katana
{

//
// Forward Declarations
//
class Controller;

///
/// Actor
/// Represents a Visible Object which can be controlled explicitly by a controller.
///
class Actor
	: public Visible, public Listener
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(Actor)

private:
	/// Constructor
	Actor();

public:
	/// Constructor which takes a visible representation, and an actor controller
	Actor( Visible * visObject, Controller * actorController );

protected:

	/// Override the messages handler to route the messages to the aggregated classes
	virtual bool OnMessage( Message * message );
};

KIMPLEMENT_STREAM( Actor );

}; // Katana

#endif // _ACTOR_H