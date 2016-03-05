/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		dx9statemanager.cpp
	Author:		Eric Bryant

	Responsible for setting up DirectX9 render states (alpha, blending, etc.)
*/

#include <d3d9.h>
#include <d3dx9shader.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "base/comptr.h"
#include "rendertypes.h"
#include "texture.h"
#include "material.h"
#include "light.h"
#include "render.h"
#include "rendertypes.h"
#include "renderstate.h"
#include "wireframestate.h"
#include "cullstate.h"
#include "multitexturestate.h"
#include "lightstate.h"
#include "blendstate.h"
#include "zbufferstate.h"
#include "materialstate.h"
#include "shaderstate.h"
#include "shadingstate.h"
#include "stencilstate.h"
#include "dx9statemanager.h"

// ------------------------------------------------
// Local Functions
// ------------------------------------------------

D3DCOLORVALUE	ConvertToD3DColor( const ColorA & color );
D3DVECTOR		ConvertToD3DVector( const Point3 & point );
D3DLIGHTTYPE	ConvertToLightType( const Light * light );
bool			CompileVertexShader( ShaderState * pShader );
bool			CompilePixelShader( ShaderState * pShader );

// ------------------------------------------------
// DX9StateManager
// ------------------------------------------------

// ------------------------------------------------
// Static Variables
// ------------------------------------------------

const char * DX9StateManager::VS_ENTRY_POINT = "vertex_main";
const char * DX9StateManager::PS_ENTRY_POINT = "pixel_main";

// ------------------------------------------------

//
// Constructor
//
DX9StateManager::DX9StateManager() 
	: m_bPreviousLightState( true )
	, m_bPreviousCullState( true )
	, m_bPreviousZWrite( false )
	, m_bPreviousZTest( false )
	, m_bPreviousFogState( false )
{
}

//
// Initialize
//
bool DX9StateManager::Initialize( shared_ptr<Render> spParentRender, IDirect3DDevice9 * pD3DDevice )
{
	// Setup the parent renderer
	m_wpParentRender = spParentRender;

	// Store the DirectX9 Device
	m_pD3DDevice = pD3DDevice;

	// In order for the winding to match OpenGL, we'll set it to clockwise
	m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

	return true;
}

//
// Terminate
//
bool DX9StateManager::Terminate()
{
	// Iterate thorough all the shaders and release them
	for ( vector< LPDIRECT3DVERTEXSHADER9 >::iterator iter = m_vertexShaders.begin();
		  iter != m_vertexShaders.end();
		  iter++ )
	{
		(*iter)->Release();
	}
	for ( vector< LPDIRECT3DPIXELSHADER9 >::iterator iter = m_pixelShaders.begin();
		iter != m_pixelShaders.end();
		iter++ )
	{
		(*iter)->Release();
	}

	// Clear the collections
	m_vertexShaders.clear();
	m_pixelShaders.clear();

	return true;
}

//
// SetState
// Dispatches the states to the various helper functions
//
bool DX9StateManager::SetState(RenderState * pState)
{
	if ( KIsDerivedFromClass<MultitextureState>(pState) )
		return SetMultitextureState( KDynamicCast<MultitextureState>(pState) );
	else if ( KIsDerivedFromClass<WireframeState>(pState) )
		return SetWireframeState( KDynamicCast<WireframeState>(pState) );
	else if ( KIsDerivedFromClass<StencilState>(pState) )
		return SetStencilState( KDynamicCast<StencilState>(pState) );
	else if ( KIsDerivedFromClass<CullState>(pState) )
		return SetCullState( KDynamicCast<CullState>(pState) );
	else if ( KIsDerivedFromClass<LightState>(pState) )
		return SetLightState( KDynamicCast<LightState>(pState) );
	else if ( KIsDerivedFromClass<BlendState>(pState) )
		return SetBlendState( KDynamicCast<BlendState>(pState) );
	else if ( KIsDerivedFromClass<ZBufferState>(pState) )
		return SetZBufferState( KDynamicCast<ZBufferState>(pState) );
	else if ( KIsDerivedFromClass<MaterialState>(pState) )
		return SetMaterialState( KDynamicCast<MaterialState>(pState) );
	else if ( KIsDerivedFromClass<ShaderState>(pState) )
		return SetShaderState( KDynamicCast<ShaderState>(pState) );
	else if ( KIsDerivedFromClass<ShadingState>(pState) )
		return SetShadingState( KDynamicCast<ShadingState>(pState) );

	return false;
}

