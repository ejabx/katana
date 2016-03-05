/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		light.cpp
	Author:		Eric Bryant

	Defines a light within the scene
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "light.h"

//
// RTTI Information
//
KIMPLEMENT_ROOT_RTTI( Light );
KIMPLEMENT_RTTI( AmbientLight, Light );
KIMPLEMENT_RTTI( DirectionLight, Light );
KIMPLEMENT_RTTI( PointLight, Light );
KIMPLEMENT_RTTI( SpotLight, PointLight );

//
// Constants
//
const ColorA Light::DEFAULT_AMBIENT						= ColorA( 1, 1, 1, 1 );
const ColorA Light::DEFAULT_DIFFUSE						= ColorA( 1, 1, 1, 1 );
const ColorA Light::DEFAULT_SPECULAR					= ColorA( 1, 1, 1, 1 );
const float  Light::DEFAULT_RANGE						= 30000;

const Point3 DirectionLight::DEFAULT_DIRECTION			= Point3( 0, 0, 1 );

const float PointLight::DEFAULT_CONSTANT_ATTENUATION	= 1;
const float PointLight::DEFAULT_LINEAR_ATTENUATION		= 0;
const float PointLight::DEFAULT_QUADRATIC_ATTENUATION	= 0;

const float SpotLight::DEFAULT_INNER_CONE_ANGLE			= kmath::toRadians( 45 );
const float SpotLight::DEFAULT_OUTER_CONE_ANGLE			= kmath::toRadians( 60 );
const float SpotLight::DEFAULT_FALLOFF					= 1;

// -------------------------------------------------------

//
// OnLoadStream
//
bool Light::OnLoadStream( kistream & istr )
{
	// Load the light information
	istr >> m_ambient;
	istr >> m_diffuse;
	istr >> m_specular;
	istr >> m_worldPosition;

	return true;
}

//
// OnSaveStream
//
bool Light::OnSaveStream( kostream & ostr ) const
{
	// Save the light information
	ostr << m_ambient;
	ostr << m_diffuse;
	ostr << m_specular;
	ostr << m_worldPosition;

	return true;
}

// -------------------------------------------------------

//
// OnLoadStream
//
bool AmbientLight::OnLoadStream( kistream & istr )
{
	return Light::OnLoadStream( istr );
}

//
// OnSaveStream
//
bool AmbientLight::OnSaveStream( kostream & ostr ) const
{
	return Light::OnSaveStream( ostr );
}

// -------------------------------------------------------

//
// OnLoadStream
//
bool DirectionLight::OnLoadStream( kistream & istr )
{
	Light::OnLoadStream( istr );

	// Load the light information
	istr >> m_direction;

	return true;
}

//
// OnSaveStream
//
bool DirectionLight::OnSaveStream( kostream & ostr ) const
{
	Light::OnSaveStream( ostr );

	// Save the light information
	ostr << m_direction;

	return true;
}

// -------------------------------------------------------

//
// OnLoadStream
//
bool PointLight::OnLoadStream( kistream & istr )
{
	Light::OnLoadStream( istr );

	// Load the light information
	istr >> m_constantAttenuation;
	istr >> m_linearAttenuation;
	istr >> m_quadraticAttenuation;

	return true;
}

//
// OnSaveStream
//
bool PointLight::OnSaveStream( kostream & ostr ) const
{
	Light::OnSaveStream( ostr );

	// Save the light information
	ostr << m_constantAttenuation;
	ostr << m_linearAttenuation;
	ostr << m_quadraticAttenuation;

	return true;
}

// -------------------------------------------------------

//
// OnLoadStream
//
bool SpotLight::OnLoadStream( kistream & istr )
{
	PointLight::OnLoadStream( istr );

	// Load the light information
	istr >> m_direction;
	istr >> m_innerCone;
	istr >> m_outerCone;
	istr >> m_falloff;

	return true;
}

//
// OnSaveStream
//
bool SpotLight::OnSaveStream( kostream & ostr ) const
{
	PointLight::OnSaveStream( ostr );

	// Save the light information
	ostr << m_direction;
	ostr << m_innerCone;
	ostr << m_outerCone;
	ostr << m_falloff;

	return true;
}