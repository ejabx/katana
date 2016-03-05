/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		messagerouter.h
	Author:		Eric Bryant

	Support for Messaging Routing System.
*/

#ifndef _MESSAGEROUTER_H
#define _MESSAGEROUTER_H

namespace Katana
{

//
// Forward Declarations
//
class Listener;
struct Message;

///
/// MessageRouter
/// This entity has the ability to aggregate listeners,
/// who can listen to the messages the message router sends.
/// Private inheritance is appropate for usage of this client.
///
class MessageRouter
{
public:
	/// Adds a listener to this router
	void addListener( Listener * listener )				{ m_listeners.push_back( listener ); }

	/// Removes a listener from this router
	bool removeListener( Listener * listener );

protected:
	
	/// Call to broadcast a message to the various listeners
	void Broadcast( Message * msg );

private:
	/// Vector of listeners who will receive messages from this router
	vector<Listener *> m_listeners;
};

}; // Katana

#endif // _MESSAGEROUTER_H