//
// SetCullState
//
bool DX9StateManager::SetCullState( CullState * pCull )
{
	if ( true == pCull->getCull() )
	{
		// Enable culling for backfacing only
		if ( pCull->getCullFace() == BACK )
		{
			long lCullState = ( pCull->getWinding() == CLOCKWISE ? D3DCULL_CW : D3DCULL_CCW );
			m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, lCullState );
		}
		else
		{
			// Disable culling
			m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		}
	}
	else
	{
		// Disable culling
		m_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	}

	return true;
}

//
// SetWireframeState
//
bool DX9StateManager::SetWireframeState( WireframeState * pWireframe )
{
	// Retrieve the wireframe mode
	long lFillMode = ( pWireframe->getWireframe() ? D3DFILL_WIREFRAME : D3DFILL_SOLID );

	// Set the fill mode in the renderer
	if ( FAILED( m_pD3DDevice->SetRenderState(D3DRS_FILLMODE, lFillMode) ) )
		return false;

	return true;
}

//
// SetStencilState
//
bool DX9StateManager::SetStencilState( StencilState * pStencil )
{
	if ( true == pStencil->getStencilEnabled() )
	{
		// Enable stencil with parameters
		m_pD3DDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE );
		m_pD3DDevice->SetRenderState( D3DRS_STENCILFUNC,  pStencil->getCompareFunction() );
		m_pD3DDevice->SetRenderState( D3DRS_STENCILZFAIL, pStencil->getZFailOp() );
		m_pD3DDevice->SetRenderState( D3DRS_STENCILFAIL,  pStencil->getFailOp() );

		// If ztest passes, inc/decrement stencil buffer value
		m_pD3DDevice->SetRenderState( D3DRS_STENCILREF,       pStencil->getStencilRef() );
		m_pD3DDevice->SetRenderState( D3DRS_STENCILMASK,      pStencil->getStencilMask() );
		m_pD3DDevice->SetRenderState( D3DRS_STENCILWRITEMASK, pStencil->getStencilWriteMask() );
		m_pD3DDevice->SetRenderState( D3DRS_STENCILPASS,      pStencil->getPassOp() );
	}
	else
	{
		// Disable stencil
		m_pD3DDevice->SetRenderState( D3DRS_STENCILENABLE, FALSE );
	}

	return true;
}

//
// SetMultitextureState
//
bool DX9StateManager::SetMultitextureState( MultitextureState * pMultitexture )
{
	shared_ptr<Render> spRender = m_wpParentRender.lock();
	if ( !spRender ) return false;

	// Disable all the Texture Stages (currently we support 4)
	m_pD3DDevice->SetTexture( 0, NULL );
	m_pD3DDevice->SetTexture( 1, NULL );
	m_pD3DDevice->SetTexture( 2, NULL );
	m_pD3DDevice->SetTexture( 3, NULL );

	// For each texture pass, enable texture state and bind the texture
	for(int index = 0; index < MAX_TEXTURE_PASSES; index++)
	{
		// Grab the texture
		shared_ptr<Texture> texture = pMultitexture->getSourceTexture( index );
		if ( !texture ) break;

		// Store the texture state
		texture->setTextureStage( index );

		// Bind the texture to dx8
		if ( !spRender->BindTexture( texture.get() ) )
			return false;

		// Setup the blending operation
		SetTexBlendOperation( index, pMultitexture->getBlendOperation( index ) );
	}

	return false;
}

