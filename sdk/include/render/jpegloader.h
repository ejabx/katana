/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		jpegloader.h
	Author:		Eric Bryant

	Loads a JPEG file to/from disk.
*/

#ifndef _JPEGLOADER_H
#define _JPEGLOADER_H

namespace Katana
{

//
// Forward Declarations
//
class Texture;

///
/// JpegLoader
///	Loads a JPEG file to/from disk.
///
class JpegLoader
{
public:
	/// Loads a JPEG file from disk and returns the parameters
	static unsigned char * Load( const char * szFileName, unsigned int & width, unsigned int & height, 
								 unsigned int & dataSize, unsigned int & colorDepth, Texture::PixelFormat & pixel);
};

}; // Katana

#endif // _JPEGLOADER_H