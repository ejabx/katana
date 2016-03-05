/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		material.cpp
	Author:		Eric Bryant

	Represents the visible surface properties of an object: ambient contribution, texture maps, etc
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "texture.h"
#include "shader.h"
#include "rendertypes.h"
#include "material.h"

// ------------------------------------------------------------------
// RTTI declaration
// ------------------------------------------------------------------

KIMPLEMENT_RTTI( Material, Streamable );

// ------------------------------------------------------------------

//
// OnLoadStream
//
bool Material::OnLoadStream( kistream & istr )
{
	// Load the material information
	istr >> matID;
	istr >> ambient;
	istr >> diffuse;
	istr >> specular;
	istr >> emissive;
	istr >> fOpacity;
	istr >> bAlphaTest;
	istr >> diffuseMap0;
	istr >> diffuseMap1;
	istr >> diffuseMap2;
	istr >> diffuseMap3;
	istr >> bumpMap;
	istr >> specularMap;
	istr >> opacityMap;
	istr >> shader;

	return true;
}

//
// OnSaveStream
//
bool Material::OnSaveStream( kostream & ostr ) const
{
	// Save the material information
	ostr << matID;
	ostr << ambient;
	ostr << diffuse;
	ostr << specular;
	ostr << emissive;
	ostr << fOpacity;
	ostr << bAlphaTest;
	ostr << diffuseMap0;
	ostr << diffuseMap1;
	ostr << diffuseMap2;
	ostr << diffuseMap3;
	ostr << bumpMap;
	ostr << specularMap;
	ostr << opacityMap;
	ostr << shader;

	return true;
}