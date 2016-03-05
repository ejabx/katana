/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		math_script.cpp
	Author:		Eric Bryant

	Registers all classes in math library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>
#include <luabind/return_reference_to_policy.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "scriptengine.h"

// --------------------------------------------------------------------
// Scriptable Functions
// --------------------------------------------------------------------

Point3 & getMatrixAt( Matrix4 & m )		{ return m.at; }
Point3 & getMatrixUp( Matrix4 & m )		{ return m.up; }
Point3 & getMatrixRight( Matrix4 & m )	{ return m.right; }
Point3 & getMatrixPos( Matrix4 & m )	{ return m.pos; }

// --------------------------------------------------------------------
// Math_Register
// --------------------------------------------------------------------
bool Math_Register( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
	[
		// ColorA Registration
		class_<ColorA>( "Color" )
			.def( constructor<>() )
			.def( constructor<float,float,float,float>() )
			.def( constructor<const ColorA &>() )
			.def_readwrite( "r", &ColorA::r )
			.def_readwrite( "g", &ColorA::g )
			.def_readwrite( "b", &ColorA::b )
			.def_readwrite( "a", &ColorA::a )
		,
		// Point2 Registration
		class_<Point2>( "Point2" )
			.def( constructor<>() )
			.def( constructor<float,float>() )
			.def( constructor<const Point2 &>() )
			.def_readwrite( "x", &Point2::x )
			.def_readwrite( "y", &Point2::y )
			.def( self + other<const Point2>() )
			.def( self - other<const Point2>() )
			.def( self * other<const Point2>() )
			.def( self * float() )
			.def( self == other<const Point2>() )
		,
		// Point3 Registration
		class_<Point3>( "Point3" )
			.def( constructor<>() )
			.def( constructor<float,float,float>() )
			.def( constructor<const Point3 &>() )
			.def_readwrite( "x", &Point3::x )
			.def_readwrite( "y", &Point3::y )
			.def_readwrite( "z", &Point3::z )
			.def( "length", &Point3::getLength )
			.def( "sqrlength", &Point3::getSqrLength )
			.def( "dot", &Point3::getDot )
			.def( "cross", &Point3::getCross )
			.def( self + other<const Point3>() )
			.def( self - other<const Point3>() )
			.def( self * other<const Point3>() )
			.def( self * float() )
			.def( self == other<const Point3>() )
		,
		// Matrix Registration
		class_<Matrix4>( "Matrix4" )
			.def( constructor<>() )
			.def( constructor<const Matrix4&>() )
			.def( "setIdentity", &Matrix4::setIdentity )
			.def( "setZero", &Matrix4::setZero )
			.def( "transpose", &Matrix4::transpose, return_reference_to( self ) )
			.def( "inverse", &Matrix4::inverse, return_reference_to( self ) )
			.def( "makeRotation", &Matrix4::makeRotation )
			.def( self * other<const Matrix4>() )
		,
		def( "getMatrixAt", &getMatrixAt ),
		def( "getMatrixUp", &getMatrixUp ),
		def( "getMatrixRight", &getMatrixRight ),
		def( "getMatrixPos", &getMatrixPos )
		,
		// Quaternion Registration
		class_<Quaternion>( "Quaternion" )
			.def( constructor<>() )
			.def( constructor<float,const Point3 &>() )
			.def( "rotate", &Quaternion::rotate )
			.def( "fromAngleAxis", &Quaternion::fromAngleAxis )
		,
		// Bound Registration
		class_<Bound>( "Bound" )
			.def( constructor<>() )
			.def( constructor<const Point3 &, float>() )
			.def( constructor<const Bound &>() )
			.def( "set", &Bound::set )
			.def( "setCenter", &Bound::setCenter )
			.def( "setRadius", &Bound::setRadius )
			.def( "getCenter", &Bound::getCenter )
			.def( "getRadius", &Bound::getRadius )
			.def( "expand", &Bound::expand )
			.def( "transform", (void(Bound::*)(const Matrix4 &))&Bound::transform )
			.def( "transform", (void(Bound::*)(const Point3 &))&Bound::transform )
			.def( "transform", (void(Bound::*)(const Point3 &, const Quaternion &))&Bound::transform )
		,
		// Plane Registration
		class_<Plane>( "Plane" )
			.def( constructor<>() )
			.def( constructor<const Point3&,float>() )
			.def( constructor<const Point3&,const Point3&,const Point3&>() )
			.def( constructor<const Point3&,const Point3&>() )
			.def( "set", (void(Plane::*)(const Point3 &, float))&Plane::set )
			.def( "set", (void(Plane::*)(float,float,float,float))&Plane::set )
			.def( "set", (void(Plane::*)(const Point3 &,const Point3 &))&Plane::set )
			.def( "set", (void(Plane::*)(const Point3 &,const Point3 &,const Point3&))&Plane::set )
			.def( "distance", &Plane::distance )
			.def( "shiftX", &Plane::shiftX )
			.def( "shiftY", &Plane::shiftY )
			.def( "shiftZ", &Plane::shiftZ )
			.def_readwrite( "normal", &Plane::m_normal )
			.def_readwrite( "constant", &Plane::m_constant )
			.enum_( "Frustum" )
			[
				value( "NEAR",					FRUSTUM_NEAR ),
				value( "FAR",					FRUSTUM_FAR ),
				value( "RIGHT",					FRUSTUM_RIGHT ),
				value( "LEFT",					FRUSTUM_LEFT ),
				value( "TOP",					FRUSTUM_TOP ),
				value( "BOTTOM",				FRUSTUM_BOTTOM ),
				value( "MAX_FRUSTUM_PLANES",	MAX_FRUSTUM_PLANES )
			]
	];

	return true;
}