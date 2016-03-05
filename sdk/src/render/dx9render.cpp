/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		dx9render.cpp
	Author:		Eric Bryant

	Concrete Render class which supports DirectX9
*/

#include <d3d9.h>
#include <d3dx9.h>
#include "katana_config.h"
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "base/comptr.h"
#include "scene/visible.h"
#include "scene/camera.h"
#include "rendertypes.h"
#include "geometry.h"
#include "shader.h"
#include "texture.h"
#include "indexbuffer.h"
#include "vertexbuffer.h"
#include "render.h"
#include "rendertypes.h"
#include "dx9render.h"
#include "dx9enumeration.h"
#include "dx9vertexbuffer.h"
#include "dx9indexbuffer.h"
#include "dx9texturemanager.h"
#include "dx9statemanager.h"

// --------------------------------------------------------
// Make sure client includes DirectX9 libaries
// --------------------------------------------------------

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

// --------------------------------------------------------
// Macros
// --------------------------------------------------------

#define CHECK_FLAG(var, flag) (var&flag) == flag

// --------------------------------------------------------
// Local Functions
// --------------------------------------------------------

void ConvertCreationToUsageFlags( BufferCreationFlags eFlags, long & dwUsage );
void ConvertFilterMode( Texture::FilterMode eFilter, int & iFilterMin, int & iFilterMax, int & iFilterMip );
void ConvertRepeatMode( Texture::RepeatMode eRepeat, int & iRepeatU, int & iRepeatV );

// --------------------------------------------------------
// DX9Render
// --------------------------------------------------------

//
// Constructor
//
DX9Render::DX9Render() :
	m_bDXinit(false)
{
#ifdef _DEBUG
	m_backgroundColor = ColorA( 0.5f, 0.5f, 0.5f, 1.f ); // GREY
#else
	m_backgroundColor = ColorA( 0.f, 0.f, 0.f, 1.f ); // BLACK
#endif
}

//
// Destructor
//
DX9Render::~DX9Render()
{
	Shutdown();
}

//
// Initialize
//
bool DX9Render::Initialize(RenderInfo & Info)
{
	// Plug the RenderInfo as the target rendering parameters
	m_RenderInfo = Info;

	// Create the enumeration class
	m_spEnumeration.reset( new DX9Enumeration );

	// Create the DirectX8 Object and Device
	HRESULT hr = m_spEnumeration->Create( Info );
	if ( FAILED( hr ) ) return false;

	// Grab the D3D object
	m_pD3D = m_spEnumeration->GetDirect3D();

	// Grab the D3DDevice
	m_pD3DDevice = m_spEnumeration->GetDirect3DDevice();

	// Create the state manager
	m_apStateManager.reset( new DX9StateManager );

	// Set the D3DDevice into the state manager
	if ( false == m_apStateManager->Initialize( this->shared_from_this(), m_pD3DDevice ) )
		return false;

	// Create the texture manager
	m_apTextureManager.reset( new DX9TextureManager );

	if ( false == m_apTextureManager->Initialize( m_pD3DDevice ) )
		return false;

	// We are now ready to go
	m_bDXinit = true;

	// Logging Activity
	KLOG("DirectX8 Renderer Initialization");

	// Log Render Statistics
	QueryDevice();

	return true;
}

//
// Shutdown
//
bool DX9Render::Shutdown()
{
	// Makes no sense to shutdown if we aren't initialized
	if ( false == m_bDXinit )
		return false;

	if ( false == m_spEnumeration->Cleanup() )
		return false;

	if ( false == m_apStateManager->Terminate() )
		return false;

	// Log Activity
	KLOG("Renderer Shutdown");

	m_bDXinit = false;

	return false;
}

//
// BeginFrame
//
bool DX9Render::BeginFrame()
{
	// Clear the existing buffer
	ClearBuffer();

	// Tell D3D we're starting a new scene
	if ( FAILED( m_pD3DDevice->BeginScene() ) )
	{
		SetError(UNKNOWN_ERROR, "IDirect3DDevice::BeginScene() Failed.");
		return false;
	}

	return true;
}

//
// EndFrame
//
bool DX9Render::EndFrame()
{
	// Scene is finished
	if ( FAILED( m_pD3DDevice->EndScene() ) )
	{
		SetError(UNKNOWN_ERROR, "IDirect3DDevice::EndScene() Failed.");
		return false;
	}

	// Swap the Buffers
	SwapBuffers();

	return true;
}

