/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		tgaloader.cpp
	Author:		Eric Bryant

	Loads a targa file to/from disk.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "texture.h"
#include "tgaloader.h"

//
// Definitions
//
#define TARGA_LOAD_RGB888 = 1
struct RGBTRIPLET
{
    char    rgbtBlue;
    char    rgbtGreen;
    char    rgbtRed;
};

//
// Prototypes for local functions
//
unsigned char * __GetGreyscaleData(FILE *strm, int width, int height);
unsigned char * __GetRGBData(FILE *strm, int width, int height);
unsigned char * __GetRGBAData(FILE *strm, int width, int height);
unsigned char * __GetCompressedRGBData(FILE *strm, int width, int height);
unsigned char * __GetCompressedRGBAData(FILE *strm, int width, int height);

//
// Load
//
unsigned char * TgaLoader::Load( const char * szFileName, unsigned int & width, unsigned int & height, 
								 unsigned int & dataSize, unsigned int & colorDepth, Texture::PixelFormat & pixel)
{
	// TODO: Use the abstract systemfile

	unsigned char * pData = 0;
	unsigned char type[4];
	unsigned char info[7];

	// Extract the FILE pointer from the resource handle
	FILE * fp = fopen( szFileName, "rb" );
	if ( !fp )
		return 0;

	// Read in colormap info and image type, byte 0 ignored
	fread(&type, sizeof(unsigned char), 3, fp);

	// Seek past the header and useless info
	fseek(fp, 12, SEEK_SET);
	fread(&info, sizeof(unsigned char), 6, fp);

	// Skip past image identification
	fseek(fp, type[0], SEEK_CUR);

	// Wrong file type ?
	if (type[1] != 0 || (type[2] != 2 && type[2] != 3 && type[2] != 10))
		return 0;

	// Setup texture information
	width = info[0] + info[1] * 256;
	height = info[2] + info[3] * 256;
	colorDepth = info[4];
	dataSize = width * height * (colorDepth == 32 ? 4 : colorDepth == 24 ? 3 : 1);
	unsigned char * data = 0;

	// Grab the image data from the file
	if ( colorDepth == 8 )
	{
		if ( type[2] != 10 )
			data = __GetGreyscaleData(fp, width, height);

		pixel = Texture::GREYSCALE;
	}
	if ( colorDepth == 24 )
	{	
		if ( type[2] != 10 )
			data = __GetRGBData(fp, width, height);
		else
			data = __GetCompressedRGBData(fp, width, height);

		pixel = Texture::BGR_EXT;
	}

	if ( colorDepth == 32 )
	{
		if ( type[2] != 10 )
			data = __GetRGBAData(fp, width, height);
		else
			data = __GetCompressedRGBAData(fp, width, height);

		pixel = Texture::BGR_EXT;
	}

	// Close File
	fclose(fp);

	return data;
}


//---------------------------------------------------

//
// __GetGreyscaleData
//
unsigned char * __GetGreyscaleData(FILE *strm, int width, int height)
{
	unsigned char *grey;
	size_t bread;
	int size = width * height;

	// Alocate space for image bytes
	grey = new unsigned char[size]; 

	if (grey == 0)
		return 0;

	// Read image bytes
	bread = fread (grey, sizeof(unsigned char), size, strm);

	// All bytes properly read ?
	if (bread != size * 1)
	{
		delete [] grey;
		return 0;
	}

	// Flip image
	unsigned int iLineSize = sizeof(unsigned char) * width;
	unsigned char* pLine1  = 0;
	unsigned char* pLine2  = 0;
	unsigned char* pTemp   = new unsigned char[width];

	for (int i=0; i<height / 2; i++)
	{
		// Set pointers to the lines that should be flipped
		pLine1 = grey + width * i;
		pLine2 = grey + width * (height - i - 1);

		// Copy Line1 into Temp
		memcpy(pTemp, pLine1, iLineSize);
		// Copy Line2 into Line1
		memcpy(pLine1, pLine2, iLineSize);
		// Copy Temp into into Line2
		memcpy(pLine2, pTemp, iLineSize);
	}

	// Kill temp line
	delete [] pTemp;
	pTemp;

	return grey;
}

