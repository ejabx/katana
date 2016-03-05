/*
Katana Engine
Copyright © 2001 Eric Bryant, Inc.

File:		scene_script.cpp
Author:		Eric Bryant

Registers all classes in render library.
*/

#include "katana_core_includes.h"
#include <luabind/luabind.hpp>
#include <luabind/iterator_policy.hpp>

#include "katana_base_includes.h"
#include "luahelper.h"
#include "luabind_policy.h"
#include "render/texture.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include "render/material.h"
#include "scriptengine.h"

//
// Local Functions
//
void setGeometryVertices( Geometry * pGeom, const luabind::object & vertexTable )
{
	if ( vertexTable.is_valid() && vertexTable.type() == LUA_TTABLE )
	{
		if ( !pGeom->m_vertexBuffer ) pGeom->m_vertexBuffer.reset( new vector<float> );
		pGeom->m_vertexBuffer->clear();
		unsigned uiNewVertices = 0;
		for( luabind::object::array_iterator iter = vertexTable.abegin();
			iter != vertexTable.aend();
			iter++, uiNewVertices++ )
		{
			boost::optional<Point3> opt = luabind::object_cast_nothrow<Point3>( *iter );
			if ( opt )
			{
				Point3 * pt = opt.get();
				pGeom->m_vertexBuffer->push_back( pt->x );
				pGeom->m_vertexBuffer->push_back( pt->y );
				pGeom->m_vertexBuffer->push_back( pt->z );
			}
		}
		pGeom->m_vertexCount = uiNewVertices;
	}
}

luabind::object getGeometryVertices( Geometry * pGeom )
{
	luabind::object vertexTable = luabind::newtable( GLOBAL_LUA_STATE );
	if ( pGeom->m_vertexBuffer && pGeom->m_vertexBuffer->size() )
	{
		for( unsigned int vertexIndex = 0, tableIndex = 1;
			vertexIndex < pGeom->m_vertexBuffer->size();
			vertexIndex += 3 )
		{
			vertexTable[tableIndex] = Point3( pGeom->m_vertexBuffer->at(vertexIndex+0), 
											  pGeom->m_vertexBuffer->at(vertexIndex+1), 
											  pGeom->m_vertexBuffer->at(vertexIndex+2) );
			tableIndex++;
		}
	}

	return vertexTable;
}

void setGeometryNormals( Geometry * pGeom, const luabind::object & normalTable )
{
	if ( normalTable.is_valid() && normalTable.type() == LUA_TTABLE )
	{
		if ( !pGeom->m_normalBuffer ) pGeom->m_normalBuffer.reset( new vector<float> );
		pGeom->m_normalBuffer->clear();
		for( luabind::object::array_iterator iter = normalTable.abegin();
			iter != normalTable.aend();
			iter++ )
		{
			boost::optional<Point3> opt = luabind::object_cast_nothrow<Point3>( *iter );
			if ( opt )
			{
				Point3 * pt = opt.get();
				pGeom->m_normalBuffer->push_back( pt->x );
				pGeom->m_normalBuffer->push_back( pt->y );
				pGeom->m_normalBuffer->push_back( pt->z );
			}
		}
	}
}

luabind::object getGeometryNormals( Geometry * pGeom )
{
	luabind::object normalTable = luabind::newtable( GLOBAL_LUA_STATE );
	if ( pGeom->m_normalBuffer && pGeom->m_normalBuffer->size() )
	{
		for( unsigned int normalIndex = 0, tableIndex = 1;
			normalIndex < pGeom->m_normalBuffer->size();
			normalIndex += 3 )
		{
			normalTable[tableIndex] = Point3( pGeom->m_vertexBuffer->at(normalIndex+0), 
											  pGeom->m_vertexBuffer->at(normalIndex+1), 
											  pGeom->m_vertexBuffer->at(normalIndex+2) );
			tableIndex++;
		}
	}

	return normalTable;
}

void setGeometryIndices( Geometry * pGeom, const luabind::object & indexTable )
{
	if ( indexTable.is_valid() && indexTable.type() == LUA_TTABLE )
	{
		if ( !pGeom->m_indexBuffer ) pGeom->m_indexBuffer.reset( new vector<unsigned short> );
		unsigned int uiNewIndices = 0;
		pGeom->m_indexBuffer->clear();
		for( luabind::object::array_iterator iter = indexTable.abegin();
			iter != indexTable.aend();
			iter++, uiNewIndices++ )
		{
			boost::optional<unsigned short> oindex = luabind::object_cast_nothrow<unsigned short>( *iter );
			if ( oindex )
				pGeom->m_indexBuffer->push_back( *oindex.get() );
		}
		pGeom->m_indexCount = uiNewIndices;
	}
}

