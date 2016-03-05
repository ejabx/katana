/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		ddsloader.cpp
	Author:		Eric Bryant
	Credits:	The DDS code was taken from NVIDIA's Image_DXTC.cpp Sample.

	Loads a DDS file to/from disk. 
	TODO: We only load the first MipMap


*/

#define _X86_
#include <stdarg.h>
#include <windef.h>
#include <winbase.h>
#include <d3d8types.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "texture.h"
#include "ddsloader.h"
#include "dds.h"

//
// Local Variables
//
enum PixelFormat
{
	PF_DXT1,
	PF_DXT2,
	PF_DXT3,
	PF_DXT4,
	PF_DXT5,
	PF_ARGB,
	PF_UNKNOWN
};

//
// Local Functions
//
PixelFormat	__GetPixelFormat( DDS_HEADER & ddsHeader );
bool		__DecompressDXT1( FILE *fp, DDS_HEADER & ddsHeader, unsigned char * comp, unsigned char *& data);
bool		__DecompressDXT3( FILE *fp, DDS_HEADER & ddsHeader, unsigned char * comp, unsigned char *& data);
bool		__DecompressDXT5( FILE *fp, DDS_HEADER & ddsHeader, unsigned char * comp, unsigned char *& data);
bool		__LoadAllMipSurfaces(FILE * fp, D3DCUBEMAP_FACES FaceType, DDS_HEADER & ddsHeader, unsigned char *& data);
bool		__LoadAllVolumeSurfaces(FILE * fp, DDS_HEADER & ddsHeader, unsigned char *& data);



//
// Load
//
unsigned char * DDSLoader::Load( const char * szFileName, unsigned int & width, unsigned int & height, unsigned int & mipmaps,
								 bool & cubmap, unsigned int & dataSize, unsigned int & colorDepth, Texture::PixelFormat & pixel)
{
	// TODO: Use the abstract systemfile

	// Extract the FILE pointer from the resource handle
	FILE * fp = fopen( szFileName, "rb" );
	if ( !fp )
		return 0;

	// Grab the Magic Number from the file
	DWORD dwMagic;
	fseek(fp, 0, SEEK_SET);
	fread(&dwMagic, 1, sizeof(DWORD), fp);

	// Is it valid?
	if ( dwMagic != MAKEFOURCC('D','D','S',' '))
		return false;

	// If so, grab the header file next
	DDS_HEADER ddsHeader;
	fread(&ddsHeader, 1, sizeof(DDS_HEADER), fp);

	// Fill in the header parameters
	width = ddsHeader.dwWidth;
	height = ddsHeader.dwHeight;
	colorDepth = 32;				// DDS textures are 32 bits
	pixel = Texture::RGBA;			// ... and RGBA format
	mipmaps = 1;

	// Check if we have a cube map
    int cubeFlags = (ddsHeader.dwCubemapFlags & DDS_CUBEMAP_ALLFACES);

	// Check if we have a volumetric texture
	int depth;
	if ( ddsHeader.dwHeaderFlags & DDS_HEADER_FLAGS_VOLUME )
		depth = ddsHeader.dwDepth;
	else
		depth = 0;

	// Grab the pixel format of the texture
	PixelFormat format = __GetPixelFormat( ddsHeader );

	// Raw data buffer
	unsigned char * data = 0;
	unsigned char * comp = 0;
	unsigned char * orig_data;

	// Load the texture
	if ( depth > 0 )
	{
		// Load the volumetric textures
		if ( !__LoadAllVolumeSurfaces( fp, ddsHeader, data ) )
			goto clean_up;
	}
	else if ( cubeFlags )
	{
        // Cubemaps created with the DX7 version of DxTex may skip some
        // cube faces.  ddsh.dwCubeMapFlags indicates which faces are
        // present.  If you only care about loading post-DX7 cubemaps, you
        // don't have to check these flags -- just load each face in sequence.
		dataSize = width * height * 6 * (ddsHeader.ddspf.dwRGBBitCount >> 3) * sizeof(unsigned char);
		data = new unsigned char[dataSize];
		orig_data = data;

        // REVIEW: zero out surfaces of missing faces?
        if (cubeFlags & DDS_CUBEMAP_POSITIVEX)
        {
			if ( !__LoadAllMipSurfaces( fp, D3DCUBEMAP_FACE_POSITIVE_X, ddsHeader, data ) )
				goto clean_up;
        }

        if (cubeFlags & DDS_CUBEMAP_NEGATIVEX)
        {
			if ( !__LoadAllMipSurfaces( fp, D3DCUBEMAP_FACE_NEGATIVE_X, ddsHeader, data ) )
				goto clean_up;
        }

        if (cubeFlags & DDS_CUBEMAP_POSITIVEY)
        {
			if ( !__LoadAllMipSurfaces( fp, D3DCUBEMAP_FACE_POSITIVE_Y, ddsHeader, data ) )
				goto clean_up;
        }

        if (cubeFlags & DDS_CUBEMAP_NEGATIVEY)
        {
			if ( !__LoadAllMipSurfaces( fp, D3DCUBEMAP_FACE_POSITIVE_Y, ddsHeader, data ) )
				goto clean_up;
        }

        if (cubeFlags & DDS_CUBEMAP_POSITIVEZ)
        {
			if ( !__LoadAllMipSurfaces( fp, D3DCUBEMAP_FACE_POSITIVE_Z, ddsHeader, data ) )
				goto clean_up;
        }

        if (cubeFlags & DDS_CUBEMAP_NEGATIVEZ)
        {
			if ( !__LoadAllMipSurfaces( fp, D3DCUBEMAP_FACE_NEGATIVE_Z, ddsHeader, data ) )
				goto clean_up;
        }

		cubmap = true;
	}
	else
	{
		dataSize = ddsHeader.dwWidth * ddsHeader.dwHeight * 4;
		comp = new unsigned char[dataSize];
		orig_data = data = new unsigned char[dataSize];
		fread( comp, dataSize, 1, fp );

		if ( format == PF_DXT1 )
		{
			if ( !__DecompressDXT1( fp, ddsHeader, comp, data ) )
				goto clean_up;
		}
		else if ( format == PF_DXT3 )
		{
			if ( !__DecompressDXT3( fp, ddsHeader, comp, data ) )
				goto clean_up;
		}
		else if ( format == PF_DXT5 )
		{
			if ( !__DecompressDXT5( fp, ddsHeader, comp, data ) )
				goto clean_up;
		}
		else
			goto clean_up;
	}

	delete [] comp;

	return orig_data;

clean_up:
	delete [] comp;
	delete [] data;
	return 0;
}

