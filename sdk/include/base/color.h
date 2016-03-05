/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		color.h
	Author:		Eric Bryant

	Color Information
*/

#ifndef _COLOR_H
#define _COLOR_H

namespace Katana
{

enum
{
	r = 0,
	g = 1,
	b = 2,
	a = 3,
};

///
/// Color
///
class ColorA
{
public:
	/// Constructor
    ColorA (float fR = 0.0f, float fG = 0.0f, float fB = 0.0f, float fA = 0.0f);

	/// Copy Constructor
    ColorA (const ColorA& c);

	/// Assignment operators
	ColorA& operator= (const ColorA& c);

	/// Equity operator
	bool operator== (const ColorA& c) const;

	/// Nonequity operator
    bool operator!= (const ColorA& c) const;

	/// Addition overload
    ColorA operator+ (const ColorA& c) const;
    ColorA & operator+= (const ColorA& c);

	/// Subtraction overload
    ColorA operator- (const ColorA& c) const;
    ColorA & operator-= (const ColorA& c);

	/// Multiply overloads
    ColorA operator* (float fScalar) const;
    ColorA operator* (const ColorA& c) const;
    ColorA & operator*= (float fScalar);
    ColorA & operator*= (const ColorA& c);

	/// Negation
    ColorA operator- () const;
    friend ColorA operator* (float fScalar, const ColorA & c);

public:
	/// Color Information (Red,Green,Blue,Alpha)
	float r, g, b, a;
};

//
// CreateColor
//
inline unsigned long CreateColor( float r, float g, float b, float a )
{
	unsigned char cr = (unsigned char)r * 255;
	unsigned char cg = (unsigned char)g * 255;
	unsigned char cb = (unsigned char)b * 255;
	unsigned char ca = (unsigned char)a * 255;

	return (ca << 24 | cr << 16 | cg << 8 | cb );
}

//
// operator overloads
//

inline ColorA& ColorA::operator= (const ColorA& c)
{ 
	r = c.r; g = c.g; b = c.b; a = c.a; 
	return *this;
}

inline bool ColorA::operator== (const ColorA& c) const
{
	return ( r == c.r && b == c.b && g == c.g && a == c.a ); 
}

inline bool ColorA::operator!= (const ColorA& c) const 
{ 
	return !(*this == c); 
}

inline ColorA ColorA::operator+ (const ColorA& c) const
{
	return ColorA( r + c.r, g + c.g, b + c.b, a + c.a );
}

inline ColorA & ColorA::operator+= (const ColorA& c)
{
	r += c.r; g += c.g; b += c.b; a += c.a;
	return *this;
}

inline ColorA ColorA::operator- (const ColorA& c) const
{
	return ColorA( r - c.r, g - c.g, b - c.b, a - c.a );
}

inline ColorA & ColorA::operator-= (const ColorA& c)
{
	r -= c.r; g -= c.g; b -= c.b; a -= c.a;
	return *this;
}

inline ColorA ColorA::operator* (float fScalar) const
{
	return ColorA( r * fScalar, g * fScalar, b * fScalar, a * fScalar );
}

inline ColorA ColorA::operator* (const ColorA& c) const
{
	return ColorA( r * c.r, g * c.g, b * c.b, a * c.a );
}

inline ColorA & ColorA::operator*= (float fScalar)
{
	r *= fScalar; g *= fScalar; b *= fScalar; a *= fScalar;
	return *this;
}

inline ColorA & ColorA::operator*= (const ColorA& c)
{
	r *= c.r; g *= c.g; b *= c.b; a *= c.a;
	return *this;
}

inline ColorA ColorA::operator- () const
{
	return ColorA( 1 - r, 1 - g, 1 - b, 1 - a );
}

inline ColorA operator* (float fScalar, const ColorA & c)
{
	return ColorA( fScalar * c.r, fScalar * c.g, fScalar * c.b, fScalar * c. a );
}

}; // Katana

#endif // _COLOR_H