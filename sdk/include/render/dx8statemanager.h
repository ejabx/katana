/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8statemanager.h
	Author:		Eric Bryant

	Responsible for setting up DirectX8 render states (alpha, blending, etc.)
*/

#ifndef _DX8STATEMANAGER_H
#define _DX8STATEMANAGER_H



namespace Katana
{

//
// Forward Declarations
//
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

///
/// DX8StateManager
/// Responsible for setting up DirectX8 render states (alpha, blending, etc.)
class DX8StateManager
{
public:
	/// Constructor
	DX8StateManager();

	/// Destructor
	~DX8StateManager();

	/// Setup the initial render states
	bool Initialize( shared_ptr<Render> spParentRender );

	/// Sets the appropate state with the manager
	bool SetState(RenderState * pState);

	/// Stores the Direct3D8 Device
	void SetDevice(IDirect3DDevice8 * pD3DDevice);

private:
	/// Various concrete state implementaitons (called by SetState())
	bool SetWireframeState(WireframeState * pWire);
	bool SetAlphaState(AlphaState * pAlpha);
	bool SetStencilState(StencilState * pStencil);
	bool SetMaterialState(MaterialState * pMaterial);
	bool SetZBufferState(ZBufferState * pZBuffer);
	bool SetFogState(FogState * pFog);
	bool SetDitherState(DitherState * pDither);
	bool SetTextureState(TextureState * pTexture);
	bool SetMultitextureState(MultitextureState * pMulti);
	bool SetCullState(CullState * pCull);
	bool SetLightState(LightState * pLight);
	bool SetBlendState(BlendState * pBlend);

	/// Setup the texture blending operation
	void SetTexBlendOperation(unsigned int uiTextureStage, int iOperation);

private:
	/// Parent renderer which owns the state manager
	weak_ptr<Render>			m_wpParentRender;

	/// Pointer to Direct3D8 Device
	ComPtr<IDirect3DDevice8>	m_pD3DDevice;

	/// Cache of previous render states
	bool		m_bPreviousLightState;
	bool		m_bPreviousCullState;
	bool		m_bPreviousZWrite;
	bool		m_bPreviousZTest;
	bool		m_bPreviousFogState;
};
}

#endif // _DX8STATEMANAGER_H