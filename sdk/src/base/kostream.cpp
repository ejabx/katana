/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kostream.cpp
	Author:		Eric Bryant

	Katana output stream. Use for serializing objects to a file.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "streamable.h"
#include "kstream.h"
#include "kostream.h"
#include "scene/visible.h"
#include "scene/visnode.h"
#include "system/systemfile.h"

//
// Constructor
//
kostream::kostream( const char * szFileName ) :
	kstream( STREAM_OUTPUT )
{
	beginSaveStream( szFileName );
}

//
// Destructor
//
kostream::~kostream()
{
	endStream();
}

//
// insertObject
//
void kostream::insertObject( shared_ptr<Streamable> object )
{
	m_saveobjects.push_back( object );
}

//
// isRoot
//

bool kostream::isRoot( const Streamable & object ) const
{
	// Check whether the object is in the collection of initial save objects
	// If not, is is not a root node
	for( vector< shared_ptr<Streamable > >::const_iterator iter = m_saveobjects.begin();
		 iter != m_saveobjects.end();
		 iter++ )
	{
		if ( (*iter).get() == &object )
			return true;
	}

	return false;
}

//
// endStream
//
bool kostream::endStream()
{
	// Do we have any objects to save
	if ( m_saveobjects.empty() ) return false;

	// Assuming all the relevalent objects has been added to 
	// this stream, we'll begin the saving process:

	// We need a flat array of all the objects to save (considering
	// some of the objects may have children).
	vector<Streamable *> expandedChildrenList;
	vector<Streamable *>::iterator iter;
	vector< shared_ptr<Streamable> >::iterator siter;
	vector<const Streamable *>::iterator citer;

	for( siter = m_saveobjects.begin(); siter != m_saveobjects.end(); siter++ )
		ExpandChildren( (*siter).get(), expandedChildrenList );

	// Now that we have *all* the objects, process the saving by iterating
	// over all the objects, saving the object's rtti info and specific info
	for( iter = expandedChildrenList.begin(); iter != expandedChildrenList.end(); iter++ )
	{
		// For readability, this is the object to be streamed
		const Streamable * object = (*iter);

		// Store the RTTI Information to the File
		operator<<( object->GetRTTI()->GetName() );

		// Store the Object Pointer as an identifier for Lookup during Loading
		long refID = (long)object;
		operator<<(refID);

		// Add the Object to the Lookup List so we don't accidently add a duplicate via SaveReference
		m_lookupTable.insert( std::pair<long, const Streamable**>( refID, &object) );

		// Allow the Object to save itself
		object->OnSaveStream( *this );
	}

	// Write the End-Of-Block to disk. This signifified the the main objects
	// have been loaded, and the pointer reference remain
	operator<<( END_OF_BLOCK );

	// Now we'll process the reference list. This are the pointers that need to be dereferenced.
	size_t refSize = m_referenceList.size();
	for( citer = m_referenceList.begin(); citer != m_referenceList.end(); citer++ )
	{
		// For readability, this is the object to be streamed
		const Streamable * object = (*citer);
		if ( !object )
			continue;

		// This is the object's identifier
		long refID = (long)object;

		// Make sure we didn't already save the reference during the main save
		if ( m_lookupTable.end() == m_lookupTable.find( refID ) )
		{
			// Store the RTTI Information to the File
			operator<<( object->GetRTTI()->GetName() );

			// Store the Object Pointer as an identifier for Lookup during Loading
			long refID = (long)object;
			operator<<(refID);

			// Add the Object to the Lookup List so we don't accidently add a duplicate via SaveReference
			m_lookupTable.insert( std::pair<long, const Streamable**>( refID, &object) );

			// Allow the Object to save itself
			object->OnSaveStream( *this );

			// Due to the previous OnSaveStream(), we may have incremented the reference list
			// check if the reference list has grown and resync the iterator if necessary
			if ( m_referenceList.size() > refSize )
				citer = m_referenceList.begin();
		}
	}

	// Write the End-Of-File to disk.
	operator<<( END_OF_FILE );

	// Clear the save object collection (so that endStream won't resave the objects)
	m_saveobjects.clear();

	return true;
}

