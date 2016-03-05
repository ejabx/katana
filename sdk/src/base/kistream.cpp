/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kistream.h
	Author:		Eric Bryant

	Katana input stream. Use for serializing objects from a file.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "rtti.h"
#include "streamable.h"
#include "kstream.h"
#include "kistream.h"
#include "kstring.h"
#include "system/systemfile.h"

//
// Constructor
//
kistream::kistream( const char * szFileName ) :
	kstream( STREAM_INPUT )
{
	beginLoadStream( szFileName );
}

//
// Destructor
//
kistream::~kistream()
{
	endStream();
}

//
// getObject
//
shared_ptr<Streamable> kistream::getObject( unsigned int index )
{
	// If load object's fail, this function automatically fails
	if ( !LoadObjects() ) return shared_ptr<Streamable>();

	// Does this index exist?
	if ( index >= m_loadobjects.size() ) return shared_ptr<Streamable>();

	return m_loadobjects[index];
}

shared_ptr<Streamable> kistream::getObject( const char * typeName )
{
	// If load object's fail, this function automatically fails
	if ( !LoadObjects() ) return shared_ptr<Streamable>();

	// Iterate through the collection of loaded objects
	for( vector< shared_ptr<Streamable> >::iterator iter = m_loadobjects.begin();
		 iter != m_loadobjects.end();
		 iter++ )
	{
		// Check if the rtti name is equal
		if ( stricmp( typeName, (*iter)->GetRTTI()->GetName() ) == 0 )
			return ( (*iter) );
	}

	return shared_ptr<Streamable>();
}

//
// LoadObjects
//
bool kistream::LoadObjects()
{
	// Was the objects already loaded?
	if ( m_loadobjects.size() ) return true;

	kstring objectType;

	// Load the RTTI Information from disk
	while ( LoadObjectRTTI( objectType ) )
	{
		// Create a new instance of this type via the virtual constructor
		shared_ptr<Streamable> object( Streamable::Construct( objectType.c_str() ) );

		// If we can't create the object, the entire file loading process is invalidated
		if ( !object ) return false;

		// Read of the reference from disk (using internally as an identifier)
		long refID = 0;
		operator>>(refID);

		// Store the real object pointer in the reference table
		m_referenceList.insert( std::pair<long, Streamable*>( refID, object.get()) );

		// Allow the object to load itself
		object->OnLoadStream( *this );

		// Insert the object in the loaded object list
		m_loadobjects.push_back( object );
	}

	// At this point, we may be in the next block, which contains
	// referenced objects only.

	// Check until we get to the end of file or another block
	while ( objectType != END_OF_FILE ||
		    objectType == END_OF_BLOCK )
	{
		// Load the object type
		if ( !LoadObjectRTTI( objectType ) )
			break;

		// Create a new instance of this type via the virtual constructor
		Streamable * object = Streamable::Construct( objectType.c_str() );

		// If we can't create the object, the entire file loading process is invalidated
		if ( !object ) return false;

		// Read of the reference from disk (using internally as an identifier)
		long refID = 0;
		operator>>(refID);

		// Allow the object to load itself
		object->OnLoadStream( *this );

		// Store the real object pointer in the reference table
		m_referenceList.insert( std::pair<long, Streamable*>( refID, object) );
	}

	// Now that we are at the end of the file, the final step is the 
	// reference lookup, that is, matching the pointers of looked up 
	// references with the actual real pointers
	for ( std::map<long, shared_ptr<Streamable>*>::iterator iter = m_lookupTable.begin();
		  iter != m_lookupTable.end();
		  iter++ )
	{
		// Grab the refernece id
		long refID = (*iter).first;

		// And check if we've already loaded this object
		std::map<long, Streamable *>::iterator found = m_referenceList.find( refID );
		if ( found != m_referenceList.end() )
		{
			// Grab the object
			Streamable * object = (*found).second;

			// Store the Point within the shared point
			shared_ptr<Streamable> * replaceRef = (*iter).second;
			replaceRef->reset( object );
		}
	}

	return true;
}

