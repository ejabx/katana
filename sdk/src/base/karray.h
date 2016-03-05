/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		karray.h
	Author:		Eric Bryant

	Extends the stl vector class
*/

#ifndef _KARRAY_H
#define _KARRAY_H

namespace Katana
{

///
/// ksafearray
/// Provides a safe wrapper for standard c++ arrays. By passing in a pointer
/// to your array, you can access it using stl::iterators and operator[]. 
/// There is no ownership, the client is responsible for cleaning up the array.
///
/// Example:
///		char base[] = { 'a', 'b', 'c', 'd', 'e' };
///		ksafearray<char> array( base, sizeof(base) );
///		for ( ksafearray<char>::iterator iter = array.begin(); iter != array.end(); iter++ )
///		{ // do something useful }
///
template <typename T>
class ksafearray
{
public:
	/// STL type definitions
	typedef T              value_type;
	typedef T*             iterator;
	typedef const T*       const_iterator;
	typedef T&             reference;
	typedef const T&       const_reference;
	typedef std::size_t    size_type;
	typedef std::ptrdiff_t difference_type;

	/// Constructor
	ksafearray() : __base(0), __maxsize(0), __init(false) {}

	/// Constructor which takes a base array and number of array elements
	ksafearray( T * base, size_type size ) : __base(0), __maxsize(0), __init(false)  { set(base,size); }

	/// Copy Constructor with automatic type conversion
	template <typename T2>
	ksafearray( const ksafearray<T2> & array )
	{
		if ( array.__maxsize % sizeof(T) == 0  ) // Check whether the array elements fits evenly within this array
		{
			__base = (T*)array.__base;
			__maxsize = array.__maxsize / sizeof(T);
			__init = array.__init;
		}
	}

#if defined(_MSC_VER) && (_MSC_VER == 1300)
	// workaround for broken reverse_iterator in VC7
	typedef std::reverse_iterator<std::_Ptrit<value_type, difference_type, iterator, reference, iterator, reference> > reverse_iterator;
	typedef std::reverse_iterator<std::_Ptrit<value_type, difference_type, const_iterator, const_reference, iterator, reference> > const_reverse_iterator;
#else
	// workaround for broken reverse_iterator implementations
	typedef std::reverse_iterator<iterator,T> reverse_iterator;
	typedef std::reverse_iterator<const_iterator,T> const_reverse_iterator;
#endif

	/// Sets a pointer to the base array with the number of array elements
	void set( T * base, size_type size )			{ __base = base; __maxsize = size; __init = true; }

	/// Resets the array to point to nothing
	void reset()									{ __base = 0; __maxsize = 0; __init = false; }

	// STL iterator support
	iterator begin()								{ return __base; }
	const_iterator begin() const					{ return __base; }
	iterator end()									{ return __base + __maxsize; }
	const_iterator end() const						{ return __base + __maxsize; }

	/// STL reverse iterator support
	reverse_iterator rbegin()						{ return reverse_iterator(end()); }
	const_reverse_iterator rbegin() const			{ return const_reverse_iterator(end()); }
	reverse_iterator rend()							{ return reverse_iterator(begin()); }
	const_reverse_iterator rend() const				{ return const_reverse_iterator(begin()); }

	/// operator[]
	reference operator[](size_type i)				{ return __getsafereference( i ); }
	const_reference operator[](size_type i)	const	{ return __getsafereference( i ); }

	/// at() with range check
	reference at(size_type i)						{ return __getsafereference( i ); }
	const_reference at(size_type i) const			{ return __getsafereference( i ); }

	// front() and back()
	reference front()								{ return __base[0]; }
	const_reference front() const					{ return __base[0]; }
	reference back()								{ return __base[__maxsize-1]; }
	const_reference back() const					{ return __base[__maxsize-1]; }

	/// Size is based on maximum size set during set()
	size_type size()								{ return __maxsize; }
	bool empty()									{ return !__init; }
	size_type max_size()							{ return __maxsize; }

	/// Assign one value to all elements
	void assign (const T& value)					{ std::fill_n(begin(),size(),value); }

	/// Comparisons
	bool operator==( const ksafearray<T> & other ) { return ( ( __base == other.__base ) && ( __init == other.__init  ) && ( __maxsize == other.__maxsize ) ); }
	bool operator!=( const ksafearray<T> & other ) { return !(*this==other); }

private:
	/// Given an index into the reference base array, return a reference to an element,
	/// or a reference to a null value
	reference __getsafereference(size_type i) {  if ( !__init ) return __null; return ( i < __maxsize ? __base[i] : __null ); }

public:
	T *		__base;		/// Pointer to the base array
	size_t	__maxsize;	/// Maximum size of the array
	bool	__init;		/// Has this ksafearray been initialized via set()
	T		__null;		/// Null value
};

} // Katana

#endif _KARRAY_H