//
// __GetPixelFormat
//
PixelFormat	__GetPixelFormat( DDS_HEADER & ddsHeader )
{
	switch( ddsHeader.ddspf.dwFourCC )
	{
		case MAKEFOURCC('D','X','T','1'):
			return PF_DXT1;
		case MAKEFOURCC('D','X','T','2'):
			return PF_DXT2;
		case MAKEFOURCC('D','X','T','3'):
			return PF_DXT3;
		case MAKEFOURCC('D','X','T','4'):
			return PF_DXT4;
		case MAKEFOURCC('D','X','T','5'):
			return PF_DXT5;
		default:
			return PF_UNKNOWN;
	}
}

// --------------------------------------------------------------------

// Decompression Code

struct DXTColBlock
{
	WORD col0;
	WORD col1;

	// no bit fields - use bytes
	BYTE row[4];
};

struct DXTAlphaBlockExplicit
{
	WORD row[4];
};

struct DXTAlphaBlock3BitLinear
{
	BYTE alpha0;
	BYTE alpha1;

	BYTE stuff[6];
};



// use cast to struct instead of RGBA_MAKE as struct is
//  much
struct Color8888
{
	BYTE r;		// change the order of names to change the 
	BYTE g;		//  order of the output ARGB or BGRA, etc...
	BYTE b;		//  Last one is MSB, 1st is LSB.
	BYTE a;
};


struct Color565
{
	unsigned nBlue  : 5;		// order of names changes
	unsigned nGreen : 6;		//  byte order of output to 32 bit
	unsigned nRed	: 5;
};






