/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kmath.h
	Author:		Eric Bryant

	General math routines
*/

#ifndef _KMATH_H
#define _KMATH_H

namespace Katana
{

//
// Forward Declarations
//
class Matrix4;

namespace kmath
{
	
	///
	/// PI Constant, with 31 digit precision
	///
	const float PI = 3.1415926535897932384626433832795f;

	///
	/// EPSILONs
	///
	const float SMALL_EPSILON	= 10e-2f;
	const float EPSILSON		= 10e-6f;
	const float LARGE_EPSILON	= 10e-15f;

	///
	/// powerOf2
	///
	/// Returns the power of 2 for a given number
	///
	inline long powerOf2(long N)
	{
		if ( N == 0 )	return 1L;
		if ( N == 1 )	return 2L;
		if ( N == 2 )	return 4L;
		if ( N == 3 ) 	return 8L;
		if ( N == 4 ) 	return 16L;
		if ( N == 5 ) 	return 32L;
		if ( N == 6 ) 	return 64L;
		else			return 1L;
	}

	///
	/// nextPowerOf2
	/// Returns the next power of 2 for a given number
	///
	inline long nextPowerOf2(long N)
	{

		if (N<=0L		) return 0L;
		if (N<=1L		) return 1L;
		if (N<=2L		) return 2L;
		if (N<=4L		) return 4L;
		if (N<=8L		) return 8L;
		if (N<=16L	    ) return 16L;
		if (N<=32L	    ) return 32L;
		if (N<=64L 	    ) return 64L;
		if (N<=128L     ) return 128L;
		if (N<=256L     ) return 256L;
		if (N<=512L     ) return 512L;
		if (N<=1024L    ) return 1024L;
		if (N<=2048L    ) return 2048L;
		if (N<=4096L    ) return 4096L;
		if (N<=8192L    ) return 8192L;
		if (N<=16384L   ) return 16384L;
		if (N<=32768L   ) return 32768L;
		if (N<=65536L	) return 65536L;
		else			  return 0;
	}

	///
	/// log2
	/// Returns the log2 for a number
	///
	inline long log2(long N)
	{
		long answer = 0;

		while (N >>= 1)
			answer++;

		return answer;
	}

	///
	/// toRadians
	/// Converts angles to radians
	///
	inline float toRadians( float degrees )
	{
		return degrees * PI / 180;
	}

	///
	/// toAngles
	/// Convers radians to angles
	///
	inline float toAngle( float rads )
	{
		return rads * 180 / PI; 
	}

	///
	/// ltof
	/// Converts a long to a float without modifying the bytes
	///
	inline float ltof( long l )
	{
		return *(float *)&l;
	}

	///
	/// createProjection
	/// Creates a projection matrix
	///
	void createProjection( float fov, float nearp, float farp, float aspect, Matrix4 & projMatrix );

	//
	// fabs
	//
	inline float fabs(float f) 
	{
		int i=((*(int*)&f)&0x7fffffff);return (*(float*)&i);
	}

	//
	// fneg
	//
	inline float fneg(float f) 
	{
		int i=((*(int*)&f)^0x80000000);return (*(float*)&i);
	}

	//
	// fsgn
	//
	inline int fsgn(float f) 
	{
		return 1+(((*(int*)&f)>>31)<<1);
	}

}; // kmath

}; // Katana

#endif // _KMATH_H