//
// SetViewport
//
bool DX9Render::SetViewport(Camera & Cam)
{
	// Compute Viewport Parameters
	float fAspect = float(m_RenderInfo.uiTargetWidth) / m_RenderInfo.uiTargetHeight;

	// Grab a Reference to the Projection Matrix because we have to modify it for DX9
	const Matrix4 & projectionMatrix = Cam.getProjection();

	// HACK: The Matrix is an OpenGL Matrix (Vectors are Column Vectors), we need to Transpose
	//		 it for DX.
	//	ProjectionMatrix.Transpose();

	// Copy it to a D3DXMATRIX for the next function
	D3DXMATRIX matProjection;
	memcpy(&matProjection(0,0), &Cam.getProjection().right[0], sizeof(D3DXMATRIX));

	// Store the Projection Matrix
	m_pD3DDevice->SetTransform(D3DTS_PROJECTION, &matProjection);

	return true;
}

//
// SetMatrix
//
bool DX9Render::SetMatrix(MatrixType eType, MatrixFunction eFunct, const Matrix4 & Mat)
{
	D3DTRANSFORMSTATETYPE d3dtst;

	// Enable the appropiate matrix type
	switch ( eType )
	{
	case TEXTURE:
		d3dtst = D3DTS_TEXTURE0;
		break;

	case MODELVIEW:
		d3dtst = D3DTS_WORLD;
		break;
	case PROJECTION:
		d3dtst = D3DTS_PROJECTION;
		break;
	default:
		// Unknown enumeration
		return false;
	};

	switch ( eFunct )
	{
	case MULTIPLY:
		m_pD3DDevice->MultiplyTransform(d3dtst, (D3DMATRIX *)&Mat.right[0]);
		break;
	case STORE:
		m_pD3DDevice->SetTransform(d3dtst, (D3DMATRIX *)&Mat.right[0]);
		break;
	case PUSH:
		// TODO
		break;
	case POP:
		// TODO
		break;
	default:
		return false;
	}

	return true;
}

//
// SetState
//
bool DX9Render::SetState(RenderState * pState)
{
	return m_apStateManager->SetState( pState );
}

//
// RenderGeometry
//
bool DX9Render::RenderGeometry(Geometry * geom)
{
	HRESULT hr;

	// Geometry is valid or if the device is set
	if ( !geom || !m_pD3DDevice.isValid() ) return false;

	// No Primitives to Render, Don't Bother
	if ( geom->m_primitiveCount == 0 || !geom->m_vertexBuffer || !geom->m_vertexBuffer->size() ) return false;

	// Compute the stride between vertices
	int vertexStride = sizeof(float) * 3; // x,y,z

	// Use Fixed Function Pipeline
	DWORD dwFVF = D3DFVF_XYZ;
	if ( CHECK_FLAG( geom->m_enabledBuffers, VERTEX_T ) )
	{
		dwFVF = D3DFVF_XYZRHW;
		vertexStride = sizeof(float) * 4; // x,y,z,w
	}

	// Cache the ligthing state (we need to turn it off for diffuse coloring to work properly)
	DWORD dwLightingState;
	m_pD3DDevice->GetRenderState( D3DRS_LIGHTING, &dwLightingState );

	// Cache the blending modes
	DWORD dwSource, dwDestination;
	m_pD3DDevice->GetRenderState( D3DRS_SRCBLEND, &dwSource );
	m_pD3DDevice->GetRenderState( D3DRS_DESTBLEND, &dwDestination );

	// Cache the z buffer operation
	DWORD dwZBuffer;
	m_pD3DDevice->GetRenderState( D3DRS_ZENABLE, &dwZBuffer );

	// Set the blending mode as the defaults
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

	// Set the z buffer to off
	m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, FALSE );

	// Disable the texture states
	m_pD3DDevice->SetTexture( 0, NULL );
	m_pD3DDevice->SetTexture( 1, NULL );
	m_pD3DDevice->SetTexture( 2, NULL );
	m_pD3DDevice->SetTexture( 3, NULL );

	// Turn off lighting
	m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	// Add the diffuse color if enabled
	if ( CHECK_FLAG( geom->m_enabledBuffers, COLOR ) )
	{
		dwFVF |= D3DFVF_DIFFUSE;
	}

	hr = m_pD3DDevice->SetVertexShader( NULL );
	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_RENDER_PRIMITIVE, "RenderGeometry::SetVertexShader (for fixed function shader) failed.");
		return false;
	}

	// Add the color to the stride if enabled
	if ( CHECK_FLAG( geom->m_enabledBuffers, COLOR ) ) vertexStride += sizeof(DWORD); 

	// Render the Triangles
	hr = m_pD3DDevice->DrawPrimitiveUP((D3DPRIMITIVETYPE)geom->m_primitiveType,
		geom->m_primitiveCount,
		&geom->m_vertexBuffer->front(),
		vertexStride );

	if ( FAILED(hr) )
		return false;

	// Restore the lighting state
	if ( CHECK_FLAG( geom->m_enabledBuffers, COLOR ) )
		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, dwLightingState );

	// Restore the blending operations
	m_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, dwSource );
	m_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, dwDestination );

	// Restore the z buffering state
	m_pD3DDevice->SetRenderState( D3DRS_ZENABLE, dwZBuffer );

	return true;
}

