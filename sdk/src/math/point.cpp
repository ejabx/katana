/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		point.cpp
	Author:		Eric Bryant

	Represents a point in 3d space.
*/

#include "point.h"
#include "matrix.h"
#include <math.h>
using namespace Katana;

// ------------------------------------------------------
// Point2
// ------------------------------------------------------

//
// Constructor
//
Point2::Point2() :
	x(0), y(0)
{
}

Point2::Point2 (float fX, float fY) :
	x(fX), y(fY)
{
}

Point2::Point2 (const Point2& pt) :
	x(pt.x), y(pt.y)
{
}

//
// getLength
//
float Point2::getLength() const
{
	return (float)sqrt( x*x+y*y );
}

//
// getSqrLength
//
float Point2::getSqrLength() const
{
	return ( x*x+y*y );
}

//
// getDot
//
float Point2::getDot(const Point2 & pt) const
{
	return x*pt.x + y*pt.y;
}

//
// getNormalized
//
float Point2::getNormalized()
{
	float fLength = getLength();

	if ( fLength > 1e-06f )
	{
		float fRecip = 1.0f/fLength;
		x *= fRecip;
		y *= fRecip;
	}
	else
	{
		x = 0.0f;
		y = 0.0f;
		fLength = 0.0f;
	}

	return fLength;
}


// ------------------------------------------------------
// Point3
// ------------------------------------------------------

//
// Constructor
//
Point3::Point3() :
	x(0), y(0), z(0)
{
}

Point3::Point3 (float fX, float fY, float fZ) :
	x(fX), y(fY), z(fZ)
{
}

Point3::Point3 (const Point3& pt) :
	x(pt.x), y(pt.y), z(pt.z)
{
}

//
// getLength
//
float Point3::getLength() const
{
	return (float)sqrt( x*x+y*y+z*z );
}

//
// getSqrLength
//
float Point3::getSqrLength() const
{
	return ( x*x+y*y+z*z );
}

//
// getDot
//
float Point3::getDot(const Point3 & pt) const
{
	return x*pt.x + y*pt.y + z*pt.z;
}

//
// getNormalized
//
Point3& Point3::getNormalized()
{
    float fLength = getLength();

    if ( fLength > 1e-06f )
    {
        float fRecip = 1.0f/fLength;
        x *= fRecip;
        y *= fRecip;
        z *= fRecip;
    }
    else
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        fLength = 0.0f;
    }

    return *this;
}

//
// getCross
//
Point3 Point3::getCross(const Point3 & pt) const
{
    return Point3(y*pt.z-z*pt.y,z*pt.x-x*pt.z,x*pt.y-y*pt.x);
}

//
// operator*=(Matrix4)
//
Point3& Point3::operator*= (const Matrix4 & m)
{
    x = m.m[0][0]*x+m.m[0][1]*y+m.m[0][2]*z;
	y = m.m[1][0]*x+m.m[1][1]*y+m.m[1][2]*z;
    z = m.m[2][0]*x+m.m[2][1]*y+m.m[2][2]*z;
	*this += m.pos;
	return *this;
}

//
// operator*(Matrix4)
//
Point3 Point3::operator*(const Matrix4 & m) const
{
	Point3 p(m.m[0][0]*x+m.m[0][1]*y+m.m[0][2]*z,
			 m.m[1][0]*x+m.m[1][1]*y+m.m[1][2]*z,
			 m.m[2][0]*x+m.m[2][1]*y+m.m[2][2]*z );

	return p;
}

// ------------------------------------------------------
// Point4
// ------------------------------------------------------

//
// Constructor
//
Point4::Point4() :
	w(1)
{
}

Point4::Point4(float fX, float fY, float fZ, float fW) :
	Point3( fX, fY, fZ ),
	w(fW)
{
}

Point4::Point4(const Point3& pt) :
	Point3(pt),
	w(1.f)
{
}

Point4::Point4(const Point4& pt) :
	Point3(pt),
	w(pt.w)
{
}

Point4 & Point4::operator*= (const Matrix4 & m)
{
	x = m.m[0][0]*x+m.m[0][1]*y+m.m[0][2]*z;
	y = m.m[1][0]*x+m.m[1][1]*y+m.m[1][2]*z;
	z = m.m[2][0]*x+m.m[2][1]*y+m.m[2][2]*z;
	return *this;
}

Point4 Point4::operator*(const Matrix4 & m) const
{
	Point4 p(m.m[0][0]*x+m.m[0][1]*y+m.m[0][2]*z,
			 m.m[1][0]*x+m.m[1][1]*y+m.m[1][2]*z,
			 m.m[2][0]*x+m.m[2][1]*y+m.m[2][2]*z );

	return p;
}