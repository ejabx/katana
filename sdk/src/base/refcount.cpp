/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		refcount.cpp
	Author:		Eric Bryant

	Base class for reference counting.
*/

#include "refcount.h"
using namespace Katana;

// Static Instance
unsigned int RefCount::m_objectCount = 0;



// Constructor
RefCount::RefCount() :
	m_refCount(0)
{
	m_objectCount++;
}




// DecRef
void RefCount::DecRef()
{
	if ( --m_refCount == 0 )
	{
		m_objectCount--;
		delete this;
	}
}