/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		matrix.cpp
	Author:		Eric Bryant

	Represents a matrix
*/

#include "kmath.h"
#include "point.h"
#include "matrix.h"
#include <memory.h>
#include <math.h>
using namespace Katana;

//
// Constructor
//
Matrix4::Matrix4()
{
	right.w = 0; up.w = 0; at.w = 0; pos.w = 1; // Identity
}

Matrix4::Matrix4 (float m00, float m01, float m02, float m03,
				  float m10, float m11, float m12, float m13,
				  float m20, float m21, float m22, float m23,
				  float m30, float m31, float m32, float m33)
{
	set(m00, m01, m02, m03,
		m10, m11, m12, m13,
		m20, m21, m22, m23,
		m30, m31, m32, m33);
}

//
// Copy Constructor
//
Matrix4::Matrix4 (const Matrix4& mat)
{
	memcpy( &right[0], &mat.right[0], sizeof( *this ) );
}

//
// setIdentity
//
void Matrix4::setIdentity()
{
	// Zero out first
	setZero();

	// Identity
	m[0][0] = 1;
	m[1][1] = 1;
	m[2][2] = 1;
	m[3][3] = 1;
}

//
// setZero
//
void Matrix4::setZero()
{
	memset( &right[0], 0, sizeof(*this) );
}

//
// transpose
//
Matrix4 & Matrix4::transpose()
{
	set( m00, m10, m20, m30,
		 m01, m11, m21, m31,
		 m02, m12, m22, m32,
		 m03, m13, m23, m33 );

	return *this;
}

//
// inverse
//
Matrix4 & Matrix4::inverse()
{
	float fA0 = s[ 0]*s[ 5] - s[ 1]*s[ 4];
	float fA1 = s[ 0]*s[ 6] - s[ 2]*s[ 4];
	float fA2 = s[ 0]*s[ 7] - s[ 3]*s[ 4];
	float fA3 = s[ 1]*s[ 6] - s[ 2]*s[ 5];
	float fA4 = s[ 1]*s[ 7] - s[ 3]*s[ 5];
	float fA5 = s[ 2]*s[ 7] - s[ 3]*s[ 6];
	float fB0 = s[ 8]*s[13] - s[ 9]*s[12];
	float fB1 = s[ 8]*s[14] - s[10]*s[12];
	float fB2 = s[ 8]*s[15] - s[11]*s[12];
	float fB3 = s[ 9]*s[14] - s[10]*s[13];
	float fB4 = s[ 9]*s[15] - s[11]*s[13];
	float fB5 = s[10]*s[15] - s[11]*s[14];

	float fDet = fA0*fB5-fA1*fB4+fA2*fB3+fA3*fB2-fA4*fB1+fA5*fB0;
	if ( fabs(fDet) <= kmath::LARGE_EPSILON )
		return *this; // No Inverse

	Matrix4 kInv;
	kInv.m[0][0] = + s[ 5]*fB5 - s[ 6]*fB4 + s[ 7]*fB3;
	kInv.m[1][0] = - s[ 4]*fB5 + s[ 6]*fB2 - s[ 7]*fB1;
	kInv.m[2][0] = + s[ 4]*fB4 - s[ 5]*fB2 + s[ 7]*fB0;
	kInv.m[3][0] = - s[ 4]*fB3 + s[ 5]*fB1 - s[ 6]*fB0;
	kInv.m[0][1] = - s[ 1]*fB5 + s[ 2]*fB4 - s[ 3]*fB3;
	kInv.m[1][1] = + s[ 0]*fB5 - s[ 2]*fB2 + s[ 3]*fB1;
	kInv.m[2][1] = - s[ 0]*fB4 + s[ 1]*fB2 - s[ 3]*fB0;
	kInv.m[3][1] = + s[ 0]*fB3 - s[ 1]*fB1 + s[ 2]*fB0;
	kInv.m[0][2] = + s[13]*fA5 - s[14]*fA4 + s[15]*fA3;
	kInv.m[1][2] = - s[12]*fA5 + s[14]*fA2 - s[15]*fA1;
	kInv.m[2][2] = + s[12]*fA4 - s[13]*fA2 + s[15]*fA0;
	kInv.m[3][2] = - s[12]*fA3 + s[13]*fA1 - s[14]*fA0;
	kInv.m[0][3] = - s[ 9]*fA5 + s[10]*fA4 - s[11]*fA3;
	kInv.m[1][3] = + s[ 8]*fA5 - s[10]*fA2 + s[11]*fA1;
	kInv.m[2][3] = - s[ 8]*fA4 + s[ 9]*fA2 - s[11]*fA0;
	kInv.m[3][3] = + s[ 8]*fA3 - s[ 9]*fA1 + s[10]*fA0;

	float fInvDet = ((float)1.0)/fDet;
	for (int iRow = 0; iRow < 4; iRow++)
	{
		for (int iCol = 0; iCol < 4; iCol++)
			kInv.m[iRow][iCol] *= fInvDet;
	}

	return ( *this = kInv );
}

