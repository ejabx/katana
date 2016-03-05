/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		objectfactor.h
	Author:		Eric Bryant

	Scriptable agent that supports creation and loading of katana
	objects. It also binds the scriptable objects with the
	internal C++ representation.
*/

#ifndef _OBJECTFACTORY_H
#define _OBJECTFACTORY_H

namespace Katana
{

///
/// ObjectFactory
///
class ObjectFactory 
	: public Scriptable<ObjectFactory>, public RefCount
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;

public:
	KDECLARE_SCRIPT_GLOBAL( Create );
	KDECLARE_SCRIPT_GLOBAL( Load );
	KDECLARE_SCRIPT_GLOBAL( Save );
};

}; // Katana

#endif // _OBJECTFACTOR_H