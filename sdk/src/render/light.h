/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		light.h
	Author:		Eric Bryant

	Defines a light within the scene
*/

#ifndef _LIGHT_H
#define _LIGHT_H

namespace Katana
{

///
/// Light
/// Base interface for all lights
///
class Light : public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(Light);
	KDECLARE_SCRIPT;

public:
	/// This is the default ambient, diffuse, and specular colors (100% white)
	static const ColorA DEFAULT_AMBIENT;
	static const ColorA DEFAULT_DIFFUSE;
	static const ColorA DEFAULT_SPECULAR;

	/// This is the default light range (30000 world units, which should encompass the entire scene)
	static const float DEFAULT_RANGE;

public:
	/// Constructor
	Light::Light()
		: m_ambient( DEFAULT_AMBIENT )
		, m_diffuse( DEFAULT_DIFFUSE )
		, m_specular( DEFAULT_SPECULAR )
		, m_range( DEFAULT_RANGE )
		, m_moved( false )
	{}

	/// Sets the world position
	void setPosition( const Point3 & pos )		{ m_worldPosition = pos; m_moved = true; }

	/// Sets the spherical range this light can affect
	void setRange( float range )				{ m_range = range; }

	/// Sets the ambient color
	void setAmbient( const ColorA & ambient )	{ m_ambient = ambient; }

	/// Sets the diffuse color
	void setDiffuse( const ColorA & diffuse )	{ m_diffuse = diffuse; }

	/// Sets the specular color
	void setSpecular( const ColorA & specular )	{ m_specular = specular; }

	/// Returns the world position
	Point3 getPosition() const					{ return m_worldPosition; }

	/// Returns the range
	float getRange() const						{ return m_range; }

	/// Returns the ambient color
	ColorA getAmbient() const					{ return m_ambient; }
	
	/// Returns the diffuse color
	ColorA getDiffuse() const					{ return m_diffuse; }

	/// Returns the specular color
	ColorA getSpecular() const					{ return m_specular; }

	/// Sets whether the light has moved.
	/// Some dependent entities many need to do recalculations (shadows, for instance).
	bool setMoved( bool moved )					{ m_moved = moved; }

	/// Returns whether this light has been moved
	bool getMoved() const						{ return m_moved; }

protected:
	/// Light attributes
	ColorA	m_ambient;
	ColorA	m_diffuse;
	ColorA	m_specular;
	Point3	m_worldPosition;
	float	m_range;
	bool	m_moved;
};

KIMPLEMENT_STREAM( Light );
KIMPLEMENT_SCRIPT( Light );

///
/// AmbientLight
///
class AmbientLight : public Light
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(AmbientLight);
};

KIMPLEMENT_STREAM( AmbientLight );

///
/// DirectionLight
///
class DirectionLight : public Light
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(DirectionLight);

public:
	/// This is the default direction, which is into the positive Z direction (into the screen)
	static const Point3 DEFAULT_DIRECTION;

public:
	/// Constructor
	DirectionLight::DirectionLight()
		: m_direction( DEFAULT_DIRECTION )
	{}

	/// Sets the direction
	void setDirection( const Point3 & dir )		{ m_direction = dir; }

	/// Gets the direction
	Point3 getDirection() const					{ return m_direction; }

protected:
	/// Direction light attributes
	Point3 m_direction;
};

KIMPLEMENT_STREAM( DirectionLight );

///
/// PointLight
///
class PointLight : public Light
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(PointLight);

public:
	/// This is the default attenuation used by the constructor (constant is 1, others are set to 0)
	static const float DEFAULT_CONSTANT_ATTENUATION;
	static const float DEFAULT_LINEAR_ATTENUATION;
	static const float DEFAULT_QUADRATIC_ATTENUATION;

public:
	/// Constructor
	PointLight() :
		m_constantAttenuation( DEFAULT_CONSTANT_ATTENUATION ),
		m_linearAttenuation( DEFAULT_LINEAR_ATTENUATION ),
		m_quadraticAttenuation( DEFAULT_QUADRATIC_ATTENUATION ) {}

public:
	/// Sets the constant attenuation
	void setConstantAttenuation( float att )		{ m_constantAttenuation = att; }
	
	/// Sets the linear attenuation
	void setLinearAttenuation( float att )			{ m_linearAttenuation = att; }

	/// Sets the quadratic attenuation
	void setQuadraticAttenuation( float att )		{ m_quadraticAttenuation = att; }

	/// Gets the constant attenuation
	float getConstantAttenuation() const			{ return m_constantAttenuation; }
	
	/// Gets the linear attenuation
	float getLinearAttenuation() const				{ return m_linearAttenuation; }

	/// Gets the quadratic attenuation
	float getQuadraticAttenuation() const			{ return m_quadraticAttenuation; }

protected:
	/// Point light attributes
	float m_constantAttenuation;
	float m_linearAttenuation;
	float m_quadraticAttenuation;
};

KIMPLEMENT_STREAM( PointLight );

///
/// SpotLight
///
class SpotLight : public PointLight
{
	KDECLARE_RTTI;
	KDECLARE_STREAM(PointLight);

public:
	/// This is the default cone angles (45 degrees for the inner cone, 60 for the outer)
	static const float DEFAULT_INNER_CONE_ANGLE;
	static const float DEFAULT_OUTER_CONE_ANGLE;

	/// This is the default falloff range for the spotlight (1 world unit)
	static const float DEFAULT_FALLOFF;

public:
	/// Constructor
	SpotLight() : 
		m_innerCone(DEFAULT_INNER_CONE_ANGLE), 
		m_outerCone(DEFAULT_OUTER_CONE_ANGLE), 
		m_falloff( DEFAULT_FALLOFF ),
		m_direction( DirectionLight::DEFAULT_DIRECTION ) {}

	/// Sets the world direction
	void setDirection( const Point3 & dir )			{ m_direction = dir; }

	/// Sets the inner cone angle (in radians)
	void setInnerCone( float inner )				{ m_innerCone = inner; }

	/// Sets the outer cone angle (in radians)
	void setOuterCone( float outer )				{ m_outerCone = outer; }

	/// Sets the falloff curve between the inner and outer cones
	void setFalloff( float falloff )				{ m_falloff = falloff; }

	/// Gets the world direction
	Point3 getDirection() const						{ return m_direction; }

	/// Gets the inner cone angle (in radians)
	float getInnerCone() const						{ return m_innerCone; }

	/// Gets the outer cone angle (in radians)
	float getOuterCone() const						{ return m_outerCone; }

	/// Gets the falloff curve between the inner and outer cones
	float getFalloff() const						{ return m_falloff; }

protected:
	/// Spot light attributes
	Point3	m_direction;
	float	m_innerCone;
	float	m_outerCone;
	float	m_falloff;
};

KIMPLEMENT_STREAM( SpotLight );

}; // Katana

#endif // _LIGHT_H