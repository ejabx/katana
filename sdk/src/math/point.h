/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		point.h
	Author:		Eric Bryant

	Represents a point in 3d space.
*/

#ifndef _POINT_H
#define _POINT_H

namespace Katana
{

//
// Forward Declarations
//
class Matrix4;

/// Axis
enum
{
	x = 0,
	y = 1,
	z = 2,
	w = 3,
};

/// Textures
enum
{
	u = 0,
	v = 1,
};

///
/// Point2
///
class Point2
{
public:
	/// Constructor
	Point2();
	Point2 (float fX, float fY);

	/// Copy Constructor
	Point2 (const Point2& pt);

	/// Operator overload for indexing
	float& operator[] (int i) const {
		const float* base = &x;
		return (float&) base[i];
	}

	/// Assignment
	Point2& operator= (const Point2& pt)			{ x = pt.x; y = pt.y; return *this; }

	/// Equality
	bool operator== (const Point2& pt) const		{ return ( x == pt.x && y == pt.y ); }

	/// Greater than or equal
	bool operator>= (const Point2& pt) const		{ return ( x >= pt.x && y >= pt.y ); }

	/// Less than or equal
	bool operator<= (const Point2& pt) const		{ return ( x <= pt.x && y <= pt.y ); }

	/// Not Equal
	bool operator!= (const Point2& pt) const		{ return !(*this == pt); }

	/// Negation
	Point2 operator-()								{ return Point2( -x, -y ); }

	/// Common Arithmetic
	Point2  operator+  (const Point2& pt) const			{ return Point2(x+pt.x, y+pt.y); }
	Point2& operator+= (const Point2& pt)				{ x += pt.x; y += pt.y; return *this; }
	Point2& operator-= (const Point2& pt)				{ x -= pt.x; y -= pt.y; return *this; }
	Point2& operator*= (float fScalar)					{ x *= fScalar; y *= fScalar; return *this; }
	Point2& operator/= (float fScalar)					{ x /= fScalar; y /= fScalar; return *this; }
	float operator*  (const Point2& pt) const			{ return (x*pt.x + y*pt.y); }
	Point2 operator* (float s) const					{ return Point2(x*s, y*s); }
	Point2 operator-(const Point2 & pt) const			{ return Point2(x-pt.x, y-pt.y); }
	friend Point2 operator* (float s, const Point2& pt) { return Point2(s*pt.x, s*pt.y); }

	/// Return the length of the point
	float getLength() const;

	/// Return the squared length
	float getSqrLength() const;

	/// Return the dot product
	float getDot(const Point2 & pt) const;

	/// Normalize the point
	float getNormalized();

public:
	float x, y;
};

///
/// Point3
///
class Point3
{
public:
	/// Constructor
	Point3();
	Point3 (float fX, float fY, float fZ);

	/// Copy Constructor
    Point3 (const Point3& pt);

	/// Operator overload for indexing
    float& operator[] (int i) const {
        const float* base = &x;
        return (float&) base[i];
    }

	/// Assignment
	Point3& operator= (const Point3& pt)			{ x = pt.x; y = pt.y; z = pt.z; return *this; }

	/// Equality
	bool operator== (const Point3& pt) const		{ return ( x == pt.x && y == pt.y && z == pt.z ); }

	/// Greater than or equal
	bool operator>= (const Point3& pt) const		{ return ( x >= pt.x && y >= pt.y && z >= pt.z ); }

	/// Less than or equal
	bool operator<= (const Point3& pt) const		{ return ( x <= pt.x && y <= pt.y && z <= pt.z ); }

	/// Not Equal
    bool operator!= (const Point3& pt) const		{ return !(*this == pt); }

	/// Negation
	Point3 operator-()								{ return Point3( -x, -y, -z ); }

