/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		box.h
	Author:		Eric Bryant

	Axis Aligned Bounding Box
	TODO: Object Aligned Bounding Box
*/

#ifndef _BOX_H
#define _BOX_H

namespace Katana
{

// Forward Declarations
class Bound;

/// Useful for octree construction, these are the octants
/// of an octree
enum Octant
{
	OCTANT_TOP_LEFT_FRONT,
	OCTANT_TOP_LEFT_BACK,
	OCTANT_TOP_RIGHT_BACK,
	OCTANT_TOP_RIGHT_FRONT,
	OCTANT_BOTTOM_LEFT_FRONT,
	OCTANT_BOTTOM_LEFT_BACK,
	OCTANT_BOTTOM_RIGHT_BACK,
	OCTANT_BOTTOM_RIGHT_FRONT,
	MAX_OCTANTS
};

///
/// AxisAlignedBox
/// A simple Axis-Aligned Bounding Box which is represented
/// by a minimum point and a maximum point which are the
/// extremities of the bounding box. The axis are aligned to
/// the world axis (unlike an OBB which can have an arbitrary
/// orientation.
///
class AxisAlignedBox
{
public:
	/// Default Constructor (which creates a zero area bounding box)
	AxisAlignedBox();

	/// Constructor which takes a minimum and maximum
	AxisAlignedBox( const Point3 & min, const Point3 & max );

	/// Constructor which takes a bound (sphere)
	AxisAlignedBox( const Bound & bounds );

	/// Copy Constructor
	AxisAlignedBox( const AxisAlignedBox & aabb );

	/// Sets the minimum and maximum coordinates
	void setMinMax( const Point3 & min, const Point3 & max );

	/// Sets the extents of the AABB given a center point and extents
	void setExtents( const Point3 & center, const Point3 & extents );

	/// Gets the center of the AABB
	Point3 getCenter() const;

	/// Gets the extents of the AABB
	Point3 getExtents() const;

	/// Operator equal overloads
	bool operator==( const AxisAlignedBox & aabb );

	/// Expands the AABB by another AABB
	void expand( const AxisAlignedBox & aabb );

	/// Given a pointer to an array of points, it will fill it with the corners
	/// of this AABB. It assumes the corners array has enough space for 8 points
	void getCorners( Point3 * corners );

	/// Useful for the construction of an octree, given an octant, it will
	/// return a AABB.
	AxisAlignedBox getOctant( Octant octant );

public:
	Point3 m_minimum, m_maximum; /// Extents of the bounding box
};

//
// Inline
//

//
// AxisAlignedBox::constructors
//
inline AxisAlignedBox::AxisAlignedBox()
{}

inline AxisAlignedBox::AxisAlignedBox( const Point3 & min, const Point3 & max ) :
	m_minimum( min ), m_maximum( max )
{}

inline AxisAlignedBox::AxisAlignedBox( const Bound & bound )
{
	m_minimum = bound.m_center - Point3( bound.m_radius, bound.m_radius, bound.m_radius );
	m_maximum = bound.m_center - Point3( bound.m_radius, bound.m_radius, bound.m_radius );
}

inline AxisAlignedBox::AxisAlignedBox( const AxisAlignedBox & aabb ) :
	m_minimum( aabb.m_minimum ), m_maximum( aabb.m_maximum )
{}

//
// AxisAlignedBox::setMinMax
//
inline void AxisAlignedBox::setMinMax( const Point3 & min, const Point3 & max )
{ m_minimum = min; m_maximum = max; }

//
// AxisAlignedBox::setExtents
//
inline void AxisAlignedBox::setExtents( const Point3 & center, const Point3 & extents )
{
	m_minimum = center - extents;
	m_maximum = center + extents;
}

//
// AxisAlignedBox::getCenter
//
inline Point3 AxisAlignedBox::getCenter() const
{
	return ( m_minimum + m_maximum ) * 0.5f;
}

//
// AxisAlignedBox::getExtents
//
inline Point3 AxisAlignedBox::getExtents() const
{
	return ( m_maximum - m_minimum ) * 0.5f;
}

//
// AxisAlignedBox::operator==
//
inline bool AxisAlignedBox::operator==( const AxisAlignedBox & aabb )
{
	return ( m_minimum == aabb.m_minimum ) && ( m_maximum == aabb.m_maximum );
}

//
// AxisAlignedBox::expand
//
inline void AxisAlignedBox::expand( const AxisAlignedBox & aabb )
{
	if ( aabb.m_minimum.x < m_minimum.x ) m_minimum.x = aabb.m_minimum.x;
	if ( aabb.m_maximum.x > m_maximum.x ) m_maximum.x = aabb.m_maximum.x;

	if ( aabb.m_minimum.y < m_minimum.y ) m_minimum.y = aabb.m_minimum.y;
	if ( aabb.m_maximum.y > m_maximum.y ) m_maximum.y = aabb.m_maximum.y;

	if ( aabb.m_minimum.z < m_minimum.z ) m_minimum.z = aabb.m_minimum.z;
	if ( aabb.m_maximum.z > m_maximum.z ) m_maximum.z = aabb.m_maximum.z;
}

//
// AxisAlignedBox::getCorners
//
inline void AxisAlignedBox::getCorners( Point3 * corners )
{
	corners[0] = m_minimum;
	corners[1] = Point3(m_minimum.x, m_maximum.y, m_minimum.z);
	corners[2] = Point3(m_maximum.x, m_maximum.y, m_minimum.z);
	corners[3] = Point3(m_maximum.x, m_minimum.y, m_minimum.z);
	corners[4] = m_maximum;
	corners[5] = Point3(m_minimum.x, m_maximum.y, m_maximum.z);
	corners[6] = Point3(m_minimum.x, m_minimum.y, m_maximum.z);
	corners[7] = Point3(m_maximum.x, m_minimum.y, m_maximum.z);
}

} // Katana

#endif // _BOX_H