inline void GetColorBlockColors( DXTColBlock * pBlock, Color8888 * col_0, Color8888 * col_1, 
													 Color8888 * col_2, Color8888 * col_3,
													 WORD & wrd  )
{
	// There are 4 methods to use - see the Time_ functions.
	// 1st = shift = does normal approach per byte for color comps
	// 2nd = use freak variable bit field color565 for component extraction
	// 3rd = use super-freak DWORD adds BEFORE shifting the color components
	//  This lets you do only 1 add per color instead of 3 BYTE adds and
	//  might be faster
	// Call RunTimingSession() to run each of them & output result to txt file

 
	// freak variable bit structure method
	// normal math
	// This method is fastest

	Color565 * pCol;

	pCol = (Color565*) & (pBlock->col0 );

	col_0->a = 0xff;
	col_0->r = pCol->nRed;
	col_0->r <<= 3;				// shift to full precision
	col_0->g = pCol->nGreen;
	col_0->g <<= 2;
	col_0->b = pCol->nBlue;
	col_0->b <<= 3;

	pCol = (Color565*) & (pBlock->col1 );
	col_1->a = 0xff;
	col_1->r = pCol->nRed;
	col_1->r <<= 3;				// shift to full precision
	col_1->g = pCol->nGreen;
	col_1->g <<= 2;
	col_1->b = pCol->nBlue;
	col_1->b <<= 3;


	if( pBlock->col0 > pBlock->col1 )
	{
		// Four-color block: derive the other two colors.    
		// 00 = color_0, 01 = color_1, 10 = color_2, 11 = color_3
		// These two bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block.

		wrd = ((WORD)col_0->r * 2 + (WORD)col_1->r )/3;
											// no +1 for rounding
											// as bits have been shifted to 888
		col_2->r = (BYTE)wrd;

		wrd = ((WORD)col_0->g * 2 + (WORD)col_1->g )/3;
		col_2->g = (BYTE)wrd;

		wrd = ((WORD)col_0->b * 2 + (WORD)col_1->b )/3;
		col_2->b = (BYTE)wrd;
		col_2->a = 0xff;

		wrd = ((WORD)col_0->r + (WORD)col_1->r *2 )/3;
		col_3->r = (BYTE)wrd;

		wrd = ((WORD)col_0->g + (WORD)col_1->g *2 )/3;
		col_3->g = (BYTE)wrd;

		wrd = ((WORD)col_0->b + (WORD)col_1->b *2 )/3;
		col_3->b = (BYTE)wrd;
		col_3->a = 0xff;

	}
	else
	{
		// Three-color block: derive the other color.
		// 00 = color_0,  01 = color_1,  10 = color_2,  
		// 11 = transparent.
		// These two bit codes correspond to the 2-bit fields 
		// stored in the 64-bit block. 

		// explicit for each component, unlike some refrasts...
		
		// TRACE("block has alpha\n");

		wrd = ((WORD)col_0->r + (WORD)col_1->r )/2;
		col_2->r = (BYTE)wrd;
		wrd = ((WORD)col_0->g + (WORD)col_1->g )/2;
		col_2->g = (BYTE)wrd;
		wrd = ((WORD)col_0->b + (WORD)col_1->b )/2;
		col_2->b = (BYTE)wrd;
		col_2->a = 0xff;

		col_3->r = 0x00;		// random color to indicate alpha
		col_3->g = 0xff;
		col_3->b = 0xff;
		col_3->a = 0x00;

	}
}			//  Get color block colors (...)



