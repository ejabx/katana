/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kstream.h
	Author:		Eric Bryant

	Base class for all katana streams. Note the constructor
	is protected because clients can only use this class by
	deriving from it, or using the kistream or kostream classes.
*/

#ifndef _KSTREAM_H
#define _KSTREAM_H

namespace Katana
{

//
// Forward Declarations
//
class SystemFile;

///
/// StreamType
///
enum StreamType
{
	STREAM_INPUT,
	STREAM_OUTPUT,
};

///
/// kstream
///
class kstream 
{
protected:
	/// Constructor, the type is set by the client
	kstream( StreamType type ) : m_streamType( type )	{}

	/// Destructor
	virtual ~kstream()									{}

public:

	/// Checks whether the target is a katana stream
	static bool isStream( const char * szFileName );

protected:

	/// Starts the process of saving a katana stream by creating the file
	/// and saving the header to disk.
	bool beginSaveStream( const char * szFileName );

	/// Starts the process of loading a katana stream by opening the file and
	/// verifying the header.
	bool beginLoadStream( const char * szFileName );

	/// Pure virtual function which is called by the destructor automatically
	/// when the object goes out of scope. Call this function to explicitly
	/// cleanup the stream (if saving it will flush the buffer, if loading,
	/// it will presumably close the file)
	virtual bool endStream() = 0;

private:

	/// Internal function to save the header information to disk
	bool saveHeader();

	/// Internal function to load the header information from disk
	bool loadHeader();

protected:

	/// Internal file system
	shared_ptr<SystemFile> m_spStreamFile;

	/// Streaming type
	const StreamType	m_streamType;

	/// Byte sequence which determine the start of a katana file
	static char KATANA_FILE_HEADER[];

	/// Byte sequence which determine the end of block
	static char END_OF_FILE[];

	/// Byte sequence which determine the end of file
	static char END_OF_BLOCK[];
};


}; // Katana

#endif _KSTREAM_H