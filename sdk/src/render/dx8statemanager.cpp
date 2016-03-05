/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8statemanager.cpp
	Author:		Eric Bryant

	Responsible for setting up DirectX8 render states (alpha, blending, etc.)
*/

#include <d3d8.h>

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
#include "dx8statemanager.h"

//
// Local Functions
//
D3DCOLORVALUE	__ConvertToD3DColor( const ColorA & color );
D3DVECTOR		__ConvertToD3DVector( const Point3 & point );
D3DLIGHTTYPE	__ConvertToLightType( const Light * light );

//
// Constructor
//
DX8StateManager::DX8StateManager() :
	m_bPreviousLightState(true),
	m_bPreviousCullState(true),
	m_bPreviousZWrite(false),
	m_bPreviousZTest(false),
	m_bPreviousFogState(false)
{
}

//
// Destructor
//
DX8StateManager::~DX8StateManager()
{
}

//
// Initialize
//
bool DX8StateManager::Initialize( shared_ptr<Render> spParentRender )
{
	// Setup the parent renderer
	m_wpParentRender = spParentRender;

	return true;
}

void DX8StateManager::SetDevice(IDirect3DDevice8 * pD3DDevice)
{
	m_pD3DDevice = pD3DDevice;

	// In order for the winding to match OpenGL, we'll set it to clockwise
	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
}

//
// SetState
//
bool DX8StateManager::SetState(RenderState * pState)
{
	if ( KIsDerivedFromClass<MultitextureState>(pState) )
		return SetMultitextureState( KDynamicCast<MultitextureState>(pState) );
	if ( KIsDerivedFromClass<WireframeState>(pState) )
		return SetWireframeState( KDynamicCast<WireframeState>(pState) );
	if ( KIsDerivedFromClass<CullState>(pState) )
		return SetCullState( KDynamicCast<CullState>(pState) );
	if ( KIsDerivedFromClass<LightState>(pState) )
		return SetLightState( KDynamicCast<LightState>(pState) );
	if ( KIsDerivedFromClass<BlendState>(pState) )
		return SetBlendState( KDynamicCast<BlendState>(pState) );
	if ( KIsDerivedFromClass<ZBufferState>(pState) )
		return SetZBufferState( KDynamicCast<ZBufferState>(pState) );
	if ( KIsDerivedFromClass<MaterialState>(pState) )
		return SetMaterialState( KDynamicCast<MaterialState>(pState) );

	return false;
}

//
// SetCullState
//
bool DX8StateManager::SetCullState( CullState * cull )
{
	if ( true == cull->getCull() )
	{
		// Enable culling for backfacing only
		if ( cull->getCullFace() == BACK )
		{
			long lCullState = ( cull->getWinding() == CLOCKWISE ? D3DCULL_CW : D3DCULL_CCW );
			m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, lCullState);
		}
		else
		{
			// Disable culling
			m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		}
	}
	else
	{
		// Disable culling
		m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	}

	return true;
}

//
// SetWireframeState
//
bool DX8StateManager::SetWireframeState( WireframeState * wire )
{
	// Retrieve the wireframe mode
	long lFillMode = ( wire->getWireframe() ? D3DFILL_WIREFRAME : D3DFILL_SOLID );
    
	// Set the fill mode in the renderer
	HRESULT hr = m_pD3DDevice->SetRenderState(D3DRS_FILLMODE, lFillMode);

	if ( FAILED(hr) )
		return false;

	return true;
}

//
// SetMultitextureState
//
bool DX8StateManager::SetMultitextureState(MultitextureState * multi)
{
	shared_ptr<Render> spRender = m_wpParentRender.lock();
	if ( !spRender ) return false;

	// Disable all the Texture Stages
	m_pD3DDevice->SetTexture( 0, NULL );
	m_pD3DDevice->SetTexture( 1, NULL );
	m_pD3DDevice->SetTexture( 2, NULL );
	m_pD3DDevice->SetTexture( 3, NULL );

	// For each texture pass, enable texture state and bind the texture
	for(int index = 0; index < MAX_TEXTURE_PASSES; index++)
	{
		// Grab the texture
		shared_ptr<Texture> texture = multi->getSourceTexture( index );
		if ( !texture ) break;

		// Store the texture state
		texture->setTextureStage( index );

		// Bind the texture to dx8
		if ( !spRender->BindTexture( texture.get() ) )
			return false;
	
		// Setup the blending operation
		SetTexBlendOperation(index, multi->getBlendOperation( index ));
	}

	return false;
}

