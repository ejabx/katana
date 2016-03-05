/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		message.h
	Author:		Eric Bryant

	Abstract interface for all messages
*/

#ifndef _MESSAGE_H
#define _MESSAGE_H

namespace Katana
{


///
/// Message
/// Base class for all messages
///
struct Message : public RTTI { KDECLARE_RTTI; };





///
/// Message Switch/Case Syntactic Macros
///
/// Start a message switch (_pMessage is a Message* derivative).
#define MESSAGE_SWITCH_BEGIN( _pMessage )													\
	{ Message* pMessage__ = _pMessage; if (0) {}

/// End a message switch.
#define MESSAGE_SWITCH_END()																\
	}																						\

/// Start a particular message case.  Verifies that the message is of type _messageType and
/// dynamically cast _varName to it.  _varName is valid only in the scope of the CASE block.
#define MESSAGE_CASE( _messageType, _varName )												\
		else if ( KIsDerivedFromClass<_messageType>(pMessage__) )							\
		{																					\
			_messageType* _varName = KDynamicCast<_messageType>(pMessage__);				\
			{

/// Ends a HZMESSAGE_CASE.  This is a mandatory closure.
#define MESSAGE_CASE_END()																	\
				return true;																\
			}																				\
		}

/// Make a default case.  This must be at the end of the list of cases within a SWITCH block.
#define MESSAGE_CASE_DEFAULT()																\
		else if (1) {	{																	

/// Discards a particular message type, preventing the default case, but not handling the message.
#define MESSAGE_CASE_DISCARD( _messageType )												\
		else if ( KIsDerivedFromClass<_messageType>(pMessage__) )							\
		{																					\
			return false;																	\
		}																					\

}; // Katana

#endif // _MESSAGE_H