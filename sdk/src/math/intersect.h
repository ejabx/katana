/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		intersect.h
	Author:		Eric Bryant

	Intersection testing functions
*/

#ifndef _INTERSECT_H
#define _INTERSECT_H

namespace Katana
{

// Forward Declarations
class AxisAlignedBox;
class Bound;
class Point3;
class Plane;

namespace kmath
{


/// Tests whether an Axis Aligned Box intersects with a point
bool testIntersect( const AxisAlignedBox & aabb, const Point3 & point );

/// Tests whether an Axis Aligned Box intersects with another Axis Aligned Box
bool testIntersect( const AxisAlignedBox & a, const AxisAlignedBox & b );

/// Tests whether a Sphere intersects with a point
bool testIntersect( const Bound & bound, const Point3 & point );

/// Tests whether a Sphere intersects with another Sphere
bool testIntersect( const Bound & a, const Bound & b );

/// Tests whether a Sphere intersects with a triangle
bool testIntersect( const Bound & bound, const Point3 & vert0, const Point3 & vert1, const Point3 & vert2 );

/// Tests whether an Axis Aligned Box intersects with a triangle
bool testIntersect( const AxisAlignedBox & aabb, const Point3 & vert0, const Point3 & vert1, const Point3 & vert2 );

/// Tests whether a point intersects with a triangle
bool testIntersect( const Point3 & point, const Point3 & vert0, const Point3 & vert1, const Point3 & vert2 );

/// Tests whether a Plane intersects with a Sphere
bool testIntersect( const Plane & plane, const Bound & bound );

/// Tests whether a Plane intersects with a Axis Aligned Box
bool testIntersect( const Plane & plane, const AxisAlignedBox & aabb );


//
// Inline
//

//
// Sphere vs. Sphere Intersection
//
inline bool testIntersect( const Bound & a, const Bound & b )
{
	float length = ( a.m_center - b.m_center ).getLength();
	float lengthSqrt = length * length;

	float radiusDelta = a.m_radius - b.m_radius;
	float radiusSum = a.m_radius + b.m_radius;

	if ( lengthSqrt <= radiusDelta * radiusDelta )
		return true; // Full Intersection
	if ( lengthSqrt <= radiusSum * radiusSum )
		return true; // Partial Intersection

	return false; // No Intersection
}


} // kmath

} // Katana

#endif // _INTERSECT_H