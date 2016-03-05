/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		dx9statemanager.h
	Author:		Eric Bryant

	Responsible for setting up DirectX9 render states (alpha, blending, etc.)
*/

#ifndef _DX9STATEMANAGER_H
#define _DX9STATEMANAGER_H

namespace Katana
{

// Forward Declarations
class Render;
class RenderState;
class WireframeState;
class AlphaState;
class StencilState;
class MaterialState;
class ZBufferState;
class FogState;
class DitherState;
class TextureState;
class MultitextureState;
class CullState;
class LightState;
class BlendState;
class MaterialState;
class ShaderState;
class ShadingState;

///
/// DX9StateManager
/// Responsible for setting up DirectX9 render states (alpha, blending, etc.)
///
class DX9StateManager
{
public:
	/// Constructor
	DX9StateManager();

	/// Setup the initial render states
	bool Initialize( shared_ptr<Render> spParentRender, IDirect3DDevice9 * pD3DDevice );

	/// Terminate the state manage which releases resources
	bool Terminate();

	/// Sets the appropate state with the manager
	bool SetState( RenderState * pState );

private:
	/// Various concrete state implementaitons (called by SetState())
	bool SetWireframeState( WireframeState * pWire );
	bool SetAlphaState( AlphaState * pAlpha );
	bool SetStencilState( StencilState * pStencil );
	bool SetMaterialState( MaterialState * pMaterial );
	bool SetZBufferState( ZBufferState * pZBuffer );
	bool SetFogState( FogState * pFog );
	bool SetDitherState( DitherState * pDither );
	bool SetTextureState( TextureState * pTexture );
	bool SetMultitextureState( MultitextureState * pMultitexture );
	bool SetCullState( CullState * pCull );
	bool SetLightState( LightState * pLight );
	bool SetBlendState( BlendState * pBlend );
	bool SetShaderState( ShaderState * pShader );
	bool SetShadingState( ShadingState * pShading );

private:
	/// Setup the texture blending operation
	void SetTexBlendOperation(unsigned int uiTextureStage, int iOperation);

	// Compiles the vertex and pixel shader
	bool CompileVertexShader( ShaderState * pShader );
	bool CompilePixelShader( ShaderState * pShader );

	// These constants define the entry point for the vertex and pixel shaders.
	// Thus, if you want to define vertex or pixel shader functions they must 
	// be declared with these names.
	// The default is:	VS_ENTRY_POINT = "vertex_main"
	//					PS_ENTRY_POINT = "pixel_main"
	static const char * VS_ENTRY_POINT;
	static const char * PS_ENTRY_POINT;

private:
	/// Parent renderer which owns the state manager
	weak_ptr<Render>			m_wpParentRender;

	/// Pointer to Direct3D8 Device
	ComPtr<IDirect3DDevice9>	m_pD3DDevice;

	/// Pointer to created Vertex and Pixel shaders (as a result of SetShaderState)
	vector<LPDIRECT3DVERTEXSHADER9> m_vertexShaders;
	vector<LPDIRECT3DPIXELSHADER9>  m_pixelShaders;

	/// Cache of previous render states
	bool		m_bPreviousLightState;
	bool		m_bPreviousCullState;
	bool		m_bPreviousZWrite;
	bool		m_bPreviousZTest;
	bool		m_bPreviousFogState;
};

} // Katana

#endif // _DX9STATEMANAGER_H