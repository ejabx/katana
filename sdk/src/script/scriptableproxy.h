/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scriptableproxy.h
	Author:		Eric Bryant

	Lightweight wrapper for all Scriptable objects. This proxy is what's
	stored within the script environment, and the concrete class is
	aggregated via Scriptable::m_ref, but because Scriptable is a template,
	we'll use this class to store objects.
*/

#ifndef _SCRIPTABLEPROXY_H
#define _SCRIPTABLEPROXY_H

namespace Katana
{

///
/// ScriptableProxy
///
class ScriptableProxy 
	: public Scriptable<ScriptableProxy, Streamable>
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;
};

}; // Katana

#endif // _SCRIPTABLEPROXY_H