//
// SetLightState
//
bool DX9StateManager::SetLightState( LightState * pLight )
{
	if ( !pLight->getEnableLights() )
	{
		// At the very least, disable all lights
		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

		return true;
	}
	else
	{
		// Grab the lights to enable for the state
		const vector< shared_ptr<Light> > & lights = pLight->getLights();

		// Do we have any lights to enable for this state
		if ( lights.size() == 0 ) return false;

		// For each light, set up the light within the hardware
		for( unsigned int index = 0; index < lights.size(); index++ )
		{
			// Check if we need to enable the lights
			if ( pLight->getEnableLights() )
			{
				D3DLIGHT9 d3dlight;
				ZeroMemory( &d3dlight, sizeof(D3DLIGHT9) );
				d3dlight.Type			= ConvertToLightType( lights[index].get() );
				d3dlight.Ambient		= ConvertToD3DColor( lights[index]->getAmbient() );
				d3dlight.Diffuse		= ConvertToD3DColor( lights[index]->getDiffuse() );
				d3dlight.Specular		= ConvertToD3DColor( lights[index]->getSpecular() );
				d3dlight.Position		= ConvertToD3DVector( lights[index]->getPosition() );
				d3dlight.Range			= lights[index]->getRange();

				if ( AmbientLight * pAmbientLight = KDynamicCast<AmbientLight>( lights[index].get() ) )
				{
					ColorA ambient = pAmbientLight->getAmbient();
					m_pD3DDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( ambient.r, ambient.g, ambient.b, ambient.a ) );
				}

				if ( PointLight * pPointLight = KDynamicCast<PointLight>( lights[index].get() ) )
				{
					d3dlight.Attenuation0 = pPointLight->getConstantAttenuation();
					d3dlight.Attenuation1 = pPointLight->getLinearAttenuation();
					d3dlight.Attenuation2 = pPointLight->getQuadraticAttenuation();
				}

				if ( SpotLight * pSpotLight = KDynamicCast<SpotLight>( lights[index].get() ) )
				{
					d3dlight.Direction		= ConvertToD3DVector( pSpotLight->getDirection() );
					d3dlight.Phi			= pSpotLight->getOuterCone();
					d3dlight.Theta			= pSpotLight->getInnerCone();
					d3dlight.Falloff		= pSpotLight->getFalloff();
				}

				if ( DirectionLight * pDirectionLight = KDynamicCast<DirectionLight>( lights[index].get() ) )
				{
					d3dlight.Direction		= ConvertToD3DVector( pDirectionLight->getDirection() );
				}

				m_pD3DDevice->SetLight( index, &d3dlight );
			}

			// Enable/disable the appropiate lights
			m_pD3DDevice->LightEnable( index, pLight->getEnableLights() ? TRUE: FALSE );
		}

		// Enable or disable DirectX8 lighting
		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, pLight->getEnableLights() ? TRUE : FALSE );

		return true;
	}
}

//
// SetBlendState
//
bool DX9StateManager::SetBlendState( BlendState * pBlending )
{
	if ( pBlending->getEnableBlend() )
	{
		// Enable alpha blending
		m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

		// Setup the blending functions
		m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, (D3DBLENDOP)pBlending->getSourceOperation() );
		m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, (D3DBLENDOP)pBlending->getDestinationOperation() );
	}
	else
	{
		// Disable alpha blending
		m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	}

	return true;
}

//
// SetZBufferState
//
bool DX9StateManager::SetZBufferState( ZBufferState * pZBuffer )
{
	if ( pZBuffer->getZBufferEnable() )
	{
		// Enable Z Buffer comparisons
		m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

		// Enable writing to the Z Buffer
		m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, pZBuffer->getZBufferWrites() );

		// Setup the Z Buffer Compare operation
		m_pD3DDevice->SetRenderState( D3DRS_ZFUNC, (D3DCMPFUNC)pZBuffer->getZBufferCompareOp() );
	}
	else
	{
		// Disable Z Buffer comparisons
		m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	}

	return true;
}

