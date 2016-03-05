/*
	Katana Engine
	Copyright © 2003 Eric Bryant, Inc.

	File:		plane.h
	Author:		Eric Bryant

	Represents an infinite plane in 3D space
*/

#ifndef _PLANE_H
#define _PLANE_H

namespace Katana
{

//
// Forward Declarations
//
class Bound;

//
// FrustumPlanes
//
enum FrustumPlanes
{
	FRUSTUM_NEAR,
	FRUSTUM_FAR,
	FRUSTUM_RIGHT,
	FRUSTUM_LEFT,
	FRUSTUM_TOP,
	FRUSTUM_BOTTOM,
	MAX_FRUSTUM_PLANES,
};

///
/// Plane
///
class Plane
{
public:
	enum Side
	{
		SIDE_PLANAR,
		SIDE_FRONT,
		SIDE_BACK,
	};
public:
	/// Constructor
	Plane() : m_constant( 0 ) {}

	/// Constructor which takes a normal and a constant
	Plane( const Point3 & normal, float constant ) : m_normal( normal ), m_constant( constant )
		{}

	/// Constructor which takes three points on a coplanar triangle
	Plane( const Point3 & ptA, const Point3 & ptB, const Point3 & ptC )			{ set( ptA, ptB, ptC ); }

	/// Constructor which takes a normal and a point though which the plane passes through
	Plane( const Point3 & normal, const Point3 & point ) : m_normal( normal )	{ set( normal, point ); }

	/// Returns the distance from a point to the plane
	float distance( const Point3 & point ) const								{ return m_normal.getDot( point ) + m_constant; }

	/// Sets the plane given a normal and constant
	void set( const Point3 & normal, float constant )							{ m_normal = normal; m_constant = constant; }

	/// Sets the plane given a normal and constant as a series of floats
	void set( float x, float y, float z, float constant )						{ m_normal = Point3( x, y, z ); m_constant = constant; }

	/// Sets the plane given three points on a planar triangle
	void set( const Point3 & ptA, const Point3 & ptB, const Point3 & ptC )		{  m_normal = ( ptA - ptB ).getCross( ptB - ptC ); m_constant = m_normal.getDot( ptA ); }

	/// Sets the plane given a normal and a point though which the plane passes through
	void set( const Point3 & normal, const Point3 & point )			{ m_normal = normal; m_constant = - point.getDot( normal ); }

	/// Shifts the plane in the X direction
	void shiftX( float scalar )										{ m_constant -= scalar * m_normal.x; }

	/// Shifts the plane in the Y direction
	void shiftY( float scalar )										{ m_constant -= scalar * m_normal.y; }

	/// Shifts the plane in the Z direction
	void shiftZ( float scalar )										{ m_constant -= scalar * m_normal.z; }

	/// Returns which side a point lies on
	Side whichSide( const Point3 & point ) const;

	/// Returns which side a bound(ing) sphere lies on
	Side whichSide( const Bound & bound ) const;

	/// Returns whether a line intersects with the plane. The line is defined as a point and a direction.
	bool intersect( const Point3 & point, const Point3 & direction ) const;

	/// Returns the intersection point between a line and a plane (assuming they intersect
	bool getIntersection( const Point3 & point, const Point3 & direction, Point3 & intersectionPoint ) const;

	/// Normalizes the plane
	void normalize();

public:
	Point3	m_normal;
	float	m_constant;
};



} // Katana

#endif // _PLANE_H