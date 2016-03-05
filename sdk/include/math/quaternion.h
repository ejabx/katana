/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		quaternion.h
	Author:		Eric Bryant
	Credits:	Magic Software, Inc.
				http://www.magic-software.com

	Represents a 4d quaternion [(x,y,z),w]
*/

#ifndef _QUATERNION_H
#define _QUATERNION_H

namespace Katana
{

// Forward Declarations
class Point3;

///
/// Quaternion
///
class Quaternion
{
public:
	/// Constructor
	Quaternion();
    Quaternion(float fX, float fY, float fZ, float fW);
	Quaternion( float angle, const Point3 & axis );

	/// Copy Constructor
    Quaternion(const Quaternion& q);
    
	/// Convert the Quaternion into a rotation matrix
	void toMatrix(Matrix4 & m) const;

	/// Converts the Matrix into a Quaternion
	void fromMatrix(const Matrix4 & m);

	/// Applies the rotation of the quaternion around a point
	Point3 rotate(const Point3 & pt) const;

	/// Converts from a angle/axis rotation
    void fromAngleAxis (float angle, const Point3& axis);

	/// Returns the dot product between this quaternion and another
    float dot (const Quaternion& q) const;

	/// Normalises this quaternion, and returns the previous length
	float normalise(); 

	/// Operator overload function
	void operator*=( const Quaternion & q );
	Quaternion operator+( const Quaternion& q ) const;
	Quaternion operator-( const Quaternion& q ) const;
	Quaternion operator*( const Quaternion& q ) const;
	Quaternion operator*( float fScalar ) const;
	friend Quaternion operator*( float fScalar, const Quaternion& q );

	/// Slerps (Spherical Linear Linerpolation) between two quaternions
	static Quaternion slerp( float fT, const Quaternion & q1, const Quaternion & q2, bool bShortestPart = false );

	/// Squads (Spherical Quadratic Linerpolation) between four quads
	static Quaternion Squad (float fT, const Quaternion& rkP, const Quaternion& rkA, 
							 const Quaternion& rkB, const Quaternion& rkQ, bool shortestPath = false);

public:
	float x, y, z, w;
};

//
// Inline
//

//
// Quaternion::constructors
//
inline Quaternion::Quaternion() 
	: x( 0 )
	, y( 0 )
	, z( 0 )
	, w( 1 ) // This is a unit quaternion
{
}

inline Quaternion::Quaternion( float fX, float fY, float fZ, float fW ) 
	: x( fX )
	, y( fY )
	, z( fZ )
	, w( fW )
{
}

inline Quaternion::Quaternion( float angle, const Point3 & axis )
{
	fromAngleAxis( angle, axis );
}

inline Quaternion::Quaternion( const Quaternion& q )
	: x( q.x )
	, y( q.y )
	, z( q.z )
	, w( q.w )
{
}

//
// Quaternion::operators
//

inline Quaternion Quaternion::operator+ ( const Quaternion& q ) const
{
	return Quaternion( x+q.x, y+q.y, z+q.z, w+q.w );
}

inline Quaternion Quaternion::operator- ( const Quaternion& q ) const
{
	return Quaternion( x-q.x, y-q.y, z-q.z, w-q.w );
}

inline void Quaternion::operator*=(const Quaternion & q)
{
	float qx, qy, qz, qw;
	qx = x;
	qy = y;
	qz = z;
	qw = w;

	x = qw * q.x + qx * q.w + qy * q.z - qz * q.y;
	y = qw * q.y - qx * q.z + qy * q.w + qz * q.x;
	z = qw * q.z + qx * q.y - qy * q.x + qz * q.w;
	w = qw * q.w - qx * q.x - qy * q.y - qz * q.z;
}

inline Quaternion Quaternion::operator* ( const Quaternion& q ) const
{
	// NOTE:  Multiplication is not generally commutative, so in most
	// cases p*q != q*p.

	return Quaternion
		(
		w * q.w - x * q.x - y * q.y - z * q.z,
		w * q.x + x * q.w + y * q.z - z * q.y,
		w * q.y + y * q.w + z * q.x - x * q.z,
		w * q.z + z * q.w + x * q.y - y * q.x
		);
}

inline Quaternion Quaternion::operator*( float fScalar ) const
{
	return Quaternion( fScalar*x, fScalar*y, fScalar*z, fScalar*w );
}

inline Quaternion operator*( float fScalar, const Quaternion& q )
{
	return Quaternion( fScalar*q.x, fScalar*q.y, fScalar*q.z, fScalar*q.w );
}

//
// Quaternion::dot
//
inline float Quaternion::dot(const Quaternion& q) const
{
	return w*q.w + x*q.x + y*q.y + z*q.z;
}


}; // Katana

#endif // _QUATERNION_H