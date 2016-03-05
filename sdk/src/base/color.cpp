/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		color.cpp
	Author:		Eric Bryant

	Color Information
*/

#include "color.h"
using namespace Katana;

//
// Constructor
//
ColorA::ColorA (float fR, float fG, float fB, float fA) :
	r(fR), g(fG), b(fB), a(fA)
{
}

//
// Copy Constructor
//
ColorA::ColorA (const ColorA& c) :
	r(c.r), g(c.g), b(c.b), a(c.a)
{
}