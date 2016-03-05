/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		intersect.cpp
	Author:		Eric Bryant
	Credits:	Portions of the code are credited to Magic Software and
				Game Gems I, Octree Construction

	Intersection testing functions
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "intersect.h"
#include <math.h>

// --------------------------------------------------------------
// Macros
// --------------------------------------------------------------

#define MIN3(a,b,c) ((((a)<(b))&&((a)<(c))) ? (a) : (((b)<(c)) ? (b) : (c)))
#define MAX3(a,b,c) ((((a)>(b))&&((a)>(c))) ? (a) : (((b)>(c)) ? (b) : (c)))
#define SIGN3( A )	(((A).x < EPSILSON) ? 4 : 0 | ((A).x > -EPSILSON) ? 32 : 0 | \
					((A).y < EPSILSON) ? 2 : 0 | ((A).y > -EPSILSON) ? 16 : 0 | \
					((A).z < EPSILSON) ? 1 : 0 | ((A).z > -EPSILSON) ? 8 : 0)

// --------------------------------------------------------------
// Local Functions
// --------------------------------------------------------------

void projectTriangle (const Point3 & rkD,
					  const Point3 & vert0, const Point3 & vert1, const Point3 & vert2, float & rfMin, float & rfMax)
{
	rfMin = rkD.getDot( vert0 );
	rfMax = rfMin;

	float fDot = rkD.getDot( vert1 );
	if ( fDot < rfMin )
		rfMin = fDot;
	else if ( fDot > rfMax )
		rfMax = fDot;

	fDot = rkD.getDot( vert2 );
	if ( fDot < rfMin )
		rfMin = fDot;
	else if ( fDot > rfMax )
		rfMax = fDot;
}


void projectBox (const Point3 & rkD, const AxisAlignedBox & rkBox,
				 float & rfMin, float & rfMax)
{
	float fDdC = rkD.getDot( rkBox.getCenter() );
	float fR = rkBox.getExtents().getDot( Point3( 1, 1, 1 ) );
	rfMin = fDdC - fR;
	rfMax = fDdC + fR;
}

// --------------------------------------------------------------
// kmath
// --------------------------------------------------------------

//
// Box vs. Point Intersection
//
bool testIntersect( AxisAlignedBox & aabb, Point3 & point )
{
	if ( point >= aabb.m_minimum && point <= aabb.m_maximum )
		return true;
	else
		return false;
}

//
// Box vs. Box Intersection
//
bool testIntersect( AxisAlignedBox & a, AxisAlignedBox & b )
{
	if ( a.m_maximum.x < b.m_minimum.x )
		return false;
	if ( a.m_maximum.y < b.m_minimum.y )
		return false;
	if ( a.m_maximum.z < b.m_minimum.z )
		return false;
	if ( a.m_minimum.x > b.m_maximum.x )
		return false;
	if ( a.m_minimum.y > b.m_maximum.y )
		return false;
	if ( a.m_minimum.z > b.m_maximum.z )
		return false;

	return true;
}

//
// Sphere vs. Point Intersection
//
bool kmath::testIntersect( const Bound & bound, const Point3 & point )
{
	float length = ( point - bound.m_center ).getLength();
	return sqrt( length ) < bound.m_radius;
}

