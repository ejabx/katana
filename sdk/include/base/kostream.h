/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kostream.h
	Author:		Eric Bryant

	Katana output stream. Use for serializing objects to a file.
*/

#ifndef _KOSTREAM_H
#define _KOSTREAM_H

namespace Katana
{

//
// Forward Declarations
//
class Streamable;
class Point2;
class Point3;
class Point4;
class Matrix4;
class Quaternion;
class Bound;
class kstring;
class ColorA;

///
/// kostream
///
class kostream : private kstream
{
public:
	/// Constructor with stream filename
	kostream( const char * szFileName );

	/// Destructor
	virtual ~kostream();

	/// Inserts an objects into the streaming archive
	void insertObject( shared_ptr<Streamable> object );

	/// Stops the streaming process. This will close the file, and flush
	/// the contents of the file buffer if saving. This function is called
	/// once the kostream goes out of scope
	virtual bool endStream();

	/// Function which is called by clients to determine if it is being saved
	/// as a root node, or if is being saved via a reference of another object.
	/// The significance is that Visible (or any object that has a parent) will
	/// not save the parent if it is the root, but a reference must save the
	/// parent to keep the relationship,
	bool isRoot( const Streamable & object ) const;

public: 

	/// Operators (Streamable clients should only use these functions)
	kostream& operator<< (char rch);
	kostream& operator<< (unsigned char rch);
	kostream& operator<< (int n);
	kostream& operator<< (unsigned int un);
	kostream& operator<< (short s);
	kostream& operator<< (unsigned short us);
	kostream& operator<< (long l);
	kostream& operator<< (unsigned long ul);
	kostream& operator<< (double d);
	kostream& operator<< (float f);
	kostream& operator<< (bool b);
	kostream& operator<< (const char * str);
	kostream& operator<< (const string & str);
	kostream& operator<< (const ColorA & c);
	kostream& operator<< (const Point2 & pt);
	kostream& operator<< (const Point3 & pt);
	kostream& operator<< (const Point4 & pt);
	kostream& operator<< (const Matrix4 & m);
	kostream& operator<< (const Quaternion & q);
	kostream& operator<< (const Bound & b);
	kostream& operator<< (const Streamable * ptr);
	template<typename T> kostream& operator<< (const shared_ptr<T> & sp);

	/// Saves an vector (or an shared_ptr containing an a vector) to the stream
	/// as a blob, which is simply a sequence of bytes.
	///
	/// IMPORTANT:	If you have a collection of pointers, such as vector<Visible *>,
	///				use the template saveAsRefs() below, otherwise only the pointers will be
	///				saves as pointers rather than references to exising objects
	///
	template<typename T> kostream& operator<< (const shared_ptr< vector<T> > & sp);
	template<typename T> kostream& operator<< (const vector<T> & a);

protected:

	/// Internal function to expand a list of streamable objects into
	/// a flat array which included their children
	static void ExpandChildren( Streamable * object, vector<Streamable *> & childrenList );

	/// Function writes a blob to a file. This is a sequence of bytes
	/// stored on the disk and prefixed by the byte count for future reference.
	void WriteBlob( void * blob, int blobSize );

protected:
	/// Collection of inserted objects to stream
	vector< shared_ptr<Streamable> >	m_saveobjects;

private:
	/// Hashed mapping of pointers to objects. We'll use this list to keep
	/// track of which objects were saved so duplicated aren't saving
	/// while we save references
	std::map<long, const Streamable**>	m_lookupTable;

	/// List of references which are stored during serialization of pointer.
	vector<const Streamable *>			m_referenceList;

	// Friendly functions
	friend void saveAsBlob( kostream & ostr, void * blob, int blobSize );
};


///
/// saveAsRefs
/// Template function which, assuming we have a collection of references
/// saves them individually, instead of as a blob (which is the default
/// behavior).
///
template<typename T>
void saveAsRefs( kostream & ostr, const vector< shared_ptr<T> > & a )
{
	// Store the number of elements
	ostr << (long)a.size();

	// Store the references
	for( vector< shared_ptr<T> >::const_iterator iter = a.begin();
		iter != a.end();
		iter++ )
	{
		ostr << (*iter);
	}
}

///
/// saveAsBlob
/// Function which saves raw bytes to disk
///
inline void saveAsBlob( kostream & ostr, void * blob, int blobSize )
{
	ostr.WriteBlob(blob, blobSize);
}

//
// operator<<(shared_ptr)
//
template<typename T> kostream& kostream::operator<< (const shared_ptr<T> & sp)
{
	operator<<( sp.get() );
	return *this;
}

//
// operator<<(shared_ptr(vector))
//
template<typename T> kostream& kostream::operator<< (const shared_ptr< vector<T> > & sp)
{
	if ( sp )
	{
		vector<T> & a = *sp.get();
		WriteBlob( &a[0], sizeof( T ) * a.size() );
	}
	else
	{
		WriteBlob( 0, 0 );
	}

	return *this; 
}

//
// operator<<(vector)
//
template<typename T> kostream& kostream::operator<< (const vector<T> & a)
{
	vector<T> & ca = const_cast<vector<T> &>(a);	// Strip of the constness
	WriteBlob( &ca[0], sizeof( T ) * a.size() );
	return *this;
}

}; // Katana

#endif // _KOSTREAM_H