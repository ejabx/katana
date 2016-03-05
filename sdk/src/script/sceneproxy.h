/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		sceneproxy.h
	Author:		Eric Bryant

	Scriptable proxy object for the Scene class
*/

#ifndef _SCENEPROXY_H
#define _SCENEPROXY_H

namespace Katana
{

///
/// SceneProxy
///
class SceneProxy 
	: public Scriptable<SceneProxy>, public RefCount
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:
	KDECLARE_SCRIPT_GLOBAL( AddNode );
	KDECLARE_SCRIPT_GLOBAL( RemoveNode );
	KDECLARE_SCRIPT_GLOBAL( AddController );
	KDECLARE_SCRIPT_GLOBAL( RemoveController );
};

}; // Katana

#endif // _SCENEPROXY_H