//
// __GetRGBData
//
unsigned char * __GetRGBData(FILE *strm, int width, int height)
{
	unsigned char *rgb;
	size_t bread;
	int size = width * height;

	// Alocate space for image bytes
	rgb = new unsigned char[size * 3]; 

	if (rgb == 0)
		return 0;

	// Read image bytes
	bread = fread (rgb, sizeof(unsigned char), size * 3, strm);

	// All bytes properly read ?
	if (bread != size * 3)
	{
		delete [] rgb;
		return 0;
	}

	// Flip image

	unsigned int iLineSize = sizeof(unsigned char) * width * 3;
	unsigned char* pLine1  = 0;
	unsigned char* pLine2  = 0;
	unsigned char* pTemp   = new unsigned char[width * 3];

	for (int i=0; i<height / 2; i++)
	{
		// Set pointers to the lines that should be flipped
		pLine1 = rgb + width * 3 * i;
		pLine2 = rgb + width * 3 * (height - i - 1);

		// Copy Line1 into Temp
		memcpy(pTemp, pLine1, iLineSize);
		// Copy Line2 into Line1
		memcpy(pLine1, pLine2, iLineSize);
		// Copy Temp into into Line2
		memcpy(pLine2, pTemp, iLineSize);
	}

	// Kill temp line
	delete [] pTemp;
	pTemp;

	return rgb;
}

//
// __GetRGBAData
//
unsigned char * __GetRGBAData(FILE *strm, int width, int height)
{
	unsigned char *rgba;
	size_t bread;
	int size = width * height;

	// Alocate space for image bytes
	rgba = new unsigned char[size * 4]; 

	if (rgba == 0)
		return 0;

	// Read image bytes
	bread = fread (rgba, sizeof(unsigned char), size * 4, strm);

	// All bytes properly read ?
	if (bread != size * 4)
	{
		delete [] rgba;
		return 0;
	}

	// Flip image

	unsigned int iLineSize = sizeof(unsigned char) * width * 4;
	unsigned char* pLine1  = 0;
	unsigned char* pLine2  = 0;
	unsigned char* pTemp   = new unsigned char[width * 4];

	for (int i=0; i<height / 2; i++)
	{
		// Set pointers to the lines that should be flipped
		pLine1 = rgba + width * 4 * i;
		pLine2 = rgba + width * 4 * (height - i - 1);

		// Copy Line1 into Temp
		memcpy(pTemp, pLine1, iLineSize);
		// Copy Line2 into Line1
		memcpy(pLine1, pLine2, iLineSize);
		// Copy Temp into into Line2
		memcpy(pLine2, pTemp, iLineSize);
	}

	// Kill temp line
	delete [] pTemp;
	pTemp;

	return rgba;
}

//
// __GetCompressedRGBData
//
unsigned char * __GetCompressedRGBData(FILE *strm, int width, int height)
{
	unsigned char * bits;
//	int bread;
	int size = width * height;

	// Alocate space for image bytes
	bits = new unsigned char[size * 3]; 

	if (bits == 0)
		return 0;

	int x = 0;
	int y = 0;
	char rle;

	int iLineSize = sizeof(unsigned char) * width * 3;

	fread( bits, iLineSize, sizeof(unsigned char), strm);

//	if(fliphoriz)
//		bits = Internal_GetScanLine(freeimage, dib, header.is_height-y-1, flipvert);
//	else
//		bits = Internal_GetScanLine(freeimage, dib, y, flipvert);

	while (1) {

		fread(&rle, 1, 1, strm);
//		io->read_proc(&rle,1, 1, handle);
		
		if (rle>127) {

			rle -= 127;

			RGBTRIPLET triple;

			fread (&triple, sizeof(RGBTRIPLET), 1, strm);
//			io->read_proc(&triple, sizeof(RGBTRIPLE), 1, handle);

			for (int ix = 0; ix < rle; ix++) {
				bits[x++] = triple.rgbtBlue;
				bits[x++] = triple.rgbtGreen;
				bits[x++] = triple.rgbtRed;

				if (x >= iLineSize) {
					x = 0;
					y++;

					if (y >= height)
						goto done243;											

					fread( bits, iLineSize, sizeof(unsigned char), strm);
//					if(fliphoriz)
//						bits = Internal_GetScanLine(freeimage, dib, header.is_height-y-1, flipvert);
//					else
//						bits = Internal_GetScanLine(freeimage, dib, y, flipvert);
				}
			}
		} else {
			rle++;

			for (int ix = 0; ix < rle; ix++) {
				RGBTRIPLET triple;		

//				io->read_proc(&triple, sizeof(RGBTRIPLE), 1, handle);
				fread (&triple, sizeof(RGBTRIPLET), 1, strm);

				bits[x++] = triple.rgbtBlue;
				bits[x++] = triple.rgbtGreen;
				bits[x++] = triple.rgbtRed;
				
				if (x >= iLineSize) {
					x = 0;
					y++;

					if (y >= height)
						goto done243;										

					fread( bits, iLineSize, sizeof(unsigned char), strm);
//					if(fliphoriz)
//						bits = Internal_GetScanLine(freeimage, dib, header.is_height-y-1, flipvert);
//					else
//						bits = Internal_GetScanLine(freeimage, dib, y, flipvert);
				}
			}
		}
	}

done243:

	return bits;
}

//
// __GetCompressedRGBAData
//
unsigned char * __GetCompressedRGBAData(FILE *strm, int width, int height)
{
	return NULL; // NOT IMPLEMENTED
}