luabind::object getGeometryIndices( Geometry * pGeom )
{
	luabind::object indexTable = luabind::newtable( GLOBAL_LUA_STATE );
	if ( pGeom->m_indexBuffer && pGeom->m_indexBuffer->size() )
	{
		for( unsigned int index = 0, tableIndex = 1;
			index < pGeom->m_indexBuffer->size();
			index += 1 )
		{
			indexTable[tableIndex] = pGeom->m_indexBuffer->at( index );
			tableIndex++;
		}
	}

	return indexTable;
}

void setGeometryTexture0( Geometry * pGeom, const luabind::object & texture0Table )
{
	if ( texture0Table.is_valid() && texture0Table.type() == LUA_TTABLE )
	{
		if ( !pGeom->m_texture0Buffer ) pGeom->m_texture0Buffer.reset( new vector<float> );
		pGeom->m_texture0Buffer->clear();
		for( luabind::object::array_iterator iter = texture0Table.abegin();
			iter != texture0Table.aend();
			iter++ )
		{
			boost::optional<Point2> opt = luabind::object_cast_nothrow<Point2>( *iter );
			if ( opt )
			{
				Point2 * pt = opt.get();
				pGeom->m_texture0Buffer->push_back( pt->x );
				pGeom->m_texture0Buffer->push_back( pt->y );
			}
		}
	}
}

luabind::object getGeometryTexture0( Geometry * pGeom )
{
	luabind::object texture0Table = luabind::newtable( GLOBAL_LUA_STATE );
	if ( pGeom->m_texture0Buffer && pGeom->m_texture0Buffer->size() )
	{
		for( unsigned int textureIndex = 0, tableIndex = 1;
			textureIndex < pGeom->m_texture0Buffer->size();
			textureIndex += 2 )
		{
			texture0Table[tableIndex] = Point2( pGeom->m_texture0Buffer->at(textureIndex+0), 
											  pGeom->m_texture0Buffer->at(textureIndex+1) );
			tableIndex++;
		}
	}

	return texture0Table;
}

void setGeometryTexture1( Geometry * pGeom, const luabind::object & texture1Table )
{
	if ( texture1Table.is_valid() && texture1Table.type() == LUA_TTABLE )
	{
		if ( !pGeom->m_texture1Buffer ) pGeom->m_texture1Buffer.reset( new vector<float> );
		pGeom->m_texture1Buffer->clear();
		for( luabind::object::array_iterator iter = texture1Table.abegin();
			iter != texture1Table.aend();
			iter++ )
		{
			boost::optional<Point2> opt = luabind::object_cast_nothrow<Point2>( *iter );
			if ( opt )
			{
				Point2 * pt = opt.get();
				pGeom->m_texture1Buffer->push_back( pt->x );
				pGeom->m_texture1Buffer->push_back( pt->y );
			}
		}
	}
}

luabind::object getGeometryTexture1( Geometry * pGeom )
{
	luabind::object texture1Table = luabind::newtable( GLOBAL_LUA_STATE );
	if ( pGeom->m_texture1Buffer && pGeom->m_texture1Buffer->size() )
	{
		for( unsigned int textureIndex = 0, tableIndex = 1;
			textureIndex < pGeom->m_texture1Buffer->size();
			textureIndex += 2 )
		{
			texture1Table[tableIndex] = Point2( pGeom->m_texture1Buffer->at(textureIndex+0), 
												pGeom->m_texture1Buffer->at(textureIndex+1) );
			tableIndex++;
		}
	}

	return texture1Table;
}

void setGeometryColor( Geometry * pGeom, const luabind::object & colorTable )
{
	if ( colorTable.is_valid() && colorTable.type() == LUA_TTABLE )
	{
		if ( !pGeom->m_colorBuffer ) pGeom->m_colorBuffer.reset( new vector<float> );
		pGeom->m_colorBuffer->clear();
		for( luabind::object::array_iterator iter = colorTable.abegin();
			iter != colorTable.aend();
			iter++ )
		{
			boost::optional<ColorA> opt = luabind::object_cast_nothrow<ColorA>( *iter );
			if ( opt )
			{
				ColorA * pt = opt.get();
				pGeom->m_colorBuffer->push_back( pt->r );
				pGeom->m_colorBuffer->push_back( pt->g );
				pGeom->m_colorBuffer->push_back( pt->b );
				pGeom->m_colorBuffer->push_back( pt->a );
			}
		}
	}
}