inline void DecodeColorBlock( DWORD * pImPos, DXTColBlock * pColorBlock, int width,
								DWORD * col_0,
								DWORD * col_1, DWORD * col_2, DWORD * col_3 )
{
	// width is width of image in pixels


	DWORD bits;
	int r,n;

	// bit masks = 00000011, 00001100, 00110000, 11000000
	const DWORD masks[] = { 3, 12, 3 << 4, 3 << 6 };
	const int   shift[] = { 0, 2, 4, 6 };

	// r steps through lines in y
	for( r=0; r < 4; r++, pImPos += width-4 )	// no width*4 as DWORD ptr inc will *4
	{

		// width * 4 bytes per pixel per line
		// each j dxtc row is 4 lines of pixels

		// pImPos = (DWORD*)((DWORD)pBase + i*16 + (r+j*4) * m_nWidth * 4 );

		// n steps through pixels
		for( n=0; n < 4; n++ )
		{
			bits =		pColorBlock->row[r] & masks[n];
			bits >>=	shift[n];

			switch( bits )
			{
			case 0 :
				*pImPos = *col_0;
				pImPos++;		// increment to next DWORD
				break;
			case 1 :
				*pImPos = *col_1;
				pImPos++;
				break;
			case 2 :
				*pImPos = *col_2;
				pImPos++;
				break;
			case 3 :
				*pImPos = *col_3;
				pImPos++;
				break;
			default:
				pImPos++;
				break;
			}
		}
	}
}



inline void  DecodeAlphaExplicit( DWORD * pImPos, DXTAlphaBlockExplicit * pAlphaBlock,
								  int width, DWORD alphazero )
{
	// alphazero is a bit mask that when & with the image color
	//  will zero the alpha bits, so if the image DWORDs  are
	//  ARGB then alphazero will be 0x00ffffff or if
	//  RGBA then alphazero will be 0xffffff00
	//  alphazero constructed automaticaly from field order of Color8888 structure

	// decodes to 32 bit format only


	int row, pix;

	WORD wrd;

	Color8888 col;
	col.r = col.g = col.b = 0;


	//TRACE("\n");

	for( row=0; row < 4; row++, pImPos += width-4 )
	{
		// pImPow += pImPos += width-4 moves to next row down

		wrd = pAlphaBlock->row[ row ];

		// TRACE("0x%.8x\t\t", wrd);

		for( pix = 0; pix < 4; pix++ )
		{
			// zero the alpha bits of image pixel
			*pImPos &= alphazero;

			col.a = wrd & 0x000f;		// get only low 4 bits
//			col.a <<= 4;				// shift to full byte precision
										// NOTE:  with just a << 4 you'll never have alpha
										// of 0xff,  0xf0 is max so pure shift doesn't quite
										// cover full alpha range.
										// It's much cheaper than divide & scale though.
										// To correct for this, and get 0xff for max alpha,
										//  or the low bits back in after left shifting
			col.a = col.a | (col.a << 4 );	// This allows max 4 bit alpha to be 0xff alpha
											//  in final image, and is crude approach to full 
											//  range scale

			*pImPos |= *((DWORD*)&col);	// or the bits into the prev. nulled alpha

			wrd >>= 4;		// move next bits to lowest 4

			pImPos++;		// move to next pixel in the row

		}
	}
}




BYTE		gBits[4][4];
WORD		gAlphas[8];
Color8888	gACol[4][4];


