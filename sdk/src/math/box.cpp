/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		box.cpp
	Author:		Eric Bryant

	Axis Aligned Bounding Box
	TODO: Object Aligned Bounding Box
*/

#include "point.h"
#include "bound.h"
#include "box.h"
using namespace Katana;

//
// AxisAlignedBox::getOctant
//
AxisAlignedBox AxisAlignedBox::getOctant( Octant octant )
{
	const float width = getExtents().x * 2;
	const Point3 vExtents = Point3( getExtents().x * 0.5f, getExtents().x * 0.5f, getExtents().x * 0.5f );
	const Point3 vCenter = getCenter();
	Point3 vNewCenter;

	switch( octant )
	{
		case OCTANT_TOP_LEFT_FRONT:
			vNewCenter = Point3(vCenter.x - width/4, vCenter.y + width/4, vCenter.z + width/4);
			break;

		case OCTANT_TOP_LEFT_BACK:
			vNewCenter = Point3(vCenter.x - width/4, vCenter.y + width/4, vCenter.z - width/4);
			break;

		case OCTANT_TOP_RIGHT_BACK:
			vNewCenter = Point3(vCenter.x + width/4, vCenter.y + width/4, vCenter.z - width/4);
			break;

		case OCTANT_TOP_RIGHT_FRONT:
			vNewCenter = Point3(vCenter.x + width/4, vCenter.y + width/4, vCenter.z + width/4);
			break;

		case OCTANT_BOTTOM_LEFT_FRONT:
			vNewCenter = Point3(vCenter.x - width/4, vCenter.y - width/4, vCenter.z + width/4);
			break;

		case OCTANT_BOTTOM_LEFT_BACK:
			vNewCenter = Point3(vCenter.x - width/4, vCenter.y - width/4, vCenter.z - width/4);
			break;

		case OCTANT_BOTTOM_RIGHT_BACK:
			vNewCenter = Point3(vCenter.x + width/4, vCenter.y - width/4, vCenter.z - width/4);
			break;

		case OCTANT_BOTTOM_RIGHT_FRONT:
			vNewCenter = Point3(vCenter.x + width/4, vCenter.y - width/4, vCenter.z + width/4);
			break;
	}

	return AxisAlignedBox( vNewCenter - vExtents, vNewCenter + vExtents );
}