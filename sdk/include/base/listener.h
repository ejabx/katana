/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		listener.h
	Author:		Eric Bryant

	Defines the interface for message router listeners
*/

#ifndef _LISTENER_H
#define _LISTENER_H

namespace Katana
{

//
// Forward Declaration
//
struct Message;

///
/// Listener
/// Definition of listener interface
///
class Listener
{
public:
	/// This is the message handler. All listeners will have this method 
	/// called when a message is broadcasted from a listened message router.
	virtual bool OnMessage( Message * message )=0;
};

} // Katana

#endif // _LISTENER_H