//
// RenderVB
//
bool DX9Render::RenderVB(VertexBuffer * pVB)
{
	HRESULT hr;

	// Check if the VB is valid and the device is set
	if ( ( pVB == NULL ) || !m_pD3DDevice.isValid() )
		return false;

	// Update the Vertex Buffer
	if ( false == pVB->UploadBuffers() )
		return false;

	// Cast the VertexBuffer into DX9VB
	// NOTE: This is unsafe, but we can't do a safe dynamic cast because
	//		 VertexBuffer inherits PROTECTED from RTTI
	DX9VertexBuffer * pDX9VB = static_cast<DX9VertexBuffer *>(pVB);
	if ( pDX9VB == NULL )
		return false;

	// Grab the native IDirect3DVertexBuffer8 from the wrapper
	IDirect3DVertexBuffer9 * pNativeVB = pDX9VB->GetNativeVB();
	if ( pNativeVB == NULL )
		return false;

	// Convert the Buffers to FVF Format and sets the FVF into the shader (applicable for FVF
	// and custom pipeline)
	if ( !ConvertBufferToFVF( pVB->getEnabledBuffers() ) ) 
		return false;

	// Setup source of next rendering (interleaved vertices, normals, textures)
	hr = m_pD3DDevice->SetStreamSource( 0, pNativeVB, 0, pDX9VB->GetVertexStride() );
	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_RENDER_PRIMITIVE, "SetStreamSource failed.");
		return false;
	}

	// Get the primitive type (the PrimitiveType enumeration corresponds to D3DPRIMITIVETYPE)
	D3DPRIMITIVETYPE primitiveType = (D3DPRIMITIVETYPE)pVB->getPrimitiveType();

	// If we don't have indices defined, just blast through the vertices
	if ( false == pVB->isBufferEnabled(INDEX) )
	{
		// This allows us to set multiple strips to the renderer
		if ( primitiveType == TRIANGLE_STRIP )
		{
			// Determine the stride of the vertices (3 or 4 float depending on if they are transformed [in which
			// case it will have an additional w component])
			const unsigned int uiVertexStride = CHECK_FLAG( pDX9VB->getEnabledBuffers(), VERTEX_T ) ? 4 : 3;

			// Draw each triangle strip
			for( unsigned int uiStrip = 0; uiStrip < pDX9VB->getPrimitiveCount(); uiStrip++ )
			{
				if ( FAILED( m_pD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 
														  uiStrip * uiVertexStride, 
														  pDX9VB->getTriangleStripCount() ) ) )
				{
					SetError(UNABLE_TO_RENDER_PRIMITIVE, "DrawPrimitive (D3DPT_TRIANLGE_STRIP) Failed.");
					return false;
				}
			}
		}
		else
		{
			// This is an ordinatary primitve being drawn
			hr = m_pD3DDevice->DrawPrimitive(primitiveType, 
				pVB->getVertexOffset(), 
				pVB->getPrimitiveCount());

			if ( FAILED(hr) )
			{
				SetError(UNABLE_TO_RENDER_PRIMITIVE, "DrawPrimitive Failed.");
				return false;
			}
		}
	}
	else
	{
		// Grab the native Index Buffer from the wrapper
		IDirect3DIndexBuffer9 * pNativeIB = pDX9VB->GetNativeIndices();
		if ( pNativeIB == NULL )
			return false;

		// Setup index source for next render
		hr = m_pD3DDevice->SetIndices( pNativeIB );
		if ( FAILED(hr) )
			return false;

		// Grab the primitive stats
		const unsigned int uiPrimitiveCount = pVB->getPrimitiveCount();
		const unsigned int uiVertexCount = pVB->getActiveVertexCount();
		const unsigned int uiVertexOffset = pVB->getVertexOffset();
		const unsigned int uiIndexOffset = pVB->getIndexOffset();

		// Draw the primitive using indices
		hr = m_pD3DDevice->DrawIndexedPrimitive(
			primitiveType, 
			uiVertexOffset,
			0,
			uiVertexCount, 
			uiIndexOffset,	
			uiPrimitiveCount );

		if ( FAILED(hr) )
		{
			SetError(UNABLE_TO_RENDER_PRIMITIVE, "DrawIndexedPrimitive Failed.");
			return false;
		}
	}

	return true;
}

