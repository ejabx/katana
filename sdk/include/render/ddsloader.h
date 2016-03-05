/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		ddsloader.h
	Author:		Eric Bryant

	Loads a DDS file to/from disk.
*/

#ifndef _DDSLOADER_H
#define _DDSLOADER_H

namespace Katana
{

///
/// DDSLoader
///	Loads a DDS file to/from disk.
///
class DDSLoader
{
public:
	/// Loads a TGA file from disk and returns the parameters
	static unsigned char * Load( const char * szFileName, unsigned int & width, unsigned int & height, unsigned int & mipmaps,
								 bool & cubmap, unsigned int & dataSize, unsigned int & colorDepth, Texture::PixelFormat & pixel);
};

}; // Katana

#endif // _DDSLOADER_H