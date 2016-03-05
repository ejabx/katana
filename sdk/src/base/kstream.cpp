/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kstream.cpp
	Author:		Eric Bryant

	Base class for all katana streams. Note the constructor
	is protected because clients can only use this class by
	deriving from it, or using the kistream or kostream classes.
*/


#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "version.h"
#include "kstream.h"
#include "kstring.h"
#include "system/systemfile.h"

//
// Constants
//
char kstream::KATANA_FILE_HEADER[]	= "Katana File Version";
char kstream::END_OF_FILE[]	 		= "EndOfFile";
char kstream::END_OF_BLOCK[]		= "EndOfBlock";

//
// isStream
//
bool kstream::isStream( const char * szFileName )
{
	// Construct a temporary file and load the header
	SystemFile temp( szFileName );

	kstring kstrCompare;
	kstring kstrHeader( KATANA_FILE_HEADER );
	kstrHeader.append( FILE_VERSION );
	
	temp.readString( kstrCompare, kstrHeader.size() );

	return ( kstrCompare == kstrHeader );
}

//
// beginSaveStream
//
bool kstream::beginSaveStream( const char * szFileName )
{
	// Wrong stream type
	if ( m_streamType == STREAM_INPUT ) return false;

	// Create a new streaming file
	m_spStreamFile.reset( new SystemFile( szFileName, READ_WRITE, BINARY_FILE ) );

	// Save the header information
	if ( !saveHeader() )
		return false;

	return true;
}

//
// beginLoadStream
// 
bool kstream::beginLoadStream( const char * szFileName )
{
	// Wrong stream type
	if ( m_streamType == STREAM_OUTPUT ) return false;

	// Load a streaming file
	m_spStreamFile.reset( new SystemFile( szFileName, READ_ONLY, BINARY_FILE  ) );

	// Load the header information
	if ( !loadHeader() )
		return false;

	return true;
}

//
// saveHeader
//
bool kstream::saveHeader()
{
	string kstrHeader( KATANA_FILE_HEADER );
	kstrHeader.append( FILE_VERSION );

	m_spStreamFile->writeString( kstrHeader );

	return true;
}

//
// loadHeader
//
bool kstream::loadHeader()
{
	string kstrCompare;
	string kstrHeader( KATANA_FILE_HEADER );
	kstrHeader.append( FILE_VERSION );
	
	m_spStreamFile->readString( kstrCompare );

	return ( kstrCompare == kstrHeader );
}