//
// ExpandChildren
//
void kostream::ExpandChildren( Streamable * object, vector<Streamable *> & childrenList )
{
	// Add this object to the collection
	childrenList.push_back( object );

	// Check if this object has children
	VisNode * node = KDynamicCast<VisNode>(object);

	// If so, iterate among the children
	if ( node )
	{
		vector< shared_ptr<Visible> > & children = node->getChildren();
		vector<Streamable *>::iterator iter;

//		for( iter = children.begin(); iter != children.end(); iter++ )
//			ExpandChildren( (*iter), childrenList );
	}
}

//
// WriteBlob
//
void kostream::WriteBlob( void * blob, int blobSize )
{
	// Write the byte count to disk
	m_spStreamFile->writeBytes( &blobSize, sizeof(int) );

	// Only write the bytes if the count > 0
	if ( blobSize > 0 )	m_spStreamFile->writeBytes( blob, blobSize );
}

//
// operator<< overloads
//
kostream& kostream::operator<< (char rch)				{ m_spStreamFile->writeBytes( &rch, sizeof(rch) ); return *this; }
kostream& kostream::operator<< (unsigned char rch)		{ m_spStreamFile->writeBytes( &rch, sizeof(rch) ); return *this; }
kostream& kostream::operator<< (int n)					{ m_spStreamFile->writeBytes( &n, sizeof(n) ); return *this; }
kostream& kostream::operator<< (unsigned int un)		{ m_spStreamFile->writeBytes( &un, sizeof(un) ); return *this; }
kostream& kostream::operator<< (short s)				{ m_spStreamFile->writeBytes( &s, sizeof(s) ); return *this; }
kostream& kostream::operator<< (unsigned short us)		{ m_spStreamFile->writeBytes( &us, sizeof(us) ); return *this; }
kostream& kostream::operator<< (long l)					{ m_spStreamFile->writeBytes( &l, sizeof(l) ); return *this; }
kostream& kostream::operator<< (unsigned long ul)		{ m_spStreamFile->writeBytes( &ul, sizeof(ul) ); return *this; }
kostream& kostream::operator<< (double d)				{ m_spStreamFile->writeBytes( &d, sizeof(d) ); return *this; }
kostream& kostream::operator<< (float f)				{ m_spStreamFile->writeBytes( &f, sizeof(f) ); return *this; }
kostream& kostream::operator<< (bool b)					{ m_spStreamFile->writeBytes( &b, sizeof(b) ); return *this; }
kostream& kostream::operator<< (const char * str)		{ m_spStreamFile->writeString( str, strlen( str ) ); return *this; }
kostream& kostream::operator<< (const ColorA & c)		{ m_spStreamFile->writeBytes( (void *)&c, sizeof(c) ); return *this; }
kostream& kostream::operator<< (const string & str)		{ m_spStreamFile->writeString( str.c_str(), str.size() ); return *this; }
kostream& kostream::operator<< (const Point2 & pt)		{ m_spStreamFile->writeBytes( (void *)&pt.x, sizeof( Point2 ) ); return *this; }
kostream& kostream::operator<< (const Point3 & pt)		{ m_spStreamFile->writeBytes( (void *)&pt.x, sizeof( Point3 ) ); return *this; }
kostream& kostream::operator<< (const Point4 & pt)		{ m_spStreamFile->writeBytes( (void *)&pt.x, sizeof( Point4 ) ); return *this; }
kostream& kostream::operator<< (const Matrix4 & m)		{ m_spStreamFile->writeBytes( (void *)&m.m00, sizeof( m ) ); return *this; }
kostream& kostream::operator<< (const Bound & b)		{ m_spStreamFile->writeBytes( (void *)&b.getCenter(), sizeof( b ) ); return *this; }
kostream& kostream::operator<< (const Quaternion & q)	{ m_spStreamFile->writeBytes( (void *)&q.x , sizeof( q ) ); return *this; }

//
// Save a pointer reference
//
kostream& kostream::operator<< (const Streamable * ptr)
{
	// Store the Object Pointer as an identifier for Lookup during Loading
	long refID = (long)ptr;
	operator<<(refID);

	// Push the object in the queue for saving later
	m_referenceList.push_back( ptr );
	return *this;
}