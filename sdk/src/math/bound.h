/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		bound.h
	Author:		Eric Bryant

	Represents a bounding volume (used for culling).
*/

#ifndef _BOUND_H
#define _BOUND_H

namespace Katana
{

//
// Forward Declarations
//
class Point3;
class Quaternion;
class AxisAlignedBox;

///
/// Intersections
///
typedef enum IntersectionType
{
	INTERSECTION_NONE,
	INTERSECTION_PARTIAL,
	INTERSECTION_FULL
};

///
/// Bound
/// Represents a bounding volume used for culling the visible objects.
/// Currently, only a sphere test is used.
///
class Bound
{
public:
	/// Constructor
	Bound();

	/// Constructor with sphere parameters
	Bound( const Point3 & center, float radius );

	/// Constructor which takes an Axis Aligned Bounding Box
	Bound( const AxisAlignedBox & aabb );

	/// Copy Constructor
	Bound(const Bound & bv );

	/// Sets the bounds
	void set( const Point3 & center, float radius )	{ m_center = center; m_radius = radius; }

	/// Sets the center
	void setCenter( const Point3 & center )					{ m_center = center; }

	/// Sets the radius
	void setRadius( float radius )							{ m_radius = radius; }

	/// Retrieves the center of the bounding region
	const Point3 & getCenter() const						{ return m_center; }

	/// Retrieves the radius of the bounding region
	float getRadius() const									{ return m_radius; }

	/// Returns whether the bound is valid
	bool isValid() const									{ return m_radius > 0; }

	/// Expands the bounding region by another bounding region (sphere expansion)
	void expand( const Bound & bv );

	/// Transform the bounding region. Note as the bounds internally is a sphere,
	/// it is currently unaffected by the rotation, however, the center will be rotated
	void transform( const Matrix4 & transform );
	void transform( const Point3 & translation );
	void transform( const Point3 & translation, const Quaternion & rot );

public:
	/// Center of the bounding sphere
	Point3	m_center;

	/// Radius of the bounding sphere
	float	m_radius;
};

// 
// Inline
//

//
// Bound::transform
//
inline void Bound::transform( const Point3 & translation )
{
	m_center += translation;
}

} // Katana

#endif // _BOUND_H