/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kistream.h
	Author:		Eric Bryant

	Katana input stream. Use for serializing objects from a file.
*/

#ifndef _KISTREAM_H
#define _KISTREAM_H

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
class karray;
class ColorA;

///
/// kistream
///
class kistream : private kstream
{
public:
	/// Constructor with stream filename
	kistream( const char * szFileName );

	/// Destructor
	~kistream();

	/// Retrieve the collection of streamable objects from disk. These are 
	/// all the objects which were inserted as root nodes in the stream.
	vector< shared_ptr<Streamable> > & getObjects()					{ LoadObjects(); return m_loadobjects; }

	/// Retrieves the nth object from the stream
	shared_ptr<Streamable> getObject( unsigned int index = 0 );

	/// Retrieves the first object in the stream that matches this type
	shared_ptr<Streamable> getObject( const char * typeName );

	/// Stops the streaming process. This will close the file and is called by the destructor
	virtual bool endStream();

public: 

	/// Operators (Streamable clients should only use these functions)
	kistream& operator>> (char & rch);
	kistream& operator>> (unsigned char & rch);
	kistream& operator>> (int & n);
	kistream& operator>> (unsigned int & un);
	kistream& operator>> (short & s);
	kistream& operator>> (unsigned short & us);
	kistream& operator>> (long & l);
	kistream& operator>> (unsigned long & ul);
	kistream& operator>> (double & d);
	kistream& operator>> (float & f);
	kistream& operator>> (bool & b);
	kistream& operator>> (string & str);
	kistream& operator>> (ColorA & c);
	kistream& operator>> (Point2 & pt);
	kistream& operator>> (Point3 & pt);
	kistream& operator>> (Point4 & pt);
	kistream& operator>> (Matrix4 & m);
	kistream& operator>> (Quaternion & q);
	kistream& operator>> (Bound & b);
	template <typename T> kistream& operator>> (shared_ptr<T> & sp);
	template <typename T> kistream& operator>> (shared_ptr< vector<T> > & sp);
	template <typename T> kistream& operator>> (vector<T> & a);
	template <typename T> kistream& operator>> (vector< shared_ptr<T> > & sp);

protected:

	/// Protected function which actually loaded the objects (root and children).
	/// It is performed lazily, that is, only when the client calls getObjects(),
	/// and the function is gready, it will load all objects, regardless of if
	/// you want (1) or (10).
	/// Also, calling this multiple times will NOT load the objects several time,
	/// but only once.
	bool LoadObjects();

	/// Function reads a blob to a file. This is a sequence of bytes
	/// stored on the disk and prefixed by the byte count for future reference.
	void ReadBlob( void *& blob, int & blobSize );

private:

	/// Only loads RTTI information from file. Returns false if at EOF
	bool LoadObjectRTTI( string & objectType );

protected:
	/// Collection of root objects from the stream
	vector< shared_ptr<Streamable> >	m_loadobjects;

private:
	/// Hashed mapping of pointers to objects. We'll use this list to keep
	/// track of which objects were saved so duplicated aren't saving
	/// while we save references
	std::map<long, shared_ptr<Streamable>*>		m_lookupTable;

	/// List of references which are stored during serialization of pointer.
	std::map<long, Streamable*>			m_referenceList;

	// Friendly functions
	friend void loadAsBlob( kistream & ostr, void *& blob, int & blobSize );
};




///
/// loadAsRefs
/// Template function which, assuming we have a collection of references
/// load them individually, instead of as a blob (which is the default
/// behavior).
///
template<typename T>
void loadAsRefs( kistream & istr, vector<T> & sp )
{
	// Load the number of elements
	long numElements = 0;
	istr >>(numElements);

	// Make room in the array
	sp.resize( numElements );

	// Load the references
	for( long index = 0; index < numElements; index++ )
	{
		istr >> ( sp[index] );
	}
}


///
/// loadAsBlob
/// Function which loads raw bytes from disk
///
inline void loadAsBlob( kistream & istr, void *& blob, int & blobSize )
{
	istr.ReadBlob(blob, blobSize);
}

//
// operator>>(shared_ptr)
//
template <typename T> 
kistream& kistream::operator>> (shared_ptr<T> & sp)
{
	// Load the Object Pointer as an identifier for Lookup Stage
	long refID = 0;
	operator>>(refID);

	// Insert it into the lookup table for later lookup
	if ( refID ) 
		m_lookupTable.insert( std::pair<long, shared_ptr<Streamable>*>( refID, reinterpret_cast<shared_ptr<Streamable> *>(&sp)) );

	return *this;
}

template <typename T>
kistream& kistream::operator>> (shared_ptr< vector<T> > & sp)
{
	// Read the blob data
	void * blob = 0; int blobSize = 0;
	ReadBlob( blob, blobSize );

	if ( blobSize )
	{
		// Create the new array
		sp.reset( new vector<T> );

		// Cast the void data into type data
		T * data = reinterpret_cast<T *>(blob);

		// Copy the raw array into the vector
		vector<T> temp;
		for( unsigned int i = 0; i < blobSize / sizeof(T); i++ ) temp.push_back( data[i] );

		// Assign the contents of the temp to the input vector
		sp->assign( temp.begin(), temp.end() );

		// Delete the temporary blob
		char * cblob = reinterpret_cast<char *>(blob);
		delete [] cblob;
	}

	return *this;
}

template <typename T> 
kistream& kistream::operator>> (vector<T> & a)
{
	// Read the blob data
	void * blob = 0; int blobSize = 0;
	ReadBlob( blob, blobSize );

	// Cast the void data into type data
	T * data = reinterpret_cast<T *>(blob);

	// Copy the raw array into the vector
	for( unsigned int i = 0; i < blobSize / sizeof(T); i++ ) a.push_back( data[i] );

	// Delete the temporary blob
	char * cblob = reinterpret_cast<char *>(blob);
	delete [] cblob;

	return *this;
}

}; // Katana

#endif // _KISTREAM_H