inline void DecodeAlpha3BitLinear( DWORD * pImPos, DXTAlphaBlock3BitLinear * pAlphaBlock,
									int width, DWORD alphazero)
{

	gAlphas[0] = pAlphaBlock->alpha0;
	gAlphas[1] = pAlphaBlock->alpha1;

	
	// 8-alpha or 6-alpha block?    

	if( gAlphas[0] > gAlphas[1] )
	{
		// 8-alpha block:  derive the other 6 alphas.    
		// 000 = alpha_0, 001 = alpha_1, others are interpolated

		gAlphas[2] = ( 6 * gAlphas[0] +     gAlphas[1]) / 7;	// bit code 010
		gAlphas[3] = ( 5 * gAlphas[0] + 2 * gAlphas[1]) / 7;	// Bit code 011    
		gAlphas[4] = ( 4 * gAlphas[0] + 3 * gAlphas[1]) / 7;	// Bit code 100    
		gAlphas[5] = ( 3 * gAlphas[0] + 4 * gAlphas[1]) / 7;	// Bit code 101
		gAlphas[6] = ( 2 * gAlphas[0] + 5 * gAlphas[1]) / 7;	// Bit code 110    
		gAlphas[7] = (     gAlphas[0] + 6 * gAlphas[1]) / 7;	// Bit code 111
	}    
	else
	{
		// 6-alpha block:  derive the other alphas.    
		// 000 = alpha_0, 001 = alpha_1, others are interpolated

		gAlphas[2] = (4 * gAlphas[0] +     gAlphas[1]) / 5;	// Bit code 010
		gAlphas[3] = (3 * gAlphas[0] + 2 * gAlphas[1]) / 5;	// Bit code 011    
		gAlphas[4] = (2 * gAlphas[0] + 3 * gAlphas[1]) / 5;	// Bit code 100    
		gAlphas[5] = (    gAlphas[0] + 4 * gAlphas[1]) / 5;	// Bit code 101
		gAlphas[6] = 0;										// Bit code 110
		gAlphas[7] = 255;									// Bit code 111
	}


	// Decode 3-bit fields into array of 16 BYTES with same value

	// first two rows of 4 pixels each:
	// pRows = (Alpha3BitRows*) & ( pAlphaBlock->stuff[0] );
	const DWORD mask = 0x00000007;		// bits = 00 00 01 11

	DWORD bits = *( (DWORD*) & ( pAlphaBlock->stuff[0] ));

	gBits[0][0] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[0][1] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[0][2] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[0][3] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[1][0] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[1][1] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[1][2] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[1][3] = (BYTE)( bits & mask );

	// now for last two rows:

	bits = *( (DWORD*) & ( pAlphaBlock->stuff[3] ));		// last 3 bytes

	gBits[2][0] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[2][1] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[2][2] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[2][3] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[3][0] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[3][1] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[3][2] = (BYTE)( bits & mask );
	bits >>= 3;
	gBits[3][3] = (BYTE)( bits & mask );


	// decode the codes into alpha values
	int row, pix;


	for( row = 0; row < 4; row++ )
	{
		for( pix=0; pix < 4; pix++ )
		{
			gACol[row][pix].a = (BYTE) gAlphas[ gBits[row][pix] ];

//			assert( gACol[row][pix].r == 0 );
//			assert( gACol[row][pix].g == 0 );
//			assert( gACol[row][pix].b == 0 );
		}
	}



	// Write out alpha values to the image bits

	for( row=0; row < 4; row++, pImPos += width-4 )
	{
		// pImPow += pImPos += width-4 moves to next row down

		for( pix = 0; pix < 4; pix++ )
		{
			// zero the alpha bits of image pixel
			*pImPos &=  alphazero;

			*pImPos |=  *((DWORD*) &(gACol[row][pix]));	// or the bits into the prev. nulled alpha
			pImPos++;
		}
	}
}

//
// __DecompressDXT3
//
bool __DecompressDXT1( FILE *fp, DDS_HEADER & ddsHeader, unsigned char * comp, unsigned char *& data)
{
	int xblocks, yblocks;

	if( (ddsHeader.dwWidth % 4 ) != 0 )
		return false;
	if( (ddsHeader.dwHeight %4 ) != 0 )
		return false;

	xblocks = ddsHeader.dwWidth / 4;
	yblocks = ddsHeader.dwHeight / 4;

	int i,j;

	DWORD * pBase  = (DWORD*)  data;
	DWORD * pImPos = (DWORD*)  pBase;			// pos in decompressed data
	WORD  * pPos   = (WORD*)   comp;			// pos in compressed data

	DXTColBlock * pBlock;

	Color8888 col_0, col_1, col_2, col_3;

	WORD wrd;

	for( j=0; j < yblocks; j++ )
	{
		// 8 bytes per block
		pBlock = (DXTColBlock*) ( (DWORD)comp + j * xblocks * 8 );

		for( i=0; i < xblocks; i++, pBlock++ )
		{
			// inline func:
			GetColorBlockColors( pBlock, &col_0, &col_1, &col_2, &col_3, wrd );


			// now decode the color block into the bitmap bits
			// inline func:

			pImPos = (DWORD*)((DWORD)pBase + i*16 + (j*4) * ddsHeader.dwWidth * 4 );


			DecodeColorBlock( pImPos, pBlock, ddsHeader.dwWidth, (DWORD*)&col_0, (DWORD*)&col_1,
								(DWORD*)&col_2, (DWORD*)&col_3 );

		}
	}

	return true;
}