//
// Sphere vs. Triangle Intersection
//
bool kmath::testIntersect( const Bound & bound, const Point3 & vert0, const Point3 & vert1, const Point3 & vert2 )
{
	// Early exit if one of the vertices is inside the sphere
	Point3 kDiff = vert2 - bound.getCenter();
	float fC = kDiff.getSqrLength();
	if(fC <= bound.getRadius())	return true;

	kDiff = vert1 - bound.getCenter();
	fC = kDiff.getSqrLength();
	if(fC <= bound.getRadius())	return true;

	kDiff = vert0 - bound.getCenter();
	fC = kDiff.getSqrLength();
	if(fC <= bound.getRadius())	return true;

	// Else do the full distance test
	Point3 TriEdge0	= vert1 - vert0;
	Point3 TriEdge1	= vert2 - vert0;

	//Point kDiff	= vert0 - mCenter;
	float fA00	= TriEdge0.getSqrLength();
	float fA01	= TriEdge0.getDot( TriEdge1 );
	float fA11	= TriEdge1.getSqrLength();
	float fB0	= kDiff.getDot( TriEdge0 );
	float fB1	= kDiff.getDot( TriEdge1 );
	//float fC	= kDiff.SquareMagnitude();
	float fDet	= kmath::fabs(fA00*fA11 - fA01*fA01);
	float u		= fA01*fB1-fA11*fB0;
	float v		= fA01*fB0-fA00*fB1;
	float SqrDist;

	if(u + v <= fDet)
	{
		if(u < 0.0f)
		{
			if(v < 0.0f)  // region 4
			{
				if(fB0 < 0.0f)
				{
					//					v = 0.0f;
					if(-fB0>=fA00)			{ /*u = 1.0f;*/		SqrDist = fA00+2.0f*fB0+fC;	}
					else					{ u = -fB0/fA00;	SqrDist = fB0*u+fC;			}
				}
				else
				{
					//					u = 0.0f;
					if(fB1>=0.0f)			{ /*v = 0.0f;*/		SqrDist = fC;				}
					else if(-fB1>=fA11)		{ /*v = 1.0f;*/		SqrDist = fA11+2.0f*fB1+fC;	}
					else					{ v = -fB1/fA11;	SqrDist = fB1*v+fC;			}
				}
			}
			else  // region 3
			{
				//				u = 0.0f;
				if(fB1>=0.0f)				{ /*v = 0.0f;*/		SqrDist = fC;				}
				else if(-fB1>=fA11)			{ /*v = 1.0f;*/		SqrDist = fA11+2.0f*fB1+fC;	}
				else						{ v = -fB1/fA11;	SqrDist = fB1*v+fC;			}
			}
		}
		else if(v < 0.0f)  // region 5
		{
			//			v = 0.0f;
			if(fB0>=0.0f)					{ /*u = 0.0f;*/		SqrDist = fC;				}
			else if(-fB0>=fA00)				{ /*u = 1.0f;*/		SqrDist = fA00+2.0f*fB0+fC;	}
			else							{ u = -fB0/fA00;	SqrDist = fB0*u+fC;			}
		}
		else  // region 0
		{
			// minimum at interior point
			if(fDet==0.0f)
			{
				//				u = 0.0f;
				//				v = 0.0f;
				SqrDist = (float)unsigned long(-1);
			}
			else
			{
				float fInvDet = 1.0f/fDet;
				u *= fInvDet;
				v *= fInvDet;
				SqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
			}
		}
	}
	else
	{
		float fTmp0, fTmp1, fNumer, fDenom;

		if(u < 0.0f)  // region 2
		{
			fTmp0 = fA01 + fB0;
			fTmp1 = fA11 + fB1;
			if(fTmp1 > fTmp0)
			{
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00-2.0f*fA01+fA11;
				if(fNumer >= fDenom)
				{
					//					u = 1.0f;
					//					v = 0.0f;
					SqrDist = fA00+2.0f*fB0+fC;
				}
				else
				{
					u = fNumer/fDenom;
					v = 1.0f - u;
					SqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
				}
			}
			else
			{
				//				u = 0.0f;
				if(fTmp1 <= 0.0f)		{ /*v = 1.0f;*/		SqrDist = fA11+2.0f*fB1+fC;	}
				else if(fB1 >= 0.0f)	{ /*v = 0.0f;*/		SqrDist = fC;				}
				else					{ v = -fB1/fA11;	SqrDist = fB1*v+fC;			}
			}
		}
		else if(v < 0.0f)  // region 6
		{
			fTmp0 = fA01 + fB1;
			fTmp1 = fA00 + fB0;
			if(fTmp1 > fTmp0)
			{
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00-2.0f*fA01+fA11;
				if(fNumer >= fDenom)
				{
					//					v = 1.0f;
					//					u = 0.0f;
					SqrDist = fA11+2.0f*fB1+fC;
				}
				else
				{
					v = fNumer/fDenom;
					u = 1.0f - v;
					SqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
				}
			}
			else
			{
				//				v = 0.0f;
				if(fTmp1 <= 0.0f)		{ /*u = 1.0f;*/		SqrDist = fA00+2.0f*fB0+fC;	}
				else if(fB0 >= 0.0f)	{ /*u = 0.0f;*/		SqrDist = fC;				}
				else					{ u = -fB0/fA00;	SqrDist = fB0*u+fC;			}
			}
		}
		else  // region 1
		{
			fNumer = fA11 + fB1 - fA01 - fB0;
			if(fNumer <= 0.0f)
			{
				//				u = 0.0f;
				//				v = 1.0f;
				SqrDist = fA11+2.0f*fB1+fC;
			}
			else
			{
				fDenom = fA00-2.0f*fA01+fA11;
				if(fNumer >= fDenom)
				{
					//					u = 1.0f;
					//					v = 0.0f;
					SqrDist = fA00+2.0f*fB0+fC;
				}
				else
				{
					u = fNumer/fDenom;
					v = 1.0f - u;
					SqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
				}
			}
		}
	}

	return kmath::fabs(SqrDist) < bound.getRadius();
}

