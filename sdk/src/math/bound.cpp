/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		bound.cpp
	Author:		Eric Bryant

	Represents a bounding volume (used for culling).
*/

#include "point.h"
#include "matrix.h"
#include "quaternion.h"
#include "bound.h"
#include "box.h"
#include <math.h>
using namespace Katana;

//
// Constructor
//
Bound::Bound() 
	: m_center( 0.f, 0.f, 0.f )
	, m_radius( 0.f )
{
}

Bound::Bound(const Point3 & center, float radius) 
	: m_center( center )
	, m_radius( radius )
{
}

Bound::Bound(const Bound & bv) 
	: m_center( bv.m_center )
	, m_radius( bv.m_radius )
{
}

Bound::Bound( const AxisAlignedBox & aabb )
{
	m_center = aabb.getCenter();
	m_radius = aabb.getExtents().x * 0.5f;
}

//
// transform
//
void Bound::transform(const Point3 & translation, const Quaternion & rot )
{
	m_center += rot.rotate( translation );
}

void Bound::transform( const Matrix4 & transform )
{
	m_center *= transform;
}

//
// expand
//
void Bound::expand( const Bound & bv )
{
	Point3 centerDiff = bv.m_center - m_center;
	float radiusDiff = bv.m_radius - m_radius;
	float radiusDiffSqr = radiusDiff * radiusDiff;
	float lengthSqr = centerDiff.getSqrLength();

	if ( radiusDiffSqr >= lengthSqr )
	{
		if ( radiusDiff >= 0.0f )
			set( bv.m_center, bv.m_radius );
	}
	else
	{
		float l = (float)sqrt(lengthSqr);
		float t = (l + (bv.m_radius - m_radius)) / (2 * l);
		
		set( m_center + t * centerDiff, (1 + bv.m_radius + m_radius)/2 );
	}
}