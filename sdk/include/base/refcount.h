/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		refcount.h
	Author:		Eric Bryant

	Base class for reference counting.
*/

#ifndef _REFCOUNT_H
#define _REFCOUNT_H

namespace Katana
{

/// RefCount
/// Derive from this class if you want to support reference counting.
/// This class is coupled with the auto pointer class AutoPtr.
class RefCount
{
public:
	/// Constructor
	RefCount();

	/// Destructor. It calls DecRef().
	virtual ~RefCount()					 { DecRef(); }

	/// Increments the reference count
	void AddRef()						 { m_refCount++; }

	/// Decrements the reference count
	void DecRef();

	/// Static method to retrieve the total number of game objects
	static unsigned int GetObjectCount() { return m_objectCount; }

protected:
	/// Reference count for this object
	unsigned int m_refCount;

	/// Global object count for all game objects
	static unsigned int m_objectCount;
};

/// AutoPtr
/// This is the auto pointer class which is coupled with the RefCount
/// class.
template <typename T>
class AutoPtr
{
public:
	/// Constructor
	AutoPtr();

	/// Constructor which takes a pointer
	AutoPtr(T * p);

	/// Copy Constructor
	AutoPtr(const AutoPtr & ap);

	/// Destructor
	~AutoPtr();

	/// Assignment of raw pointer
	AutoPtr & operator=(T * p);

	/// Assignment of auto pointer
	AutoPtr & operator=(const AutoPtr & ap);

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
	T * getPtr() const							{ return ptr; }

	/// Retrieve a reference to the internal pointer
	T ** getPtrRef()							{ return &ptr; }

	/// Retrieve a reference to the object
	T & getRef()								{ return *ptr; }
	
private:
	/// Pointer to the reference object
	T * ptr;
};

// Constructors
template <typename T>
AutoPtr<T>::AutoPtr() :
	ptr(0)
{
}

template <typename T>
AutoPtr<T>::AutoPtr(T * p) :
	ptr(p)
{
	if ( ptr )
		ptr->AddRef();
}

template <typename T>
AutoPtr<T>::AutoPtr(const AutoPtr<T> & ap) :
	ptr(ap.ptr)
{
	if ( ptr )
		ptr->AddRef();
}

// Destructor
template <typename T>
AutoPtr<T>::~AutoPtr()
{
	if ( ptr )
	{
		ptr->DecRef();
		ptr = 0;
	}
}

// Assignments
template <typename T>
AutoPtr<T> & AutoPtr<T>::operator=(T * p)
{
	if ( ptr == p )
	{
		return *this;
	}

	if ( ptr )
	{
		ptr->DecRef();
	}

	ptr = p;

	if ( ptr )
	{
		ptr->AddRef();
	}

	return *this;
}

template <typename T>
AutoPtr<T> & AutoPtr<T>::operator=(const AutoPtr<T> & ap)
{
	if ( ptr == ap.ptr )
	{
		return *this;
	}

	if ( ptr )
	{
		ptr->DecRef();
	}

	ptr = ap.ptr;

	if ( ptr )
	{
		ptr->AddRef();
	}

	return *this;
}

}; // Katana

#endif // _REFCOUNT_H