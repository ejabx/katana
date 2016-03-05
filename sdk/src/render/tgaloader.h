/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		tgaloader.h
	Author:		Eric Bryant

	Loads a targa file to/from disk.
*/

#ifndef _TGALOADER_H
#define _TGALOADER_H

namespace Katana
{

///
/// TgaLoader
///	Loads a targa file to/from disk.
///
class TgaLoader
{
public:
	/// Loads a TGA file from disk and returns the parameters
	static unsigned char * Load( const char * szFileName, unsigned int & width, unsigned int & height, 
								 unsigned int & dataSize, unsigned int & colorDepth, Texture::PixelFormat & pixel);
};

}; // Katana

#endif // _TGALOADER_H