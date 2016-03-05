/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		systemfile.h
	Author:		Eric Bryant

	A System File
*/

#ifndef _SYSTEMFILE_H
#define _SYSTEMFILE_H

namespace Katana
{

///
/// FileType
///
enum FileType
{
	BINARY_FILE,
	TEXT_FILE,
};

///
/// OpenMode
///
enum OpenMode
{
	READ_ONLY,
	READ_WRITE,
	READ_WRITE_NEW,
};

///
/// SeekOrigin
enum SeekOrigin
{
	SEEK_BEGIN,
	SEEK_CURRENT,
	SEEK_EOF,
};

///
/// SystemFile
///
class SystemFile
{
public:
	/// Constructor creates or loads the specificed file
	SystemFile( const char * szFileName, OpenMode mode = READ_ONLY, FileType type = BINARY_FILE );

	/// Destructor automatically closes the file
	~SystemFile();

	/// Is this a valid file?
	bool isValid() const					{ return m_isOpen; }

	/// Return the read/write state
	OpenMode getReadWrite() const			{ return m_readWriteStatus; }
	
	/// Return the file type
	FileType getFileType() const			{ return m_fileType; }

	/// Opens an existing file
	bool openFile( const char * szFileName, OpenMode mode = READ_ONLY, FileType type = BINARY_FILE );

	/// Close the file
	bool closeFile();

	/// Reads a string from the file. By default, is will read all characters until
	/// the next NULL character.
	bool readString( string & kstr, int stringSize = -1 );

	/// Writes a string to the file
	bool writeString( const char * str, int stringSize );
	bool writeString( const string & kstr );

	/// Reads bytes from the file
	bool readBytes( void * buffer, int bufferSize );

	/// Writes bytes to the file
	bool writeBytes( void * buffer, int bufferSize );

	/// Seeks to the specified position
	bool seekTo( long offset, SeekOrigin origin );

private:

	// Is the file still opened
	bool			m_isOpen;

	// Read/Write Status of the file (set by the client)
	OpenMode		m_readWriteStatus;

	// Whether the file is binary or text (set internally)
	FileType		m_fileType;

	// Handle to the current file
	FileHandle		m_fileHandle;
};

}; // Katana

#endif // _SYSTEMFILE_H