/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		streamable.cpp
	Author:		Eric Bryant

	Interface for classes which can store and retreive itself
	from katana streams.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "streamable.h"


//
// RTTI Definition
//
KIMPLEMENT_ROOT_RTTI( Streamable );

//
// Static Variables
//
std::map< kstring, Streamable::CreationMethod *> * Streamable::m_vcstrTable = 0;

//
// Local Variables
//
std::auto_ptr< std::map< kstring, Streamable::CreationMethod *> > g_creationTable;

//
// Construct
//
Streamable * Streamable::Construct( const char * objectType )
{
	// If possible find the creation method in the hash table
	std::map< kstring, CreationMethod * >::iterator iter = m_vcstrTable->find( objectType );

	// The method wasn't found
	if ( iter == m_vcstrTable->end() ) return false;

	// Otherwise, invoke the method and return the value
	CreationMethod * pf = (*iter).second;
	return (*pf)();
}

//
// Register
//
bool Streamable::Register( const char * objectType, CreationMethod * method )
{
	// Create the hash if necessary (because if we did it statically,
	// the order is not guaranteded, this function may be called before
	// it exists.
	if ( !m_vcstrTable ) 
	{
		m_vcstrTable = new std::map< kstring, Streamable::CreationMethod * >;

		// As a result, we use an auto_ptr for automatic deletion
		g_creationTable.reset( m_vcstrTable );
	}

	// Insert the hash
	m_vcstrTable->insert( std::pair< kstring, CreationMethod * >(objectType, method) );

	return true;
}