//
// SetMaterialState
//
bool DX9StateManager::SetMaterialState( MaterialState * pMaterial )
{
	// Grab the material
	shared_ptr<Material> material = pMaterial->getMaterial();
	if ( !material ) return false;

	// Create a DX8 Material and copy the settings
	D3DMATERIAL9 d3dmat;
	ZeroMemory( &d3dmat, sizeof( D3DMATERIAL9 ) );
	d3dmat.Ambient.r = material->ambient.r;		// Ambient
	d3dmat.Ambient.g = material->ambient.g;
	d3dmat.Ambient.b = material->ambient.b;
	d3dmat.Ambient.a = material->ambient.a;
	d3dmat.Diffuse.r = material->diffuse.r;		// Diffuse
	d3dmat.Diffuse.g = material->diffuse.g;
	d3dmat.Diffuse.b = material->diffuse.b;
	d3dmat.Diffuse.a = material->diffuse.a;
	d3dmat.Specular.r = material->specular.r;	// Specular
	d3dmat.Specular.g = material->specular.g;
	d3dmat.Specular.b = material->specular.b;
	d3dmat.Specular.a = material->specular.a;
	d3dmat.Emissive.r = material->emissive.r;	// Emissive
	d3dmat.Emissive.g = material->emissive.g;
	d3dmat.Emissive.b = material->emissive.b;
	d3dmat.Emissive.a = material->emissive.a;

	// Store the material within the device
	m_pD3DDevice->SetMaterial( &d3dmat );

	return true;
}

//
// SetShaderState
//
bool DX9StateManager::SetShaderState( ShaderState * pShader )
{
	// First, check the shader to see if it has already been compiled
	if ( !pShader->getShaderHandle0() )
	{
		// Check that we have shader code to execute
		if ( pShader->getShaderFile().empty() )
		{
			// Disable shaders and use the fixed function pipeline
			m_pD3DDevice->SetVertexShader( NULL );
			m_pD3DDevice->SetPixelShader( NULL );

			return false;
		}

		// Compile the vertex shader
		if ( !CompileVertexShader( pShader ) ) return false;

		// Compile the pixel shader (this is optional)
		if ( !CompilePixelShader( pShader ) ) return false;
	}

	// Retrieve the vertex shader from the shader state
	LPDIRECT3DVERTEXSHADER9 pVertexShader = (LPDIRECT3DVERTEXSHADER9) pShader->getShaderHandle0();

	// Set this as the current vertex shader
	if ( FAILED( m_pD3DDevice->SetVertexShader( pVertexShader ) ) )
		return false;

	// Iterate and setup the vertex shader constants
	for ( unsigned int vindex = 0, offset = 0; 
		  vindex < pShader->getShaderConstants0().size(); 
		  vindex++ )
	{
		const ShaderConstant & constant = pShader->getShaderConstants0()[vindex];
		if ( constant.constSize ) {
			m_pD3DDevice->SetVertexShaderConstantF( offset, 
													(float *)&constant.scalar, 
													constant.constSize >> 4 );
		}
		offset += ( constant.constSize >> 4 );
	}

	// Retrrieve the pixel shader from the shader state
	LPDIRECT3DPIXELSHADER9 pPixelShader = (LPDIRECT3DPIXELSHADER9) pShader->getShaderHandle1();

	// Did this shader have a pixel shader associated with it
	if ( pPixelShader )
	{
		// Sets this as the current pixel shader
		if ( FAILED( m_pD3DDevice->SetPixelShader( pPixelShader ) ) )
			return false;

		// Iterate and setup the pixel shader constants
		for ( unsigned int pindex = 0, offset = 0; 
			pindex < pShader->getShaderConstants1().size(); 
			pindex++ )
		{
			const ShaderConstant & constant = pShader->getShaderConstants1()[pindex];
			if ( constant.constSize )
			{
				m_pD3DDevice->SetPixelShaderConstantF( offset, 
					(float *)&constant.scalar, 
					constant.constSize >> 4 );
			}
			offset += ( constant.constSize >> 4 );
		}
	}

	return true;
}

