/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		jpegloader.cpp
	Author:		Eric Bryant

	Loads a JPEG file to/from disk.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "texture.h"
#include "jpegloader.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	// This needs to be defined so the struct sizes match
	#define HAVE_BOOLEAN
	#define boolean unsigned char

	// Note this is needs "C" linkage.
	#include "jpeglib.h"

#ifdef __cplusplus
}
#endif // __cplusplus

// --------------------------------------------------------------------

// Make sure client includes Tokamak libaries
#ifdef _DEBUG
#pragma comment(lib, "libjpeg_d.lib")
#else
#pragma comment(lib, "libjpeg.lib")
#endif

// --------------------------------------------------------------------

//
// Load
//
unsigned char * JpegLoader::Load( const char * szFileName, unsigned int & width, unsigned int & height, 
								  unsigned int & dataSize, unsigned int & colorDepth, Texture::PixelFormat & pixel)
{
	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;

	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct jpeg_error_mgr jerr;

	/* More stuff */
	FILE * infile;		/* source file */

	/* In this example we want to open the input file before doing anything else,
	* so that the setjmp() error recovery below can assume the file is open.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to read binary files.
	*/

	if ((infile = fopen(szFileName, "rb")) == NULL) {
		return 0;
	}

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr);

	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);

	/* Step 2: specify data source (eg, a file) */
	jpeg_stdio_src(&cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */
	jpeg_read_header(&cinfo, TRUE);

	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.doc for more info.
	*/

	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	* jpeg_read_header(), so we do nothing here.
	*/

	/* Step 5: Start decompressor */
	jpeg_start_decompress(&cinfo);

	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* We may need to do some setup of our own at this point before reading
	* the data.  After jpeg_start_decompress() we have the correct scaled
	* output image dimensions available, as well as the output colormap
	* if we asked for color quantization.
	* In this example, we need to make an output work buffer of the right size.
	*/ 

	/* Make our output buffer/destination for the decompressed data */
	unsigned char * buffer;
    unsigned char * dest = buffer =
		new unsigned char[cinfo.output_width * cinfo.output_height * cinfo.num_components];

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could ask for
		* more than one scanline at a time if that's more convenient.
		*/
		jpeg_read_scanlines(&cinfo, &dest, 1);

		dest += cinfo.output_width * cinfo.num_components;
	}

	/* Step 7: Finish decompression */

	jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	/* After finish_decompress, we can close the input file.
	* Here we postpone it until after no more JPEG errors are possible,
	* so as to simplify the setjmp error logic above.  (Actually, I don't
	* think that jpeg_destroy can do an error exit, but why assume anything...)
	*/
	fclose(infile);

	// Setup texture information
	width = cinfo.output_width;
	height = cinfo.output_height;
	dataSize = width * height * cinfo.num_components;
	colorDepth = ( cinfo.num_components == 4 ) ? 32 : ( cinfo.num_components == 3 ) ? 24 : 8;

	// Setup the pixel format
	if ( cinfo.out_color_space == JCS_RGB )
		pixel =  Texture::RGB;
	else if ( cinfo.out_color_space == JCS_GRAYSCALE )
		pixel = Texture::GREYSCALE;
	else
		pixel = Texture::UNKNOWN;

	return buffer;
}