//
// LoadObjectRTTI
//
bool kistream::LoadObjectRTTI( string & objectType )
{
	// Clear the string
	objectType.clear();

	// Read the next string
	if ( false == m_spStreamFile->readString(objectType) )
		return false;

	// Check for the End-Of-Block
	if ( objectType == END_OF_BLOCK )
		return false;

	// Check for the End-Of-File
	if ( objectType == END_OF_FILE )
		return false;

	return true;
}

//
// endStream
//
bool kistream::endStream()
{
	m_spStreamFile.reset();

	return true;
}

//
// ReadBlob
//
void kistream::ReadBlob( void *& blob, int & blobSize )
{
	// Read the size of the blob from disk
	operator>>(blobSize);

	// Allocate space for the array
	blob = new char[blobSize * sizeof(char)];

	// Read the bytes into the array
	m_spStreamFile->readBytes( blob, blobSize );
}

//
// operator>> overloads
//
kistream& kistream::operator>> (char & rch)				{ m_spStreamFile->readBytes( &rch, sizeof(rch) ); return *this; }
kistream& kistream::operator>> (unsigned char & rch)	{ m_spStreamFile->readBytes( &rch, sizeof(rch) ); return *this; }
kistream& kistream::operator>> (int & n)				{ m_spStreamFile->readBytes( &n, sizeof(n) ); return *this; }
kistream& kistream::operator>> (unsigned int & un)		{ m_spStreamFile->readBytes( &un, sizeof(un) ); return *this; }
kistream& kistream::operator>> (short & s)				{ m_spStreamFile->readBytes( &s, sizeof(s) ); return *this; }
kistream& kistream::operator>> (unsigned short & us)	{ m_spStreamFile->readBytes( &us, sizeof(us) ); return *this; }
kistream& kistream::operator>> (long & l)				{ m_spStreamFile->readBytes( &l, sizeof(l) ); return *this; }
kistream& kistream::operator>> (unsigned long & ul)		{ m_spStreamFile->readBytes( &ul, sizeof(ul) ); return *this; }
kistream& kistream::operator>> (double & d)				{ m_spStreamFile->readBytes( &d, sizeof(d) ); return *this; }
kistream& kistream::operator>> (float & f)				{ m_spStreamFile->readBytes( &f, sizeof(f) ); return *this; }
kistream& kistream::operator>> (bool & b)				{ m_spStreamFile->readBytes( &b, sizeof(b) ); return *this; }
kistream& kistream::operator>> (ColorA & c)				{ m_spStreamFile->readBytes( &c, sizeof(c) ); return *this; }
kistream& kistream::operator>> (string & str)			{ m_spStreamFile->readString( str ); return *this; }
kistream& kistream::operator>> (Point2 & pt)			{ m_spStreamFile->readBytes( (void *)&pt.x, sizeof( Point2 ) ); return *this; }
kistream& kistream::operator>> (Point3 & pt)			{ m_spStreamFile->readBytes( (void *)&pt.x, sizeof( Point3 ) ); return *this; }
kistream& kistream::operator>> (Point4 & pt)			{ m_spStreamFile->readBytes( (void *)&pt.x, sizeof( Point4 ) ); return *this; }
kistream& kistream::operator>> (Matrix4 & m)			{ m_spStreamFile->readBytes( (void *)&m.m00, sizeof( m ) ); return *this; }
kistream& kistream::operator>> (Bound & b)				{ m_spStreamFile->readBytes( (void *)&b.getCenter(), sizeof( b ) ); return *this; }
kistream& kistream::operator>> (Quaternion & q)			{ m_spStreamFile->readBytes( (void *)&q.x , sizeof( q ) ); return *this; }

//
// Read a pointer reference
//
