/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		comptr.h
	Author:		Eric Bryant

	Autopointer for COM classes.
*/

#ifndef _COMPTR_H
#define _COMPTR_H

namespace Katana
{

///
/// ComPtr
/// Autopointer for COM classes. It calls AddRef() and Release()
/// automatically during constructor and destructor
template <typename T>
class ComPtr
{
public:
	/// Constructor
	ComPtr();

	/// Copy Constructor
	ComPtr(const ComPtr & cp);

	/// Destructor
	~ComPtr();

	/// Assignment of raw pointer
	ComPtr & operator=(T * p);

	/// Assignment of auto pointer
	ComPtr & operator=(const ComPtr & ap);

	/// Defrerence of auto pointer
	operator T&() const							{ return *ptr; }
	T & operator*()								{ return *ptr; }

	/// Extract the pointer from the auto pointer
	T * operator->()							{ return ptr; }
	const T * operator->() const				{ return ptr; }

	/// Cast this auto pointer into a normal pointer
	operator T*() const							{ return ptr; }

	/// Is this auto pointer valid (is it not null)
	bool isValid() const						{ return ptr != 0; }

	/// Explicitly retrieve the internal pointer
	T * getPtr()								{ return ptr; }

private:
	/// Pointer to Referenced COM Object
	T * ptr;
};

// Constructors
template <typename T>
ComPtr<T>::ComPtr() :
	ptr(0)
{
}

template <typename T>
ComPtr<T>::ComPtr(const ComPtr<T> & ap) :
	ptr(ap.ptr)
{
	if ( ptr )
		ptr->AddRef();
}

// Destructor
template <typename T>
ComPtr<T>::~ComPtr() 
{
	if ( ptr )
	{
		ptr->Release();
		ptr = 0;
	}
}

// Assignments
template <typename T>
ComPtr<T> & ComPtr<T>::operator=(T * p)
{
	if ( ptr == p )
	{
		return *this;
	}

	if ( ptr )
	{
		ptr->Release();
	}

	ptr = p;

	if ( ptr )
	{
		ptr->AddRef();
	}

	return *this;
}

template <typename T>
ComPtr<T> & ComPtr<T>::operator=(const ComPtr<T> & ap)
{
	if ( ptr == ap.ptr )
	{
		return *this;
	}

	if ( ptr )
	{
		ptr->Release();
	}

	ptr = ap.ptr;

	if ( ptr )
	{
		ptr->AddRef();
	}

	return *this;
}

} // Katana

#endif // _COMPTR_H