//
// __DecompressDXT3
//
bool __DecompressDXT3( FILE *fp, DDS_HEADER & ddsHeader, unsigned char * comp, unsigned char *& data)
{
	int xblocks, yblocks;

	if( (ddsHeader.dwWidth % 4 ) != 0 )
		return false;
	if( (ddsHeader.dwHeight %4 ) != 0 )
		return false;

	xblocks = ddsHeader.dwWidth / 4;
	yblocks = ddsHeader.dwHeight / 4;

	int i,j;

	DWORD * pBase  = (DWORD*)  data;
	DWORD * pImPos = (DWORD*)  pBase;			// pos in decompressed data
	WORD  * pPos   = (WORD*)   comp;			// pos in compressed data

	DXTColBlock * pBlock;
	DXTAlphaBlockExplicit	* pAlphaBlock;

	Color8888 col_0, col_1, col_2, col_3;

	WORD wrd;

	// fill alphazero with appropriate value to zero out alpha when
	//  alphazero is ANDed with the image color 32 bit DWORD:
	col_0.a = 0;
	col_0.r = col_0.g = col_0.b = 0xff;
	DWORD alphazero = *((DWORD*) &col_0);

	for( j=0; j < yblocks; j++ )
	{
		// 8 bytes per block
		// 1 block for alpha, 1 block for color

		pBlock = (DXTColBlock*) ( (DWORD)comp + j * xblocks * 16 );

		for( i=0; i < xblocks; i++, pBlock ++ )
		{

			// inline
			// Get alpha block

			pAlphaBlock = (DXTAlphaBlockExplicit*) pBlock;

			// inline func:
			// Get color block & colors
			pBlock++;
			GetColorBlockColors( pBlock, &col_0, &col_1, &col_2, &col_3, wrd );

			// Decode the color block into the bitmap bits
			// inline func:

			pImPos = (DWORD*)((DWORD)pBase + i*16 + (j*4) * ddsHeader.dwWidth * 4 );


			DecodeColorBlock( pImPos, pBlock, ddsHeader.dwWidth, (DWORD*)&col_0, (DWORD*)&col_1,
								(DWORD*)&col_2, (DWORD*)&col_3 );

			// Overwrite the previous alpha bits with the alpha block
			//  info
			// inline func:
			DecodeAlphaExplicit( pImPos, pAlphaBlock, ddsHeader.dwWidth, alphazero );


		}
	}

	return true;
}

//
// __DecompressDXT5
//
bool __DecompressDXT5( FILE *fp, DDS_HEADER & ddsHeader, unsigned char * comp, unsigned char *& data)
{
	int xblocks, yblocks;

	if( (ddsHeader.dwWidth % 4 ) != 0 )
		return false;
	if( (ddsHeader.dwHeight %4 ) != 0 )
		return false;

	xblocks = ddsHeader.dwWidth / 4;
	yblocks = ddsHeader.dwHeight / 4;

	int i,j;

	DWORD * pBase  = (DWORD*)  data;
	DWORD * pImPos = (DWORD*)  pBase;			// pos in decompressed data
	WORD  * pPos   = (WORD*)   comp;			// pos in compressed data

	DXTColBlock				* pBlock;
	DXTAlphaBlock3BitLinear * pAlphaBlock;

	Color8888 col_0, col_1, col_2, col_3;
	WORD wrd;

	// fill alphazero with appropriate value to zero out alpha when
	//  alphazero is ANDed with the image color 32 bit DWORD:
	col_0.a = 0;
	col_0.r = col_0.g = col_0.b = 0xff;
	DWORD alphazero = *((DWORD*) &col_0);

	for( j=0; j < yblocks; j++ )
	{
		// 8 bytes per block
		// 1 block for alpha, 1 block for color

		pBlock = (DXTColBlock*) ( (DWORD)comp + j * xblocks * 16 );

		for( i=0; i < xblocks; i++, pBlock ++ )
		{

			// inline
			// Get alpha block

			pAlphaBlock = (DXTAlphaBlock3BitLinear*) pBlock;

			// inline func:
			// Get color block & colors
			pBlock++;

			// TRACE("pBlock:   0x%.8x\n", pBlock );

			GetColorBlockColors( pBlock, &col_0, &col_1, &col_2, &col_3, wrd );

			// Decode the color block into the bitmap bits
			// inline func:

			pImPos = (DWORD*)((DWORD)pBase + i*16 + (j*4) * ddsHeader.dwWidth * 4 );


			DecodeColorBlock( pImPos, pBlock, ddsHeader.dwWidth, (DWORD*)&col_0, (DWORD*)&col_1,
								(DWORD*)&col_2, (DWORD*)&col_3 );

			// Overwrite the previous alpha bits with the alpha block
			//  info

			DecodeAlpha3BitLinear( pImPos, pAlphaBlock, ddsHeader.dwWidth, alphazero );


		}
	}

	return true;
}

