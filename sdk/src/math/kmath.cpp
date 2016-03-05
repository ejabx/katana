/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kmath.cpp
	Author:		Eric Bryant

	General math routines
*/

#include "kmath.h"
#include "Point.h"
#include "Matrix.h"
#include <math.h>
using namespace Katana;

//
// createProjection
//
void kmath::createProjection( float fov, float nearp, float farp, float aspect, Matrix4 & projMatrix )
{
	// Compute projection parameters
	float w = ( 1 / aspect ) * (float)( cos( fov/2 ) / sin( fov/2 ) );
	float h =   1.0f  * (float)( cos( fov/2 ) / sin( fov/2 ) );
    float Q = farp / ( farp - nearp );

	// Make sure the output matrix starts as a identity matrix
	projMatrix.setZero();

	// Create the projection matrix
	projMatrix.m[0][0] = w;
	projMatrix.m[1][1] = h;
	projMatrix.m[2][2] = Q;
	projMatrix.m[2][3] = 1.0f;
	projMatrix.m[3][2] = -Q*nearp;
}