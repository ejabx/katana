/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		material.h
	Author:		Eric Bryant

	Represents the visible surface properties of an object: ambient contribution, texture maps, etc
*/

#ifndef _MATERIAL_H
#define _MATERIAL_H

namespace Katana
{

// Forward declarations
class Texture;
class Shader;

///
/// MaterialID
/// Material Identifier. This is for grouping objects by material
/// types so they can be sorted before rendering. This is to reduce render state switching.
/// 
typedef int MaterialID;

///
/// Material
///
struct Material
	: public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( Material );
	KDECLARE_SCRIPT;

public:
	Material();				/// Constructor
	MaterialID	matID;		/// This is the material's unique id. This is for grouping objects by material
							/// types so they can be sorted before rendering. This is to reduce render state switching.

	ColorA	ambient;		/// Color which is used to modulate the ambient value
	ColorA	diffuse;		/// Color which is used to modulate the diffuse value
	ColorA	specular;		/// Color which is used to modulate the specular value
	ColorA	emissive;		/// Color which is used to modulate the emissive value
	float	fOpacity;		/// Scalar opacity value. 0 = transparent, 1 = opaque
	bool	bAlphaTest;		/// Does this material require alpha testing?

	shared_ptr<Texture> diffuseMap0;		/// The diffuse map (1 of 4). A material can have up to 4 diffuse maps because of multitexturing.
	shared_ptr<Texture> diffuseMap1;		/// The diffuse map (2 of 4). A material can have up to 4 diffuse maps because of multitexturing.
	shared_ptr<Texture> diffuseMap2;		/// The diffuse map (3 of 4). A material can have up to 4 diffuse maps because of multitexturing.
	shared_ptr<Texture> diffuseMap3;		/// The diffuse map (4 of 4). A material can have up to 4 diffuse maps because of multitexturing.
	shared_ptr<Texture> bumpMap;			/// The bump/normal map
	shared_ptr<Texture> specularMap;		/// Specular map
	shared_ptr<Texture> opacityMap;			/// Alpha test map

	shared_ptr<Shader> shader;				/// The shader associated with this material.
};

KIMPLEMENT_STREAM( Material );
KIMPLEMENT_SCRIPT( Material );

//
// Material::Constructor
//
inline Material::Material()
	: matID( 0 )
	, fOpacity( 1 )
	, bAlphaTest( false )
{
}

}; // Katana

#endif // _MATERIAL_H