//
// Axis Aligned Box vs. Triangle Intersection
//
bool kmath::testIntersect( const AxisAlignedBox & aabb, const Point3 & vert0, const Point3 & vert1, const Point3 & vert2 )
{
	float fMin0, fMax0, fMin1, fMax1;
	Point3 kD, akE[3];

	// Constants
	const float aabbExtents[3] = { aabb.getExtents().x, aabb.getExtents().y, aabb.getExtents().z };
	const Point3 AABBaxis[3] = { Point3( 1, 0, 0 ), Point3( 0, 1, 0 ), Point3( 0, 0, 1 ) };

	// test direction of triangle normal
	akE[0] = vert1 - vert0;
	akE[1] = vert2 - vert0;
	kD = akE[0].getCross( akE[1] );
	fMin0 = kD.getDot( vert0 );
	fMax0 = fMin0;
	projectBox( kD, aabb, fMin1, fMax1 );
	if ( fMax1 < fMin0 || fMax0 < fMin1 )
		return false;

	// test direction of box faces
	for (int i = 0; i < 3; i++)
	{
		kD = AABBaxis[ i ];
		projectTriangle( kD, vert0, vert1, vert2, fMin0, fMax0 );
		float fDdC = kD.getDot( aabb.getCenter() );
		fMin1 = fDdC - aabbExtents[ i ];
		fMax1 = fDdC + aabbExtents[ i ];
		if ( fMax1 < fMin0 || fMax0 < fMin1 )
			return false;
	}

	// test direction of triangle-box edge cross products
	akE[2] = akE[1] - akE[0];
	for (int i0 = 0; i0 < 3; i0++)
	{
		for (int i1 = 0; i1 < 3; i1++)
		{
			kD = akE[i0].getCross( AABBaxis[ i1 ] );
			projectTriangle( kD, vert0, vert1, vert2, fMin0, fMax0 );
			projectBox( kD, aabb, fMin1, fMax1 );
			if ( fMax1 < fMin0 || fMax0 < fMin1 )
				return false;
		}
	}

	return true;
}

//
// Point vs. Triangle Intersection
//
bool kmath::testIntersect( const Point3 & point, const Point3 & vert0, const Point3 & vert1, const Point3 & vert2 )
{
	//	First, a quick bounding-box test:                               
	//  If P is outside triangle's Bounding Box, there cannot be an intersection. 
	if (point.x > MAX3(vert0.x, vert1.x, vert2.x)) return false;
	if (point.y > MAX3(vert0.y, vert1.y, vert2.y)) return false;
	if (point.z > MAX3(vert0.z, vert1.z, vert2.z)) return false;
	if (point.x < MIN3(vert0.x, vert1.x, vert2.x)) return false;
	if (point.y < MIN3(vert0.y, vert1.y, vert2.y)) return false;
	if (point.z < MIN3(vert0.z, vert1.z, vert2.z)) return false;

	//	For each triangle side, make a vector out of it by subtracting vertexes; 
	//	make another vector from one vertex to point P.                          
	//  The crossproduct of these two vectors is orthogonal to both and the      
	//  signs of its X,Y,Z components indicate whether P was to the inside or    
	//  to the outside of this triangle side.
	Point3 vert12 = vert0 - vert1;
	Point3 vert1h = vert0 - point;
	Point3 cross12_1p = vert12.getCross( vert1h );
	int sign12 = SIGN3(cross12_1p);      /* Extract X,Y,Z signs as 0..7 or 0...63 integer */

	Point3 vert23 = vert1 - vert2;
	Point3 vert2h = vert1 - point;
	Point3 cross23_2p = vert23.getCross( vert2h );
	int	sign23 = SIGN3(cross23_2p);

	Point3 vert31 = vert2 - vert0;
	Point3 vert3h = vert2 - point;
	Point3 cross31_3p = vert31.getCross( vert3h );
	int	sign31 = SIGN3(cross31_3p);

	//	If all three crossproduct vectors agree in their component signs,
	//  then the point must be inside all three.                           
	//  P cannot be OUTSIDE all three sides simultaneously.                
	return (((sign12 & sign23 & sign31) == 0) ? false : true);
}

//
// Plane vs Sphere Intersection
//
bool kmath::testIntersect( const Plane & plane, const Bound & bound )
{
	return plane.distance( bound.getCenter() ) < bound.getRadius();
}

//
// Plane vs. Axis Aligned Box Intersection
//
bool kmath::testIntersect( const Plane & plane, const AxisAlignedBox & aabb )
{
	const Point3 vExtents = aabb.getExtents();
	const float fRadius = kmath::fabs( vExtents.x * plane.m_normal.x ) + 
						  kmath::fabs( vExtents.y * plane.m_normal.y ) + 
						  kmath::fabs( plane.m_normal.z );
	const float fPseudoDistance = plane.distance( aabb.getCenter() );

	return kmath::fabs( fPseudoDistance ) <= fRadius;
}