//
// RenderVB
//
bool DX9Render::RenderVB(VertexBuffer * pVB, IndexBuffer * pIB)
{
	HRESULT hr;

	if ( pVB == NULL || pIB == NULL || !m_pD3DDevice.isValid() )
		return false;

	// Update the Vertex Buffer
	if ( false == pVB->UploadBuffers() )
		return false;

	// Update the Index Buffer
	if ( false == pIB->UploadBuffers() )
		return false;

	// Cast the VertexBuffer into DX9VB
	// NOTE: This is unsafe, but we can't do a safe dynamic cast because
	//		 VertexBuffer inherits PROTECTED from RTTI
	DX9VertexBuffer * pDX9VB = static_cast<DX9VertexBuffer *>(pVB);
	if ( pDX9VB == NULL ) return false;

	// Cast the IndexBuffer into DX9IB
	DX9IndexBuffer * pDX9IB = static_cast<DX9IndexBuffer *>( pIB );
	if ( pDX9IB == NULL ) return false;

	// Grab the native IDirect3DVertexBuffer8 from the wrapper
	IDirect3DVertexBuffer9 * pNativeVB = pDX9VB->GetNativeVB();
	if ( pNativeVB == NULL ) return false;

	// Grab the native IDirect3DIndexBuffer8 from the wrapper
	IDirect3DIndexBuffer9 * pNativeIB = pDX9IB->GetNativeIndices();
	if ( pNativeIB == NULL ) return false;

	// Convert the Buffers to FVF Format and sets the FVF into the shader (applicable for FVF
	// and custom pipeline)
	if ( !ConvertBufferToFVF( pVB->getEnabledBuffers() ) ) 
		return false;

	// Setup source of next rendering (interleaved vertices, normals, textures)
	hr = m_pD3DDevice->SetStreamSource( 0, pNativeVB, 0, pDX9VB->GetVertexStride() );
	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_RENDER_PRIMITIVE, "SetStreamSource failed.");
		return false;
	}

	// Get the primitive type (the PrimitiveType enumeration corresponds to D3DPRIMITIVETYPE)
	D3DPRIMITIVETYPE primitiveType = (D3DPRIMITIVETYPE)pVB->getPrimitiveType();

	// Setup index source for next render
	hr = m_pD3DDevice->SetIndices( pNativeIB );
	if ( FAILED(hr) )
		return false;

	// TODO: Store or calculate the number of primitives based on the
	//		 index count
	const unsigned int uiPrimitiveCount = pVB->getPrimitiveCount();
	const unsigned int uiVertexCount = pVB->getActiveVertexCount();
	const unsigned int uiVertexOffset = pVB->getVertexOffset();
	const unsigned int uiIndexOffset = pVB->getIndexOffset();

	// Draw the primitive using indices
	hr = m_pD3DDevice->DrawIndexedPrimitive(
		primitiveType, 
		uiVertexOffset,
		0,
		uiVertexCount, 
		uiIndexOffset, 
		uiPrimitiveCount);

	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_RENDER_PRIMITIVE, "DrawIndexedPrimitive Failed.");
		return false;
	}

	return true;
}

