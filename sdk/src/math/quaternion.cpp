/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		quaternion.cpp
	Author:		Eric Bryant
	Credits:	Magic Software, Inc.
				http://www.magic-software.com

	Represents a 4d quaternion [(x,y,z),w]
*/

#include "kmath.h"
#include "point.h"
#include "matrix.h"
#include "quaternion.h"
#include <math.h>
using namespace Katana;
    
//
// toMatrix
//
void Quaternion::toMatrix(Matrix4 & m) const
{
	// operations (*,+,-) = 24
	float tx  = 2.0f*x;
	float ty  = 2.0f*y;
	float tz  = 2.0f*z;
	float twx = tx*w;
	float twy = ty*w;
	float twz = tz*w;
	float txx = tx*x;
	float txy = ty*x;
	float txz = tz*x;
	float tyy = ty*y;
	float tyz = tz*y;
	float tzz = tz*z;

	m.m[0][0] = 1.0f-(tyy+tzz);
	m.m[0][1] = txy-twz;
	m.m[0][2] = txz+twy;
	m.m[1][0] = txy+twz;
	m.m[1][1] = 1.0f-(txx+tzz);
	m.m[1][2] = tyz-twx;
	m.m[2][0] = txz-twy;
	m.m[2][1] = tyz+twx;
	m.m[2][2] = 1.0f-(txx+tyy);
}

//
// fromMatrix
void Quaternion::fromMatrix(const Matrix4 & m)
{
    // Algorithm in Ken Shoemake's article in 1987 SIGGraPH course notes
    // article "Quaternion Calculus and Fast Animation".

    float fTrace = m.m[0][0]+m.m[1][1]+m.m[2][2];
    float fRoot;

    if ( fTrace > 0.0f )
    {
        // |w| > 1/2, may as well choose w > 1/2
        fRoot = (float) sqrt( fTrace+1.0f );  // 2w
        w = 0.5f*fRoot;
        fRoot = 0.5f/fRoot;  // 1/(4w)
        x = m.m[2][1]-m.m[1][2]*fRoot;
        y = m.m[0][2]-m.m[2][0]*fRoot;
        z = m.m[1][0]-m.m[0][1]*fRoot;
    }
    else
    {
        // |w| <= 1/2
        static int next[3] = { 1, 2, 0 };
        int i = 0;
        if ( m.m[1][1] > m.m[0][0] )
            i = 1;
        if ( m.m[2][2] > m.m[i][i] )
            i = 2;
        int j = next[i];
        int k = next[j];

        fRoot = (float) sqrt( m.m[i][i]-m.m[j][j]-m.m[k][k]+1.0f );
        float* quat[3] = { &x, &y, &z };
        *quat[i] = 0.5f*fRoot;
        fRoot = 0.5f/fRoot;
        w = m.m[k][j]-m.m[j][k]*fRoot;
        *quat[j] = m.m[j][i]+m.m[i][j]*fRoot;
        *quat[k] = m.m[k][i]+m.m[i][k]*fRoot;
    }
}

//
// rotate
//
Point3 Quaternion::rotate(const Point3 & pt) const
{
	Point3 out;

	Point3 temp(pt);
	Point3 imag(x, y, z);

	out = temp*(2*w*w-1.0f);
	out += 2.0f*(imag.getDot( temp ) * imag+w*imag.getCross( temp ));

	return out;
}

//
// fromAngleAxis
//
void Quaternion::fromAngleAxis (float angle, const Point3& axis)
{
    float halfAngle = angle * 0.5f;
    float sn = (float) sin (halfAngle);
    w = (float) cos (halfAngle);
    x = axis.x * sn;
    y = axis.y * sn;
    z = axis.z * sn;
}

//
// normalize
// Normalises this quaternion, and returns the previous length
//
float Quaternion::normalise()
{
	float len = w*w+x*x+y*y+z*z;
	float factor = 1.0f / (float)sqrt(len);
	*this = *this * factor;
	return len;
}

//
// slerp
//
Quaternion Quaternion::slerp( float fT, const Quaternion & q1, const Quaternion & q2, bool bShortestPart )
{
	float fCos = q1.dot(q2);
	float fAngle = (float)acos(fCos);

	if ( fabs(fAngle) < kmath::LARGE_EPSILON )
		return q1;

	float fSin = (float)sin(fAngle);
	float fInvSin = 1.0/fSin;
	float fCoeff0 = (float)sin((1.0-fT)*fAngle)*fInvSin;
	float fCoeff1 = (float)sin(fT*fAngle)*fInvSin;

	// Do we need to invert rotation?
	if ( fCos < 0.0f && bShortestPart )
	{
		fCoeff0 = -fCoeff0;
		// taking the complement requires renormalisation
		Quaternion t(fCoeff0*q1 + fCoeff1*q2);
		t.normalise();
		return t;
	}
	else
	{
		return fCoeff0*q1 + fCoeff1*q2;
	}
}

//
// squad
//
Quaternion Quaternion::Squad( float fT,	const Quaternion& rkP, const Quaternion& rkA, 
										const Quaternion& rkB, const Quaternion& rkQ, bool shortestPath )
{
	float fSlerpT = 2.0*fT*(1.0-fT);
	Quaternion kSlerpP = slerp(fT, rkP, rkQ, shortestPath);
	Quaternion kSlerpQ = slerp(fT, rkA, rkB);
	return slerp(fSlerpT, kSlerpP ,kSlerpQ);
}