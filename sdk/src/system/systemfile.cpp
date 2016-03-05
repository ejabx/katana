/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		systemfile.cpp
	Author:		Eric Bryant

	A System File
*/

#include "katana_core_includes.h"
#include "systemfile.h"

//
// Constructor
//
SystemFile::SystemFile( const char * szFileName, OpenMode mode, FileType type ) :
	m_readWriteStatus( mode ),
	m_fileType( type ),
	m_isOpen( false )
{
	openFile( szFileName, mode, type );
}

//
// Desturctor
//
SystemFile::~SystemFile()
{
	closeFile();
}

//
// openFile
//
bool SystemFile::openFile( const char * szFileName, OpenMode mode, FileType type )
{
	// Decypher the file types
	const char * szPermissions = 0;

	if ( mode == READ_ONLY && type == TEXT_FILE )
		szPermissions = "rt";
	else if ( mode == READ_ONLY && type == BINARY_FILE )
		szPermissions = "rb";
	else if ( mode == READ_WRITE && type == TEXT_FILE )
		szPermissions = "wt";
	else if ( mode == READ_WRITE && type == BINARY_FILE )
		szPermissions = "wb";

	// Open the file
	m_fileHandle = (void *) fopen( szFileName, szPermissions );
	if ( !m_fileHandle )
		return false;

	m_isOpen = true;

	return true;
}

//
// closeFile
//
bool SystemFile::closeFile()
{
	if ( m_fileHandle )
		return ( fclose( (FILE *)m_fileHandle ) != 0 );
	else
		return false;
}

//
// readString
//
bool SystemFile::readString( string & kstr, int stringSize )
{
	if ( m_fileHandle )
	{
		// NOTE: We can't use fgets because it only checks for CRLF as
		//		 the end of a string. We have NULL terminated strings.
		FILE * fp = (FILE *)m_fileHandle;

		int pos = 1;
		char ch[2]; ch[1] = 0;

		// Read the string until we reach a NULL character
		while ( ( ( ch[0] = fgetc(fp) ) > 0 ) )
			kstr.append( ch  );

		return true;
	}

	return false;
}

//
// writeString
//
bool SystemFile::writeString( const char * str, int stringSize )
{
	return writeBytes( (void *)str, stringSize + 1 );
}

bool SystemFile::writeString( const string & kstr )
{
	return writeString( kstr.c_str(), kstr.size() );
}

//
// readBytes
//
bool SystemFile::readBytes( void * buffer, int bufferSize )
{
	if ( m_fileHandle )
	{
		FILE * fp = (FILE *)m_fileHandle;

		// Read bytes from the stream
		return ( fread( buffer, bufferSize, 1, fp ) > 0 );
	}

	return false;
}

//
// writeBytes
//
bool SystemFile::writeBytes( void * buffer, int bufferSize )
{
	if ( m_fileHandle )
	{
		FILE * fp = (FILE *)m_fileHandle;

		// Writes bytes to the stream
		return ( fwrite( buffer, bufferSize, 1, fp ) > 0 );
	}

	return false;
}

//
// seekTo
// Seeks to the specified position
//
bool SystemFile::seekTo( long offset, SeekOrigin origin )
{
	if ( m_fileHandle )
	{
		FILE * fp = (FILE *)m_fileHandle;

		// Seek to the position
		return ( fseek( fp, offset, origin ) == 0 );
	}
	return false;
}