bool DX9StateManager::SetShadingState( ShadingState * pShading )
{
	// Retrieve the shading mode
	long lShadingMode = (long) pShading->getShadingMode();

	// Set the fill mode in the renderer
	if ( FAILED( m_pD3DDevice->SetRenderState(D3DRS_SHADEMODE, lShadingMode) ) )
		return false;

	return true;
}

bool DX9StateManager::CompileVertexShader( ShaderState * pShader )
{
	// TODO: Store this value within the render info
	string vertexShaderProfile = D3DXGetVertexShaderProfile( m_pD3DDevice );

	// Buffer which stores the resultant compiled shader code and error messages
	LPD3DXBUFFER pCompiledShaderCode = NULL;
	LPD3DXBUFFER pErrorMessage = NULL;

	// If not, let's compile the shader
	HRESULT hr = D3DXCompileShaderFromFile(	pShader->getShaderFile().c_str(),	// Uncompiled shader file
		NULL,								// An optional NULL terminated array of D3DXMACRO structures.
		NULL,								// Optional interface pointer, ID3DXInclude, to use for handling #include directives. 
		VS_ENTRY_POINT,						// Shader entry point function where execution begins
		vertexShaderProfile.c_str(),		// Shader profile which detemines the shader instruction set
#ifdef _DEBUG	// Enable shader debugging if in debug mode
		D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION,					// Compile options 
#else			// Otherwise no flags are needed
		0,
#endif
		&pCompiledShaderCode,				// Returns a buffer containing the created shader
		&pErrorMessage,						// Returns a buffer containing a listing of errors and warnings that were encountered during the compile
		NULL								// Returns an ID3DXConstantTable interface, which can be used to access shader constants.
		);

	// If the compilation failed, exit
	if ( FAILED(hr) ) { KERROR( "%s", pErrorMessage->GetBufferPointer() ); pErrorMessage->Release(); return false; }

	// Pointer to the actual vertex shader
	LPDIRECT3DVERTEXSHADER9 pVertexShader = NULL;

	// Create the actual vertex shader
	hr = m_pD3DDevice->CreateVertexShader( (DWORD *)pCompiledShaderCode->GetBufferPointer(), &pVertexShader );
	if ( FAILED(hr) ) return false;

	// Add this shader to our collection of shader (for release during shutdown)
	m_vertexShaders.push_back( pVertexShader );

	// Finally, store this vertex shader within the shader state
	pShader->setShaderHandle0( (long) pVertexShader );

	// Release the buffers
	pCompiledShaderCode->Release();

	return true;
}