//
// __LoadAllMipSurfaces
//
bool __LoadAllMipSurfaces(FILE * fp, D3DCUBEMAP_FACES FaceType, DDS_HEADER & ddsHeader, unsigned char *& data)
{
	int mipmaps = ddsHeader.dwMipMapCount == 0 ? 1 : ddsHeader.dwMipMapCount;

    for (int iLevel = 0; iLevel < mipmaps; iLevel++)
    {
		int bytesPerRow;
		if ( memcmp( &ddsHeader.ddspf, &DDSPF_DXT1, sizeof( DDS_PIXELFORMAT ) ) == 0 )
            bytesPerRow = 0;
		else if ( memcmp( &ddsHeader.ddspf, &DDSPF_DXT2, sizeof( DDS_PIXELFORMAT ) ) == 0 )
            bytesPerRow = 0;
		else if ( memcmp( &ddsHeader.ddspf, &DDSPF_DXT3, sizeof( DDS_PIXELFORMAT ) ) == 0 )
            bytesPerRow = 0;
		else if ( memcmp( &ddsHeader.ddspf, &DDSPF_DXT4, sizeof( DDS_PIXELFORMAT ) ) == 0 )
            bytesPerRow = 0;
		else if ( memcmp( &ddsHeader.ddspf, &DDSPF_DXT5, sizeof( DDS_PIXELFORMAT ) ) == 0 )
            bytesPerRow = 0;
		else if ( memcmp( &ddsHeader.ddspf, &DDSPF_A8R8G8B8, sizeof( DDS_PIXELFORMAT ) ) == 0 )
            bytesPerRow = 4 * ddsHeader.dwWidth;
		else if ( memcmp( &ddsHeader.ddspf, &DDSPF_A1R5G5B5, sizeof( DDS_PIXELFORMAT ) ) == 0 )
            bytesPerRow = 2 * ddsHeader.dwWidth;
		else if ( memcmp( &ddsHeader.ddspf, &DDSPF_A4R4G4B4, sizeof( DDS_PIXELFORMAT ) ) == 0 )
            bytesPerRow = 2 * ddsHeader.dwWidth;
		else if ( memcmp( &ddsHeader.ddspf, &DDSPF_R8G8B8, sizeof( DDS_PIXELFORMAT ) ) == 0 )
            bytesPerRow = 3 * ddsHeader.dwWidth;
		else if ( memcmp( &ddsHeader.ddspf, &DDSPF_R5G6B5, sizeof( DDS_PIXELFORMAT ) ) == 0 )
            bytesPerRow = 2 * ddsHeader.dwWidth;

		if ( bytesPerRow == 0 )
		{
			int dataSize = ddsHeader.dwWidth * ddsHeader.dwHeight * 4 * sizeof(unsigned char);
			fread( data, dataSize, 1, fp );
			data += dataSize;
		}
		else
		{
			for ( unsigned int ptr = 0; ptr < ddsHeader.dwHeight; ptr++ )
			{
				fread(data, bytesPerRow, 1, fp );
				data += ddsHeader.dwPitchOrLinearSize;
			}
		}
	}

	return true;
}

//
// __LoadAllVolumeSurfaces
//
bool __LoadAllVolumeSurfaces(FILE * fp, DDS_HEADER & ddsHeader, unsigned char *& data)
{
	return true;
}