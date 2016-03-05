/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		messagerouter.cpp
	Author:		Eric Bryant

	Support for Messaging Routing System.
*/

#include "katana_core_includes.h"
#include "messagerouter.h"
#include "listener.h"

//
// removeListener
//
bool MessageRouter::removeListener( Listener * listener )
{
	// Find the child and remove the reference count
	vector<Listener *>::iterator iter;

	for( iter = m_listeners.begin(); iter != m_listeners.end(); iter++ )
	{
		if ( listener == (*iter) )
		{
			// Erase it from the children list
			m_listeners.erase( iter );

			// It was found
			return true;
		}
	}

	return false;
}

//
// Broadcast
//
void MessageRouter::Broadcast( Message * msg )
{
	std::for_each(	m_listeners.begin(),
					m_listeners.end(),
					std::bind2nd( std::mem_fun( &Listener::OnMessage ), msg ) );
}