	/// Common Arithmetic
	Point3  operator+  (const Point3& pt) const			{ return Point3(x+pt.x, y+pt.y, z+pt.z); }
	Point3& operator+= (const Point3& pt)				{ x += pt.x; y += pt.y; z += pt.z; return *this; }
	Point3& operator-= (const Point3& pt)				{ x -= pt.x; y -= pt.y; z -= pt.z; return *this; }
	Point3& operator*= (float fScalar)					{ x *= fScalar; y *= fScalar; z *= fScalar; return *this; }
	Point3& operator/= (float fScalar)					{ x /= fScalar; y /= fScalar; z /= fScalar; return *this; }
	float operator*  (const Point3& pt) const			{ return (x*pt.x + y*pt.y + z*pt.z); }
	Point3 operator* (float s) const					{ return Point3(x*s, y*s, z*s); }
	Point3 operator/ (float s) const					{ return Point3(x/s, y/s, z/s); }
	Point3 operator-(const Point3 & pt) const			{ return Point3(x-pt.x, y-pt.y, z-pt.z); }
	friend Point3 operator* (float s, const Point3& pt) { return Point3(s*pt.x, s*pt.y, s*pt.z); }
	Point3& operator*= (const Matrix4 & m);
	Point3 operator*(const Matrix4 & m) const;

	/// Return the length of the point
	float getLength() const;

	/// Return the squared length
	float getSqrLength() const;

	/// Return the dot product
	float getDot(const Point3 & pt) const;

	/// Normalize the point
	Point3 & getNormalized();

	/// Return the Cross Product
	Point3 getCross(const Point3 & pt) const;

public:
	float x, y, z;
};

///
/// Point4
///
class Point4 : public Point3
{
public:
	/// Constructor
	Point4();
	Point4(float fX, float fY, float fZ, float fW = 1.f);

	/// Copy Constructor
	Point4(const Point3& pt);
	Point4(const Point4& pt);

	/// Assignment
	Point4& operator= (const Point4& pt)			{ x = pt.x; y = pt.y; z = pt.z; w = pt.w; return *this; }

	/// Equality
	bool operator== (const Point4& pt) const		{ return ( x == pt.x && y == pt.y && z == pt.z && w == pt.w ); }

	/// Greater than or equal
	bool operator>= (const Point4& pt) const		{ return ( x >= pt.x && y >= pt.y && z >= pt.z && w >= pt.w ); }

	/// Less than or equal
	bool operator<= (const Point4& pt) const		{ return ( x <= pt.x && y <= pt.y && z <= pt.z && w <= pt.w ); }

	/// Not Equal
	bool operator!= (const Point4& pt) const		{ return !(*this == pt); }

	/// Negation
	Point4 operator-()								{ return Point4( -x, -y, -z ); }

	/// Common Arithmetic
	Point4  operator+  (const Point4& pt) const			{ return Point4(x+pt.x, y+pt.y, z+pt.z); }
	Point4& operator+= (const Point4& pt)				{ x += pt.x; y += pt.y; z += pt.z; return *this; }
	Point4& operator-= (const Point4& pt)				{ x -= pt.x; y -= pt.y; z -= pt.z; return *this; }
	Point4& operator*= (float fScalar)					{ x *= fScalar; y *= fScalar; z *= fScalar; return *this; }
	Point4& operator/= (float fScalar)					{ x /= fScalar; y /= fScalar; z /= fScalar; return *this; }
	float operator*  (const Point4& pt) const			{ return (x*pt.x + y*pt.y + z*pt.z); }
	Point4 operator* (float s) const					{ return Point4(x*s, y*s, z*s); }
	Point4 operator-(const Point4 & pt) const			{ return Point4(x-pt.x, y-pt.y, z-pt.z); }
	friend Point4 operator* (float s, const Point4& pt) { return Point4(s*pt.x, s*pt.y, s*pt.z); }
	Point4& operator*= (const Matrix4 & m);
	Point4 operator*(const Matrix4 & m) const;

public:
	float w;
};

}; // Katana

#endif // _POINT_H