//
// operator*=
//
Matrix4 & Matrix4::operator*=(const Matrix4 & mat)
{
	set(
		s[ 0] * mat.s[ 0] + s[ 1] * mat.s[ 4] + s[ 2] * mat.s[ 8] + s[ 3] * mat.s[12], // s0
		s[ 0] * mat.s[ 1] + s[ 1] * mat.s[ 5] + s[ 2] * mat.s[ 9] + s[ 3] * mat.s[13], // s1
		s[ 0] * mat.s[ 2] + s[ 1] * mat.s[ 6] + s[ 2] * mat.s[10] + s[ 3] * mat.s[14], // s2
		s[ 0] * mat.s[ 3] + s[ 1] * mat.s[ 7] + s[ 2] * mat.s[11] + s[ 3] * mat.s[15], // s3

		s[ 4] * mat.s[ 0] + s[ 5] * mat.s[ 4] + s[ 6] * mat.s[ 8] + s[ 7] * mat.s[12], // s4
		s[ 4] * mat.s[ 1] + s[ 5] * mat.s[ 5] + s[ 6] * mat.s[ 9] + s[ 7] * mat.s[13], // s5
		s[ 4] * mat.s[ 2] + s[ 5] * mat.s[ 6] + s[ 6] * mat.s[10] + s[ 7] * mat.s[14], // s6
		s[ 4] * mat.s[ 3] + s[ 5] * mat.s[ 7] + s[ 6] * mat.s[11] + s[ 7] * mat.s[15], // s7

		s[ 8] * mat.s[ 0] + s[ 9] * mat.s[ 4] + s[10] * mat.s[ 8] + s[11] * mat.s[12], // s8
		s[ 8] * mat.s[ 1] + s[ 9] * mat.s[ 5] + s[10] * mat.s[ 9] + s[11] * mat.s[13], // s9
		s[ 8] * mat.s[ 2] + s[ 9] * mat.s[ 6] + s[10] * mat.s[10] + s[11] * mat.s[14], // s10
		s[ 8] * mat.s[ 3] + s[ 9] * mat.s[ 7] + s[10] * mat.s[11] + s[11] * mat.s[15], // s11

		s[12] * mat.s[ 0] + s[13] * mat.s[ 4] + s[14] * mat.s[ 8] + s[15] * mat.s[12], // s12
		s[12] * mat.s[ 1] + s[13] * mat.s[ 5] + s[14] * mat.s[ 9] + s[15] * mat.s[13], // s13
		s[12] * mat.s[ 2] + s[13] * mat.s[ 6] + s[14] * mat.s[10] + s[15] * mat.s[14], // s14
		s[12] * mat.s[ 3] + s[13] * mat.s[ 7] + s[14] * mat.s[11] + s[15] * mat.s[15]  // s15
		);

	return*this;
}

Matrix4 Matrix4::operator*(const Matrix4 & mat) const
{
	return Matrix4(
		s[ 0] * mat.s[ 0] + s[ 1] * mat.s[ 4] + s[ 2] * mat.s[ 8] + s[ 3] * mat.s[12], // s0
		s[ 0] * mat.s[ 1] + s[ 1] * mat.s[ 5] + s[ 2] * mat.s[ 9] + s[ 3] * mat.s[13], // s1
		s[ 0] * mat.s[ 2] + s[ 1] * mat.s[ 6] + s[ 2] * mat.s[10] + s[ 3] * mat.s[14], // s2
		s[ 0] * mat.s[ 3] + s[ 1] * mat.s[ 7] + s[ 2] * mat.s[11] + s[ 3] * mat.s[15], // s3

		s[ 4] * mat.s[ 0] + s[ 5] * mat.s[ 4] + s[ 6] * mat.s[ 8] + s[ 7] * mat.s[12], // s4
		s[ 4] * mat.s[ 1] + s[ 5] * mat.s[ 5] + s[ 6] * mat.s[ 9] + s[ 7] * mat.s[13], // s5
		s[ 4] * mat.s[ 2] + s[ 5] * mat.s[ 6] + s[ 6] * mat.s[10] + s[ 7] * mat.s[14], // s6
		s[ 4] * mat.s[ 3] + s[ 5] * mat.s[ 7] + s[ 6] * mat.s[11] + s[ 7] * mat.s[15], // s7

		s[ 8] * mat.s[ 0] + s[ 9] * mat.s[ 4] + s[10] * mat.s[ 8] + s[11] * mat.s[12], // s8
		s[ 8] * mat.s[ 1] + s[ 9] * mat.s[ 5] + s[10] * mat.s[ 9] + s[11] * mat.s[13], // s9
		s[ 8] * mat.s[ 2] + s[ 9] * mat.s[ 6] + s[10] * mat.s[10] + s[11] * mat.s[14], // s10
		s[ 8] * mat.s[ 3] + s[ 9] * mat.s[ 7] + s[10] * mat.s[11] + s[11] * mat.s[15], // s11

		s[12] * mat.s[ 0] + s[13] * mat.s[ 4] + s[14] * mat.s[ 8] + s[15] * mat.s[12], // s12
		s[12] * mat.s[ 1] + s[13] * mat.s[ 5] + s[14] * mat.s[ 9] + s[15] * mat.s[13], // s13
		s[12] * mat.s[ 2] + s[13] * mat.s[ 6] + s[14] * mat.s[10] + s[15] * mat.s[14], // s14
		s[12] * mat.s[ 3] + s[13] * mat.s[ 7] + s[14] * mat.s[11] + s[15] * mat.s[15]  // s15
	);
}

//
// makeRotation
void Matrix4::makeRotation( float angle, const Point3 & axis )
{
	const float x = axis.x;
	const float y = axis.y;
	const float z = axis.z;

    float cs = (float)cos(angle);
    float sn = (float)sin(angle);
    float omcs = 1.0f-cs;
    float x2 = x*x;
    float y2 = y*y;
    float z2 = z*z;
    float xym = x*y*omcs;
    float xzm = x*z*omcs;
    float yzm = y*z*omcs;
    float xsin = x*sn;
    float ysin = y*sn;
    float zsin = z*sn;
    
    m[0][0] = x2*omcs+cs;
    m[0][1] = xym+zsin;
    m[0][2] = xzm-ysin;
    m[1][0] = xym-zsin;
    m[1][1] = y2*omcs+cs;
    m[1][2] = yzm+xsin;
    m[2][0] = xzm+ysin;
    m[2][1] = yzm-xsin;
    m[2][2] = z2*omcs+cs;
}