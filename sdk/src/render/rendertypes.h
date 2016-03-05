/*
	Katana Engine
	Copyright © 2003 Eric Bryant, Inc.

	File:		rendertypes.h
	Author:		Eric Bryant

	Common structures, enumerations used in the render library
*/

#ifndef _RENDERTYPES_H
#define _RENDERTYPES_H

namespace Katana
{

///
/// Buffer Types
///
/// Typedefed for the sake of clarity. Note they are
/// INTEGERS so you can OR them together.
///
typedef int BufferTypes;
enum
{
	VERTEX	=	(1 << 0),
	VERTEX_T =	(1 << 1),	// A vertex that is already transformed (it contains a w component)
	COLOR =		(1 << 2),
	INDEX =		(1 << 3),
	NORMALS =	(1 << 4),
	TEXTURE_0 = (1 << 5),
	TEXTURE_1 = (1 << 6),
	TEXTURE_2 = (1 << 7),
	TEXTURE_3 = (1 << 8),
	TANGENT_S = (1 << 9),
	TANGENT_T = (1 << 10),
	TANGENT_ST =(1 << 11),
	TANGENTS = (1 << 11) | (1 << 10) | (1 << 9),
};

/// BufferCreationFlags
/// This is for readability, we need type INTs so
/// we can combine flags via OR, enums won't work
typedef int BufferCreationFlags;

/// Enumeration of Buffer Creation Flags
enum
{
	STATIC =			(1 << 0),
	DYNAMIC =			(1 << 1),
	WRITE_ONLY =		(1 << 2),
	DONOTCLIP =			(1 << 3),
	POINT_SPRITES =		(1 << 4),
	PATCHES =			(1 << 5),
	INTERLEAVED_ARRAYS =(1 << 6),
};

/// PrimitiveType
/// Renderable primitive types
enum PrimitiveType
{
	POINTS = 1,
	LINES = 2,
	LINESTRIP = 3,
	TRIANGLE_LIST = 4,
	TRIANGLE_STRIP = 5,
	TRIANGLE_FAN = 6,
};

/// MatrixFunction
/// Enumeration of operations on matrix types
enum MatrixFunction
{
	MULTIPLY,
	STORE,
	PUSH,
	POP,
};

///
/// RenderInfo
///
struct RenderInfo
{
	enum
	{
		MAX_NAME_SIZE = 64,
		MAX_EXTENSION_SIZE = 2048,
	};

	enum DriverType
	{
		NULL_RENDER,		// Diagnostic, used to see if Application is CPU Bound
		OPENGL,
		DIRECTX8,
		DIRECTX9,
		SOFTWARE,
	};

	// These constant values are set by the renderer
	DriverType eDriver;
	int iMaximumTextureUnits;
	int iMaximumTexutreSize;
	int iMaximumHardwareLights;
	int iMaximumClipPlanes;

	bool bSupportCubeMaps;
	bool bSupportProjectedTextures;
	bool bUseReferenceDriver;

	char szDriverName[MAX_NAME_SIZE];
	char szDriverVersion[MAX_NAME_SIZE];
	char szVendorName[MAX_NAME_SIZE];
	char szExtensions[MAX_EXTENSION_SIZE];
	char szVertexShaderProfile[MAX_NAME_SIZE];
	char szPixelShaderProfile[MAX_NAME_SIZE];

	unsigned int uiMaxViewWidth;
	unsigned int uiMaxViewHeight;
	unsigned int uiMaxColorDepth;

	// These values are sent to the renderer from the client
	AppHandle		hApplication;
	WindowHandle	hMainWindow;

	unsigned int uiTargetWidth;
	unsigned int uiTargetHeight;
	unsigned int uiTargetColorDepth;

	// This value is read/write, you can set it before initialize or retrieve it
	bool bFullscreen;
};

/// Currently we support a maximum of 4 simultaneous textures in one pass
enum { MAX_TEXTURE_PASSES = 4 };

///
/// Vertex Shader Constant Types
///
enum VertexShaderConstants
{
	VERTEX_CONST_WORLDVIEWPROJ				= 0,
	VERTEX_CONST_WORLDVIEW					= 1,
	VERTEX_CONST_WORLDVIEW_IT				= 2,
	VERTEX_CONST_EYE_POSITION				= 3,
	VERTEX_CONST_LIGHT_POSITION				= 4,
	VERTEX_CONST_LIGHT_ATTENUATION			= 5,
//	VERTEX_CONST_PROJECTION_TEXTURE_MATRIX	= 6,

	MAX_DEFAULT_VERTEX_CONSTANTS,
};

///
/// Pixel Shader Constant Types
///
enum PixelShaderConstants
{
	PIXEL_CONST_MATERIAL					= 0,

	MAX_DEFAULT_PIXEL_CONSTANTS,
};

///
/// ShaderConstant
///
struct ShaderConstant
{
public:
	/// Accessors to the constant. Use these assignment operators to set values in the constants
	void operator=( const Point4 & p );
	void operator=( const ColorA & c );
	void operator=( const Matrix4 & m );

public:
	/// Constant Data
	union {
		float			matrix[4][4];
		float			scalar;
		float			vector[4];
	};

	// Size of the constant data
	int constSize;
};

//
// ShaderConstant::operator=*
//
inline void ShaderConstant::operator=( const Point4 & p ) { vector[0] = p.x; vector[1] = p.y; vector[2] = p.z; vector[3] = p.w; constSize = sizeof(Point4); }
inline void ShaderConstant::operator=( const ColorA & c ) { vector[0] = c.r; vector[1] = c.g; vector[2] = c.b; vector[3] = c.a; constSize = sizeof(ColorA); }
inline void ShaderConstant::operator=( const Matrix4 & m ) { memcpy( &matrix[0][0], &m.m[0][0], sizeof(m) ); constSize = sizeof(Matrix4); }

}; // Katana

#endif //  _RENDERTYPES_H