//
// SetLightState
//
bool DX8StateManager::SetLightState( LightState * light )
{
	// Grab the lights to enable for the state
	const vector< shared_ptr<Light> > & lights = light->getLights();

	// Do we have any lights to enable for this state
	if ( lights.size() == 0 ) return false;

	// For each light, set up the light within the hardware
	for( unsigned int index = 0; index < lights.size(); index++ )
	{
		// Check if we need to enable the lights
		if ( light->getEnableLights() )
		{
			D3DLIGHT8 d3d8light;
			ZeroMemory( &d3d8light, sizeof(D3DLIGHT8) );
			d3d8light.Type			= __ConvertToLightType( lights[index].get() );
			d3d8light.Ambient		= __ConvertToD3DColor( lights[index]->getAmbient() );
			d3d8light.Diffuse		= __ConvertToD3DColor( lights[index]->getDiffuse() );
			d3d8light.Specular		= __ConvertToD3DColor( lights[index]->getSpecular() );
			d3d8light.Position		= __ConvertToD3DVector( lights[index]->getPosition() );
			d3d8light.Range			= lights[index]->getRange();

			if ( AmbientLight * pAmbientLight = KDynamicCast<AmbientLight>( lights[index].get() ) )
			{
				ColorA ambient = pAmbientLight->getAmbient();
				m_pD3DDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( ambient.r, ambient.g, ambient.b, ambient.a ) );
			}

			if ( PointLight * pPointLight = KDynamicCast<PointLight>( lights[index].get() ) )
			{
				d3d8light.Attenuation0 = pPointLight->getConstantAttenuation();
				d3d8light.Attenuation1 = pPointLight->getLinearAttenuation();
				d3d8light.Attenuation2 = pPointLight->getQuadraticAttenuation();
			}
				
			if ( SpotLight * pSpotLight = KDynamicCast<SpotLight>( lights[index].get() ) )
			{
				d3d8light.Direction		= __ConvertToD3DVector( pSpotLight->getDirection() );
				d3d8light.Phi			= pSpotLight->getOuterCone();
				d3d8light.Theta			= pSpotLight->getInnerCone();
				d3d8light.Falloff		= pSpotLight->getFalloff();
			}

			if ( DirectionLight * pDirectionLight = KDynamicCast<DirectionLight>( lights[index].get() ) )
			{
				d3d8light.Direction		= __ConvertToD3DVector( pDirectionLight->getDirection() );
			}

			m_pD3DDevice->SetLight( index, &d3d8light );
		}

		// Enable/disable the appropiate lights
		m_pD3DDevice->LightEnable( index, light->getEnableLights() ? TRUE: FALSE );
	}

	// Enable or disable DirectX8 lighting
	m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, light->getEnableLights() ? TRUE : FALSE );

	return true;
}

//
// SetBlendState
//
bool DX8StateManager::SetBlendState( BlendState * blend )
{
	if ( blend->getEnableBlend() )
	{
		// Enable alpha blending
		m_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

		// Setup the blending functions
		m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, (D3DBLENDOP)blend->getSourceOperation() );
		m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, (D3DBLENDOP)blend->getDestinationOperation() );
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
bool DX8StateManager::SetZBufferState( ZBufferState * zbuffer )
{
	if ( zbuffer->getZBufferEnable() )
	{
		// Enable Z Buffer comparisons
		m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

		// Enable writing to the Z Buffer
		m_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, zbuffer->getZBufferWrites() );

		// Setup the Z Buffer Compare operation
		m_pD3DDevice->SetRenderState( D3DRS_ZFUNC, (D3DCMPFUNC)zbuffer->getZBufferCompareOp() );
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
bool DX8StateManager::SetMaterialState(MaterialState * pMaterial)
{
	// Grab the material
	shared_ptr<Material> material = pMaterial->getMaterial();
	if ( !material ) return false;

	// Create a DX8 Material and copy the settings
	D3DMATERIAL8 dx8mat;
	ZeroMemory( &dx8mat, sizeof(D3DMATERIAL8) );
	dx8mat.Ambient.r = material->ambient.r;		// Ambient
	dx8mat.Ambient.g = material->ambient.g;
	dx8mat.Ambient.b = material->ambient.b;
	dx8mat.Ambient.a = material->ambient.a;
	dx8mat.Diffuse.r = material->diffuse.r;		// Diffuse
	dx8mat.Diffuse.g = material->diffuse.g;
	dx8mat.Diffuse.b = material->diffuse.b;
	dx8mat.Diffuse.a = material->diffuse.a;
	dx8mat.Specular.r = material->specular.r;	// Specular
	dx8mat.Specular.g = material->specular.g;
	dx8mat.Specular.b = material->specular.b;
	dx8mat.Specular.a = material->specular.a;
	dx8mat.Emissive.r = material->emissive.r;	// Emissive
	dx8mat.Emissive.g = material->emissive.g;
	dx8mat.Emissive.b = material->emissive.b;
	dx8mat.Emissive.a = material->emissive.a;

	// Store the material within the device
	m_pD3DDevice->SetMaterial( &dx8mat );

	return true;
}

// ---------------------------------------------------------------------------------

//
// SetTexBlendOperation
//
void DX8StateManager::SetTexBlendOperation(unsigned int uiTextureStage, int iOperation)
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

//
// Local Functions
//

// ---------------------------------------------------------------------------------

D3DCOLORVALUE __ConvertToD3DColor( const ColorA & color )
{
	D3DCOLORVALUE d3dcolor;
	d3dcolor.r = color.r; d3dcolor.g = color.g; d3dcolor.b = color.b; d3dcolor.a = color.a;

	return d3dcolor;
}

D3DVECTOR __ConvertToD3DVector( const Point3 & point )
{
	D3DVECTOR d3dvector;
	d3dvector.x = point.x; d3dvector.y = point.y; d3dvector.z = point.z;

	return d3dvector;
}

D3DLIGHTTYPE __ConvertToLightType( const Light * light )
{
	D3DLIGHTTYPE d3dlighttype = D3DLIGHT_POINT;

	if ( KIsDerivedFromClass<PointLight>(light) )		d3dlighttype = D3DLIGHT_POINT;
	if ( KIsDerivedFromClass<SpotLight>(light) )		d3dlighttype = D3DLIGHT_SPOT;
	if ( KIsDerivedFromClass<DirectionLight>(light) )	d3dlighttype = D3DLIGHT_DIRECTIONAL;

	return d3dlighttype;
}