// 
// CreateVB
//
VertexBuffer * DX9Render::CreateVB(BufferTypes eEnabledBuffers,
								   BufferCreationFlags eCreationFlags,
								   unsigned int uiVertexCount,
								   unsigned int uiIndexCount)
{
	// Check the device
	if ( !m_pD3DDevice.isValid() )
		return false;

	// Create the DX9 "vertex buffer" Wrapper
	DX9VertexBuffer * pVB = new DX9VertexBuffer( uiVertexCount, uiIndexCount, eEnabledBuffers, eCreationFlags );
	if ( NULL == pVB )
	{
		SetError(UNABLE_TO_CREATE_VB, "Creation of DX9VertexBuffer failed.");
		return 0;
	}

	// Initialize the DX9 Vertex Buffer
	pVB->Initialize();

	// Determine usage flags
	long dwUsage;
	ConvertCreationToUsageFlags( eCreationFlags, dwUsage );

	// Determine flexible vertex format
	long dwFVF;
	ConvertBufferToFVF( eEnabledBuffers, dwFVF );

	// Create a native DirectX8 Vertex Buffer
	HRESULT hr;
	IDirect3DVertexBuffer9 * pDX9VB = 0;
	hr = m_pD3DDevice->CreateVertexBuffer(	uiVertexCount * pVB->GetVertexStride(), 
											dwUsage,
											dwFVF,
											D3DPOOL_DEFAULT,
											&pDX9VB,
											NULL );

	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_CREATE_VB, "Creation of IDirect3DVertexBuffer9 failed.");
		return 0;
	}

	// Setup the native DX9 VB inside the DX9 VB Wrapper
	pVB->SetNativeVB( pDX9VB );

	// Are Index Buffers enabled?
	if ( CHECK_FLAG( eEnabledBuffers, INDEX ) )
	{
		// Create a native DirectX8 Index Buffer
		IDirect3DIndexBuffer9 * pDX9IB = 0;
		hr = m_pD3DDevice->CreateIndexBuffer(	uiIndexCount * sizeof(unsigned short),
												dwUsage,
												D3DFMT_INDEX16,
												D3DPOOL_DEFAULT,
												&pDX9IB,
												NULL );
		if ( FAILED(hr) )
		{
			SetError(UNABLE_TO_CREATE_VB, "Creation of IDirect3DIndexBuffer9 failed.");
			return 0;
		}

		// Setup the native DX9 IndexBuffer inside the DX9 VB Wrapper
		pVB->SetNativeIndices( pDX9IB );
	}

	return pVB;
}

//
// CreateIB
//
IndexBuffer * DX9Render::CreateIB(BufferCreationFlags eCreationFlags,
								  unsigned int uiIndexCount)
{
	// Check the device
	if ( !m_pD3DDevice.isValid() )
		return false;

	// Create the DX9 "index buffer" Wrapper
	DX9IndexBuffer * pIB = new DX9IndexBuffer( uiIndexCount, eCreationFlags );
	if ( NULL == pIB )
	{
		SetError(UNABLE_TO_CREATE_VB, "Creation of DX9IndexBuffer failed.");
		return 0;
	}

	// Initialize the DX9 Vertex Buffer
	pIB->Initialize();

	// Determine usage flags
	long dwUsage;
	ConvertCreationToUsageFlags( eCreationFlags, dwUsage );

	// Create a native DirectX9 Index Buffer
	IDirect3DIndexBuffer9 * pDX9IB = 0;
	HRESULT hr = m_pD3DDevice->CreateIndexBuffer( uiIndexCount * sizeof(unsigned short),
												  dwUsage,
												  D3DFMT_INDEX16,
												  D3DPOOL_DEFAULT,
												  &pDX9IB,
												  NULL );
	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_CREATE_VB, "Creation of IDirect3DIndexBuffer9 failed.");
		return 0;
	}

	// Setup the native DX9 IndexBuffer inside the DX9 VB Wrapper
	pIB->SetNativeIndices( pDX9IB );

	return pIB;
}

