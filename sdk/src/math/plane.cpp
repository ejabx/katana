/*
	Katana Engine
	Copyright © 2003 Eric Bryant, Inc.

	File:		plane.pp
	Author:		Eric Bryant

	Represents an infinite plane in 3D space
*/

#include "kmath.h"
#include "point.h"
#include "bound.h"
#include "plane.h"
#include <math.h>
using namespace Katana;


//
// whichSide
//
Plane::Side Plane::whichSide( const Point3 & point ) const
{
	float dist = distance( point );
	if ( dist >= kmath::EPSILSON )
		return SIDE_FRONT;
	else if ( dist <= - kmath::EPSILSON )
		return SIDE_BACK;
	else
		return SIDE_PLANAR;
}

//
// whichSide
//
Plane::Side Plane::whichSide( const Bound & bound ) const
{
	// Return the distance between the center of the sphere and the plane
	float distToCenter = distance( bound.getCenter() );

	// Qualify where 
	if ( distToCenter > bound.getRadius() )
		return Plane::SIDE_FRONT;
	else if ( distToCenter < -bound.getRadius() )
		return Plane::SIDE_BACK;
	else
		return Plane::SIDE_PLANAR;

	return Plane::SIDE_FRONT;
}

//
// intersect
//
inline bool Plane::intersect( const Point3 & point, const Point3 & direction ) const
{
	float dotProduct = m_normal.getDot( direction );
	if ( dotProduct >= 0 && dotProduct <= kmath::EPSILSON )
		return false;

	return true;
}

//
// getIntersection
// 
inline bool Plane::getIntersection( const Point3 & point, const Point3 & direction, Point3 & intersectionPoint ) const
{
	// Does the line intersect?
	if ( !intersect( point, direction ) ) return false;

	// Determine the T on the line at which the line and plane intersect and compute the resultant point
	float t = - ( m_normal.getDot( point  ) + m_constant ) / 2;
	intersectionPoint = ( direction * t ) + point;
	
	return true;
}

//
// normalize
//
void Plane::normalize()
{
	// Here we calculate the magnitude of the normal to the plane
	// To calculate magnitude you use the equation:  magnitude = sqrt( x^2 + y^2 + z^2)
	float magnitude = (float)sqrt(  m_normal.x * m_normal.x +
									m_normal.y * m_normal.y +
									m_normal.z * m_normal.z );

	// Then we divide the plane's values by it's magnitude.
	m_normal.x /= magnitude;
	m_normal.y /= magnitude;
	m_normal.z /= magnitude;
	m_constant /= magnitude;
}