luabind::object getGeometryColor( Geometry * pGeom )
{
	luabind::object colorTable = luabind::newtable( GLOBAL_LUA_STATE );
	if ( pGeom->m_colorBuffer && pGeom->m_colorBuffer->size() )
	{
		for( unsigned int colorIndex = 0, tableIndex = 1;
			colorIndex < pGeom->m_colorBuffer->size();
			colorIndex += 2 )
		{
			colorTable[tableIndex] = ColorA( pGeom->m_texture1Buffer->at(colorIndex+0), 
											 pGeom->m_texture1Buffer->at(colorIndex+1),
											 pGeom->m_texture1Buffer->at(colorIndex+2), 
											 pGeom->m_texture1Buffer->at(colorIndex+3) );
			tableIndex++;
		}
	}

	return colorTable;
}

void setMaterialShader( Material * pMaterial, shared_ptr<Shader> shader )
{
	pMaterial->shader = shader;
}

shared_ptr<Shader> getMaterialShader( Material * pMaterial )
{
	return pMaterial->shader;
}

// ------------------------------------------------------

//
// Geometry Registration
//
bool Geometry::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
		[
			class_< Geometry, shared_ptr<Geometry> >( "Geometry" )
			.def( constructor<>() )
			.def_readwrite( "primitiveCount",		&Geometry::m_primitiveCount )
			.def_readwrite( "vertexCount",			&Geometry::m_vertexCount )
			.def_readwrite( "indexCount",			&Geometry::m_indexCount )
			.def_readwrite( "triangleStripCount",	&Geometry::m_triangleStripCount )
			.def_readwrite( "primitiveType",		&Geometry::m_primitiveType )
			.def_readwrite( "enableBuffers",		&Geometry::m_enabledBuffers )
			.def( "setVertices", &setGeometryVertices ).property( "vertices", &getGeometryVertices )
			.def( "setNormals", &setGeometryNormals ).property( "normals", &getGeometryNormals )
			.def( "setIndices", &setGeometryIndices ).property( "indices", &getGeometryIndices )
			.def( "setTexture0", &setGeometryTexture0 ).property( "texture0", &getGeometryTexture0 )
			.def( "setTexture1", &setGeometryTexture1 ).property( "texture0", &getGeometryTexture1 )
			.def( "setColor", &setGeometryColor ).property( "color", &getGeometryColor )
			.enum_("primitiveType")
			[
				value( "POINTS", POINTS ),
				value( "LINES", LINES ),
				value( "LINESTRIP", LINESTRIP ),
				value( "TRIANGLE_LIST", TRIANGLE_LIST ),
				value( "TRIANGLE_STRIP", TRIANGLE_STRIP ),
				value( "TRIANGLE_FAN", TRIANGLE_FAN )
			]
			.enum_("bufferTypes")
				[
					value( "VERTEX", VERTEX ),
					value( "COLOR", COLOR ),
					value( "INDEX", INDEX ),
					value( "NORMALS", NORMALS ),
					value( "TEXTURE_0", TEXTURE_0 ),
					value( "TEXTURE_1", TEXTURE_1 ),
					value( "TEXTURE_2", TEXTURE_2 ),
					value( "TEXTURE_3", TEXTURE_3 ),
					value( "TANGENT_S", TANGENT_S ),
					value( "TANGENT_T", TANGENT_T ),
					value( "TANGENT_ST", TANGENT_ST ),
					value( "TANGENTS", TANGENTS )
				]
		];

	return true;
}

//
// Material Registration
//
bool Material::OnRegister( lua_State * env )
{
	REGISTER_SCRIPTING_GUARD();

	module( env )
		[
			class_< Material, shared_ptr<Material> >( "Material" )
			.def( constructor<>() )
			.def_readwrite( "ambient",		&Material::ambient )
			.def_readwrite( "diffuse",		&Material::diffuse )
			.def_readwrite( "specular",		&Material::specular )
			.def_readwrite( "opacity",		&Material::fOpacity )
			.def_readwrite( "alphaTest",	&Material::bAlphaTest )
			.def_readwrite( "diffuseMap0",	&Material::diffuseMap0 )
			.def_readwrite( "diffuseMap1",	&Material::diffuseMap1 )
			.def_readwrite( "diffuseMap2",	&Material::diffuseMap2 )
			.def_readwrite( "diffuseMap3",	&Material::diffuseMap3 )
			.def_readwrite( "bumpMap",		&Material::bumpMap )
			.def_readwrite( "specularMap",	&Material::specularMap )
			.def_readwrite( "opacityMap",	&Material::opacityMap )
			.def( "setShader", &setMaterialShader, shared_ptr_policy( _1 ) )
			.property( "shader", &getMaterialShader )
		];

	return true;
}