bool DX9StateManager::CompilePixelShader( ShaderState * pShader )
{
	// TODO: Store this value within the render info
	string pixelShaderProfile = D3DXGetPixelShaderProfile( m_pD3DDevice );

	// Buffer which stores the resultant compiled shader code and error messages
	LPD3DXBUFFER pCompiledShaderCode = NULL;
	LPD3DXBUFFER pErrorMessage = NULL;

	// If not, let's compile the shader
	HRESULT hr = D3DXCompileShaderFromFile(	pShader->getShaderFile().c_str(),	// Uncompiled shader file
		NULL,								// An optional NULL terminated array of D3DXMACRO structures.
		NULL,								// Optional interface pointer, ID3DXInclude, to use for handling #include directives. 
		PS_ENTRY_POINT,						// Shader entry point function where execution begins
		pixelShaderProfile.c_str(),			// Shader profile which detemines the shader instruction set
		0,									// No flags needed
		&pCompiledShaderCode,				// Returns a buffer containing the created shader
		&pErrorMessage,						// Returns a buffer containing a listing of errors and warnings that were encountered during the compile
		NULL								// Returns an ID3DXConstantTable interface, which can be used to access shader constants.
		);

	// If the compilation failed, exit
	if ( FAILED(hr) ) { 

		// If the error is X3501, don't bother logging, this simply means a pixel shader is not defined for this shader
		char * errorString = static_cast<char *>( pErrorMessage->GetBufferPointer() );
		if ( !strstr( errorString, "X3501" )  ) KERROR( "%s", errorString ); 

		// Release the error buffer and return false
		pErrorMessage->Release(); 
		return false; 
	}

	// Pointer to the actual pixel shader
	LPDIRECT3DPIXELSHADER9 pPixelShader = NULL;

	// Create the actual pixel shader
	hr = m_pD3DDevice->CreatePixelShader( (DWORD *)pCompiledShaderCode->GetBufferPointer(), &pPixelShader );
	if ( FAILED(hr) ) return false;

	// Add this shader to our collection of shader (for release during shutdown)
	m_pixelShaders.push_back( pPixelShader );

	// Finally, store this pixel shader within the shader state
	pShader->setShaderHandle1( (long) pPixelShader );

	// Release the buffers
	pCompiledShaderCode->Release();

	return true;
}

// ---------------------------------------------------------------------------------

//
// SetTexBlendOperation
//
void DX9StateManager::SetTexBlendOperation(unsigned int uiTextureStage, int iOperation)
{
	switch(iOperation)
	{
	case MultitextureState::NONE:
		// NOP
		break;

	case MultitextureState::REPLACE:
		// TEX2 = TEX1
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE);            
		break;

	case MultitextureState::MODULATE:
		// TEX2 * TEX1
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLOROP, D3DTOP_MODULATE);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG2, D3DTA_CURRENT);
		break;

	case MultitextureState::DECAL:
		// TEX2.ALPHA * TEX2 + (1 - TEX2.ALPHA) * TEX1
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG2, D3DTA_CURRENT);            
		break;

	case MultitextureState::ADD:
		// requires both Multitexture AND combine ext
		// TEX2 + TEX1
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLOROP, D3DTOP_ADD);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG2, D3DTA_CURRENT);            
		break;

	case MultitextureState::ADD_SIGNED:
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG2, D3DTA_CURRENT);  			
		break;

	case MultitextureState::DETAIL:
		// requires both Multitexture AND combine ext
		// TEX2 * TEX1 * 2.0
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG2, D3DTA_CURRENT);            
		break;

	case MultitextureState::ALPHA:
		// TEX2.ALPHA * TEX2 + (1 - TEX2.ALPHA) * TEX1
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_COLORARG2, D3DTA_CURRENT);            
		break;

	default:
		break;
	}
}

// ---------------------------------------------------------------------------------

D3DCOLORVALUE ConvertToD3DColor( const ColorA & color )
{
	D3DCOLORVALUE d3dcolor;
	d3dcolor.r = color.r; d3dcolor.g = color.g; d3dcolor.b = color.b; d3dcolor.a = color.a;

	return d3dcolor;
}

D3DVECTOR ConvertToD3DVector( const Point3 & point )
{
	D3DVECTOR d3dvector;
	d3dvector.x = point.x; d3dvector.y = point.y; d3dvector.z = point.z;

	return d3dvector;
}

D3DLIGHTTYPE ConvertToLightType( const Light * light )
{
	D3DLIGHTTYPE d3dlighttype = D3DLIGHT_POINT;

	if ( KIsDerivedFromClass<PointLight>(light) )		d3dlighttype = D3DLIGHT_POINT;
	if ( KIsDerivedFromClass<SpotLight>(light) )		d3dlighttype = D3DLIGHT_SPOT;
	if ( KIsDerivedFromClass<DirectionLight>(light) )	d3dlighttype = D3DLIGHT_DIRECTIONAL;

	return d3dlighttype;
}