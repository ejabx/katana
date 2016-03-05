/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8shadermanager.h
	Author:		Eric Bryant

	Concrete shader manager for DX8/CG Shaders
*/

#include <d3d8.h>
#include <d3dx8.h>
#include <Cg/Cg.h>
#include <Cg/Cgd3d8.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "base/comptr.h"
#include "rendertypes.h"
#include "texture.h"
#include "shader.h"
#include "render.h"
#include "rendertypes.h"
#include "renderstate.h"
#include "wireframestate.h"
#include "cullstate.h"
#include "multitexturestate.h"
#include "dx8shadermanager.h"

// Make sure client includes Cg libaries
#pragma comment(lib, "cg.lib")
#pragma comment(lib, "cgD3D8.lib")

//
// Local Variables
//
CGcontext 					g_cgContext;					// Context of the main Cg Compiler.
CGprogram					g_cgProgram;					// Current program

//
// Initialize
//
bool DX8ShaderManager::Initialize(IDirect3DDevice8 * pD3DDevice)
{
/*
	// Store the Device
	m_pD3DDevice = pD3DDevice;
	cgD3D8SetDevice( pD3DDevice );

	// Setup Cg Environment
	g_cgContext = cgCreateContext();
	if ( !g_cgContext )
		return false;
*/
	return true;
}

//
// Terminate
//
bool DX8ShaderManager::Terminate()
{
	// Shutdown Cg
//    cgDestroyContext( g_cgContext );

	return true;
}

//
// enableShader
//
bool DX8ShaderManager::enableShader( Shader & shader )
{
/*
	// Check whether we already attempted to compile the shader and it returned errors
	if ( shader.hasErrors() )
		return false;

	// Check to see whether the shader has already been compiled, and if not, we need to compile it.
	if ( !shader.isCompiled() )
		if ( !CompileShader( shader ) )
			return false;

	// Grab the shader program
	CGprogram program = (CGprogram)shader.getInstance();
	if ( !cgIsProgram( program ) || !cgD3D8IsProgramLoaded( program ) )
		return false;
	
	// Activate the program
	if ( FAILED( cgD3D8BindProgram( g_cgProgram = program ) ) )
		return false;
*/
	return true;
}

//
// disableShader
//
bool DX8ShaderManager::disableShader( Shader & shader )
{
/*
	if ( KIsDerivedFromClass<VertexShader>( &shader ) )
		m_pD3DDevice->SetVertexShader( NULL );
	if ( KIsDerivedFromClass<PixelShader>( &shader ) )
		m_pD3DDevice->SetPixelShader( NULL );
*/
	return true;
}

//
// DisableShaders
//
bool DX8ShaderManager::disableShaders()
{
//	m_pD3DDevice->SetVertexShader( NULL );
//	m_pD3DDevice->SetPixelShader( NULL );

	return true;
}

//
// setConstants
//
bool DX8ShaderManager::setConstants( std::vector<ShaderConstant> * vertexConstants, std::vector<ShaderConstant> * pixelConstants )
{
/*
	if ( vertexConstants )
	{
		for( unsigned int i = 0, constIndex = 0, constSize = 0; i < vertexConstants->size(); i++, constIndex += constSize )
		{
			constSize = (*vertexConstants)[i].constSize / sizeof(float) / sizeof(float);
			m_pD3DDevice->SetVertexShaderConstant( constIndex, &(*vertexConstants)[i], constSize );
		}
	}

	if ( pixelConstants )
	{
		for( unsigned int i = 0, constIndex = 0, constSize = 0; i < pixelConstants->size(); i++, constIndex += constSize )
		{
			constSize = (*pixelConstants)[i].constSize / sizeof(float) / sizeof(float);
			m_pD3DDevice->SetPixelShaderConstant( constIndex, &(*pixelConstants)[i], constSize );
		}
	}
*/
	return true;
}

//
// CompileShader
//
bool DX8ShaderManager::CompileShader( Shader & shader )
{
/*
	// Does the shader contain code
	if ( !shader.isLoaded() ) return false;

	// Log the compile
	KLOG( "Compiling shader: %s", shader.getShaderPath().c_str() );

	// Determine the whether we are compiling a vertex or pixel program
	CGprofile profileType = KIsDerivedFromClass<VertexShader>(&shader) ? CG_PROFILE_VS_1_1 : CG_PROFILE_PS_1_1;

	// Load the shader
	CGprogram program =	cgCreateProgram( g_cgContext,					// Cg Context
										 CG_SOURCE,						// Cg Program
										 shader.getCode().c_str(),		// Program Source
										 profileType,					// Profile type (NV_vertex_program or NV_fragment_program)
										 NULL,							// use default "main" entry
										 NULL );						// no arguments for compiler

	// Did we load sucessfully?
	if ( program == NULL )
	{
		// The shader failed. This is so we don't attempt to load the shader
		// again during the next enableShader().
		shader.compileFailed();

		// Return the error string
		const char * errorString = cgGetLastListing( g_cgContext );
		KERROR( "DX8 Shader Compilation Error: %s", errorString );

		return false;
	}
	else
	{
		// Log that the compile was successful
		const char * logString = cgGetLastListing( g_cgContext );
		KLOG2( "%s", logString );
	}

	// Now the shader has been loaded, attempt to compile the shader
//	DWORD vertexDeclarations[MAX_FVF_DECL_SIZE];
//	memset( &vertexDeclarations, 0, sizeof(vertexDeclarations) );
//	DWORD * vertexDeclaration = &vertexDeclarations[0];
//	cgD3D8GetVertexDeclaration( program, vertexDeclaration );
    DWORD vertexDeclaration[] = {
        D3DVSD_STREAM(0),
        D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
        D3DVSD_REG(D3DVSDE_NORMAL, D3DVSDT_FLOAT3),
        D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2),
        D3DVSD_REG(D3DVSDE_TEXCOORD1, D3DVSDT_FLOAT3),
        D3DVSD_REG(D3DVSDE_TEXCOORD2, D3DVSDT_FLOAT3),
        D3DVSD_REG(D3DVSDE_TEXCOORD3, D3DVSDT_FLOAT3),
        D3DVSD_END(),
    };

	HRESULT hr =  cgD3D8LoadProgram( program,							// Cg Program
									 TRUE,								// Use parameter shadowing
									 D3DXASM_DEBUG,						// Assemble flags
									 D3DUSAGE_SOFTWAREPROCESSING,		// Vertex shader usages
									 vertexDeclaration );				// Vertex declaration
	if ( FAILED(hr) )
	{
		// The shader failed. This is so we don't attempt to load the shader
		// again during the next enableShader().
		shader.compileFailed();

		// Return the error string
		const char * errorString = cgD3D8TranslateHRESULT( hr );
		KERROR( "DX8 Shader Compilation Error: %s", errorString );

		return false;
	}

	// Setup the program identifier within the shader
	shader.compileSuccessful( program );
*/
	return true;
}