//
// BindTexture
//
bool DX9Render::BindTexture(Texture * pTexture)
{
	// Check the device
	if ( !m_pD3DDevice.isValid() )
		return false;

	// Retrieve the associates D3DTexture from the texture manager
	IDirect3DBaseTexture9 * pDX9Texture = 0;
	if ( false == m_apTextureManager->BindTexture( pTexture, &pDX9Texture ) )
		return false;

	// Check the Base Texture
	if ( pDX9Texture == NULL )
		return false;

	// Sets the Texture Parameters for the given texture stage
	unsigned int uiTextureStage = pTexture->getTextureStage();

	// Sets the Repeat Texture Parameters for this pass
	int iRepeatU, iRepeatV;
	ConvertRepeatMode(pTexture->getRepeatMode(), iRepeatU, iRepeatV);
	m_pD3DDevice->SetSamplerState( uiTextureStage, D3DSAMP_ADDRESSU, iRepeatU );
	m_pD3DDevice->SetSamplerState( uiTextureStage, D3DSAMP_ADDRESSV, iRepeatV );

	// Sets the Filter Texture Parameters for this pass
	int iFilterMin, iFilterMax, iFilterMip;
	ConvertFilterMode(pTexture->getFilterMode(), iFilterMin, iFilterMax, iFilterMip );
	m_pD3DDevice->SetSamplerState( uiTextureStage, D3DSAMP_MINFILTER, iFilterMin );
	m_pD3DDevice->SetSamplerState( uiTextureStage, D3DSAMP_MAGFILTER, iFilterMax );
	m_pD3DDevice->SetSamplerState( uiTextureStage, D3DSAMP_MIPFILTER, iFilterMip );

	// Sets the source texture for this stage
	HRESULT hr = m_pD3DDevice->SetTexture( uiTextureStage, pDX9Texture );
	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_BIND_TEXTURE, "IDirect3DDevice::SetTexture failed.");
		return false;
	}

	return true;
}

//
// GrabScreenBuffer
//
bool DX9Render::GrabScreenBuffer(char * pBuffer, unsigned int uiLeft, unsigned uiTop, unsigned uiRight, unsigned int uiBottom)
{
	return false; // TODO
}

//
// SetBackgroundColor
//
void DX9Render::SetBackgroundColor(ColorA & color)
{
	m_backgroundColor = color;
}

//
// ClearBuffers
//
void DX9Render::ClearBuffer()
{
	// Convert float colors to bytes ( [0.0f - 1.0f] range to [0 -255])
	char r = char( m_backgroundColor.r * 255 );
	char g = char( m_backgroundColor.g * 255 );
	char b = char( m_backgroundColor.b * 255 );

	// Clears the backbuffer
	HRESULT hr;
	hr = m_pD3DDevice->Clear(0, 
		NULL, 
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
		D3DCOLOR_XRGB( r, g, b ), 
		1.0, 
		0);
	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_CLEAR_BUFFER, "Problem Clearing the BackBuffer");
	}
}

//
// SwapBuffers
//
void DX9Render::SwapBuffers()
{
	// Present the backbuffer contents to the display
	HRESULT hr;
	hr = m_pD3DDevice->Present(NULL, NULL, NULL, NULL);

	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_SWAP_BUFFERS, "Problem Swapping Buffers");
	}
}

//
// QueryDevice
//
void DX9Render::QueryDevice()
{
	D3DCAPS9 DeviceCaps;
	if ( m_spEnumeration->GetDirect3DSettings().IsWindowed )
		DeviceCaps = m_spEnumeration->GetDirect3DSettings().pWindowed_DeviceInfo->Caps;
	else
		DeviceCaps = m_spEnumeration->GetDirect3DSettings().pFullscreen_DeviceInfo->Caps;

	// Setup Render Info
	m_RenderInfo.eDriver = RenderInfo::DIRECTX9;		
	m_RenderInfo.iMaximumTexutreSize = DeviceCaps.MaxTextureWidth;
	m_RenderInfo.iMaximumHardwareLights = DeviceCaps.MaxActiveLights;
	m_RenderInfo.iMaximumTextureUnits = DeviceCaps.MaxSimultaneousTextures;
	m_RenderInfo.iMaximumClipPlanes = DeviceCaps.MaxUserClipPlanes;
	m_RenderInfo.bSupportCubeMaps = ( DeviceCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP ? true : false );
	m_RenderInfo.bSupportProjectedTextures = ( DeviceCaps.TextureCaps & D3DPTEXTURECAPS_PROJECTED ? true : false );
	strcpy( m_RenderInfo.szVertexShaderProfile, D3DXGetVertexShaderProfile( m_pD3DDevice ) );
	strcpy( m_RenderInfo.szPixelShaderProfile, D3DXGetPixelShaderProfile( m_pD3DDevice ) );
	strcpy( m_RenderInfo.szDriverName, "UNKNOWN" );	// TODO
	strcpy( m_RenderInfo.szVendorName, "UNKNOWN" );	// TODO

	KLOG("Render Statistics:\r\n"
		"\tRender Driver: DIRECTX/3D Version %d\r\n"
		"\tTarget Fullscreen: %d\r\n"
		"\tTarget Width: %d\r\n"
		"\tTarget Height: %d\r\n"
		"\tTarget Bit Depth: %d\r\n"
		"\tMaximum Simultaneous Textures: %d\r\n"
		"\tMaximum Texture Size: %d\r\n"
		"\tMaximum Hardware Lights: %d\r\n"
		"\tMaximum User Clip Planes: %d\r\n"
		"\tDriver Vender: %s\r\n"
		"\tDriver Name: %s\r\n"
		"\tVertex Shader Profile: %s\r\n"
		"\tPixel Shader Profile: %s\r\n"
		"\tMaximum Vertex Shader Constants: %d",
		DIRECT3D_VERSION >> 8,
		m_RenderInfo.bFullscreen,
		m_RenderInfo.uiTargetWidth,
		m_RenderInfo.uiTargetHeight,
		m_RenderInfo.uiTargetColorDepth,
		m_RenderInfo.iMaximumTextureUnits,
		m_RenderInfo.iMaximumTexutreSize,
		m_RenderInfo.iMaximumHardwareLights,
		m_RenderInfo.iMaximumClipPlanes,
		m_RenderInfo.szVendorName,
		m_RenderInfo.szDriverName,
		m_RenderInfo.szVertexShaderProfile,
		m_RenderInfo.szPixelShaderProfile,
		DeviceCaps.MaxVertexShaderConst );
}

