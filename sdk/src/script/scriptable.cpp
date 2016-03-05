/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scriptable.cpp
	Author:		Eric Bryant

	Scripting interface. Any object which exposed itself to the scripting
	environment must have a proxy which supports this interface. It provdies
	the glue between the scripting engine and the C++ code.
*/

#include "scriptable.h"
using namespace Katana;