//
// getD3D
// Gets the D3D
//
IDirect3D9 * DX9Render::getD3D()
{
	return m_pD3D.getPtr();
}

//
// getD3DDevice
// Gets the D3D Device
//
IDirect3DDevice9 *	DX9Render::getD3DDevice()
{
	return m_pD3DDevice.getPtr();
}

//
// ConvertBufferToFVF
// Converts the enabled buffer type into a FVF flag and set it, or if
// the vertex declaration is incompatable with FVF, explicitly set the
// vertex declaration
//
bool DX9Render::ConvertBufferToFVF( BufferTypes eEnabledBuffers )
{
	// If TANGENT_S (or S & T) buffer is enabled, then we need to use the custom vertex declaration
	if ( CHECK_FLAG( eEnabledBuffers, TANGENT_S ) )
	{
		D3DVERTEXELEMENT9 tangentSDeclaration[]=
		{			
			{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
			{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,	0 },
			{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
			{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,	0 },
			D3DDECL_END()
		};

		D3DVERTEXELEMENT9 tangentSandTDeclaration[]=
		{			
			{ 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 },
			{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,	0 },
			{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 },
			{ 0, 32, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,	0 },
			{ 0, 44, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL,	0 },
			D3DDECL_END()
		};

		// Choose the [S] or [S and T] declaration
		D3DVERTEXELEMENT9 * declaration = tangentSDeclaration;
		if ( CHECK_FLAG( eEnabledBuffers, TANGENT_T ) )
			declaration = tangentSandTDeclaration;

		LPDIRECT3DVERTEXDECLARATION9 pVertexDeclaration = m_pD3DVertexDeclaration.getPtr();
		if ( FAILED ( m_pD3DDevice->CreateVertexDeclaration( declaration, &pVertexDeclaration ) ))
		{
			SetError(UNABLE_TO_RENDER_PRIMITIVE, "CreateVertexDeclaration failed.");
			return false;
		}

		// Set this as the current vertex declaration
		m_pD3DDevice->SetVertexDeclaration ( pVertexDeclaration );
	}
	else
	{
		// Compute the FVF flags
		long dwFVF;
		ConvertBufferToFVF( eEnabledBuffers, dwFVF );

		// Set the FVF Format into the Shader, this let's it the format of the vertices
		if ( FAILED( m_pD3DDevice->SetFVF( dwFVF ) ) )
		{
			SetError(UNABLE_TO_RENDER_PRIMITIVE, "ConvertBufferToFVF (for fixed function shader) failed.");
			return false;
		}
	}

	return true;
}

//
// ConvertBufferToFVF
// Converts the enabled buffer type into a FVF flag and returns it
//
bool DX9Render::ConvertBufferToFVF( BufferTypes eEnabledBuffers, long & dwFVF )
{
	dwFVF = 0;

	if ( CHECK_FLAG(eEnabledBuffers, VERTEX) )		dwFVF |= D3DFVF_XYZ;
	if ( CHECK_FLAG(eEnabledBuffers, VERTEX_T) )	dwFVF |= D3DFVF_XYZRHW;
	if ( CHECK_FLAG(eEnabledBuffers, TEXTURE_0) )	dwFVF |= D3DFVF_TEX1;
	if ( CHECK_FLAG(eEnabledBuffers, TEXTURE_1) )	dwFVF |= D3DFVF_TEX2;
	if ( CHECK_FLAG(eEnabledBuffers, NORMALS) )		dwFVF |= D3DFVF_NORMAL;
	if ( CHECK_FLAG(eEnabledBuffers, COLOR) )		dwFVF |= D3DFVF_DIFFUSE;

	return ( dwFVF != 0 );
}

// ---------------------------------------------------------

//
// ConvertCreationToUsageFlags
//
void ConvertCreationToUsageFlags( BufferCreationFlags eFlags, long & dwUsage )
{
	dwUsage = 0;

	if ( CHECK_FLAG(eFlags, DYNAMIC) )
		dwUsage |= D3DUSAGE_DYNAMIC;

	if ( CHECK_FLAG(eFlags, WRITE_ONLY) )
		dwUsage |= D3DUSAGE_WRITEONLY;

	if ( CHECK_FLAG(eFlags, DONOTCLIP) )
		dwUsage |= D3DUSAGE_DONOTCLIP;

	if ( CHECK_FLAG(eFlags, POINT_SPRITES) )
		dwUsage |= D3DUSAGE_POINTS;

	if ( CHECK_FLAG(eFlags, PATCHES) )
		dwUsage |= D3DUSAGE_RTPATCHES;
}

//
// ConvertFilterMode
//
void ConvertFilterMode( Texture::FilterMode eFilter, int & iFilterMin, int & iFilterMax, int & iFilterMip )
{
	switch ( eFilter )
	{
	case Texture::NEAREST:
		iFilterMin = D3DTEXF_POINT;
		iFilterMax = D3DTEXF_POINT;
		iFilterMip = D3DTEXF_NONE;
		break;

	case Texture::LINEAR:
		iFilterMin = D3DTEXF_LINEAR;
		iFilterMax = D3DTEXF_LINEAR;
		iFilterMip = D3DTEXF_NONE;
		break;

	case Texture::TRILINEAR:
		iFilterMin = D3DTEXF_ANISOTROPIC;
		iFilterMax = D3DTEXF_ANISOTROPIC;
		iFilterMip = D3DTEXF_NONE;
		break;

	case Texture::NEAREST_MIPMAP:
		iFilterMin = D3DTEXF_POINT;
		iFilterMax = D3DTEXF_POINT;
		iFilterMip = D3DTEXF_POINT;
		break;

	case Texture::LINEAR_MIPMAP:
		iFilterMin = D3DTEXF_LINEAR;
		iFilterMax = D3DTEXF_LINEAR;
		iFilterMip = D3DTEXF_LINEAR;
		break;

	default:
		// Defaults
		iFilterMin = D3DTEXF_POINT;
		iFilterMax = D3DTEXF_POINT;
		break;
	}
}

//
// ConvertRepeatMode
//
void ConvertRepeatMode( Texture::RepeatMode eRepeat, int & iRepeatU, int & iRepeatV )
{
	switch ( eRepeat )
	{
	case Texture::CLAMP_ST:
		iRepeatU = D3DTADDRESS_CLAMP;
		iRepeatV = D3DTADDRESS_CLAMP;
		break;

	case Texture::WRAP_ST:
		iRepeatU = D3DTADDRESS_WRAP;
		iRepeatV = D3DTADDRESS_WRAP;
		break;

	case Texture::CLAMP_S_WRAP_T:
		iRepeatU = D3DTADDRESS_CLAMP;
		iRepeatV = D3DTADDRESS_WRAP;
		break;

	case Texture::WRAP_S_CLAMP_T:
		iRepeatU = D3DTADDRESS_WRAP;
		iRepeatV = D3DTADDRESS_CLAMP;
		break;

	default:
		// Defaults
		iRepeatU = D3DTADDRESS_WRAP;
		iRepeatV = D3DTADDRESS_WRAP;
	}
}