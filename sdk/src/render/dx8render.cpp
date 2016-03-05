/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8render.cpp
	Author:		Eric Bryant

	Concrete Render class which supports DirectX8
*/

#include <d3d8.h>
#include <d3dx8.h>
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
#include "dx8render.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8statemanager.h"
#include "dx8texturemanager.h"
#include "dx8shadermanager.h"
#include "dx8utility.h"

// Make sure client includes DirectX8 libaries
#pragma comment(lib, "d3d8.lib")
#pragma comment(lib, "d3dx8.lib")

//
// Macros
//
#define CHECK_FLAG(var, flag) (var&flag) == flag

//
// Local Functions
//
void __ConvertCreationToUsageFlags(BufferCreationFlags eFlags, long & dwUsage);
void __ConvertBufferToFVF(BufferTypes eEnabledBuffers, long & dwFVF);
void __ConvertFilterMode(Texture::FilterMode eFilter, int & iFilterMin, int & iFilterMax, int & iFilterMip);
void __ConvertRepeatMode(Texture::RepeatMode eRepeat, int & iRepeatU, int & iRepeatV);

//
// Constructor
//
DX8Render::DX8Render() :
	m_DXinit(false),
	m_useFVF(true)
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
DX8Render::~DX8Render()
{
	Shutdown();
}

//
// Initialize
//
bool DX8Render::Initialize(RenderInfo & Info)
{
	HRESULT hr;

	// Plug the RenderInfo as the target rendering parameters
	m_RenderInfo = Info;

	// Create the Utility class
	m_apUtility.reset( new DX8Utility );

	// Create the DirectX8 Object and Device
	hr = m_apUtility->Create(&Info);

	if ( FAILED(hr) )
		return false;

	// Grab the D3D object
	m_pD3D = m_apUtility->GetDirect3D();

	// Grab the D3DDevice
	m_pD3DDevice = m_apUtility->GetDirect3DDevice();

	// Create the state manager
	m_apStateManager.reset( new DX8StateManager );

	if ( false == m_apStateManager->Initialize( this->shared_from_this() ) )
		return false;

	// Set the D3DDevice into the state manager
	m_apStateManager->SetDevice( m_pD3DDevice );

	// Create the texture manager
	m_apTextureManager.reset( new DX8TextureManager );
	
	if ( false == m_apTextureManager->Initialize( m_pD3DDevice ) )
		return false;

	// Create the shader manager
	m_apShaderManager.reset( new DX8ShaderManager );

	if ( false == m_apShaderManager->Initialize( m_pD3DDevice ) )
		return false;

	// We are now ready to go
	m_DXinit = true;

	// Logging Activity
	KLOG("DirectX8 Renderer Initialization");

	// Log Render Statistics
	QueryDevice();

	return true;
}

//
// Shutdown
//
bool DX8Render::Shutdown()
{
	// Makes no sense to shutdown if we aren't initialized
	if ( false == m_DXinit )
		return false;

	if ( false == m_apUtility->Cleanup() )
		return false;

	if ( false == m_apShaderManager->Terminate() )
		return false;

	if ( false == m_apTextureManager->Terminate() )
		return false;

	// Log Activity
	KLOG("Renderer Shutdown");

	m_DXinit = false;

	return true;
}

//
// BeginFrame
//
bool DX8Render::BeginFrame()
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
bool DX8Render::EndFrame()
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
bool DX8Render::SetViewport(Camera & Cam)
{
	// Compute Viewport Parameters
	float fAspect = float(m_RenderInfo.uiTargetWidth) / m_RenderInfo.uiTargetHeight;

	// Grab a Reference to the Projection Matrix because we have to modify it for DX8
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
bool DX8Render::SetMatrix(MatrixType eType, MatrixFunction eFunct, const Matrix4 & Mat)
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
bool DX8Render::SetState(RenderState * pState)
{
	return m_apStateManager->SetState( pState );
}

//
// RenderGeometry
//
bool DX8Render::RenderGeometry(Geometry * geom)
{
	HRESULT hr;

	// Geometry is valid or if the device is set
	if ( !geom || !m_pD3DDevice.isValid() ) return false;

	// No Primitives to Render, Don't Bother
	if ( geom->m_primitiveCount == 0 || !geom->m_vertexBuffer || !geom->m_vertexBuffer->size() ) return false;

	// Use Fixed Function Pipeline
	DWORD dwFVF = D3DFVF_XYZ;

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

	// Add the diffuse color if enabled
	if ( CHECK_FLAG( geom->m_enabledBuffers, COLOR ) )
	{
		dwFVF |= D3DFVF_DIFFUSE;
		m_pD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
	}

	hr = m_pD3DDevice->SetVertexShader(dwFVF);
	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_RENDER_PRIMITIVE, "RenderGeometry::SetVertexShader (for fixed function shader) failed.");
		return false;
	}

	// Compute the stride between vertices
	int vertexStride = sizeof(float) * 3; // x,y,z

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
bool DX8Render::RenderVB(VertexBuffer * pVB)
{
	HRESULT hr;

	// Check if the VB is valid and the device is set
	if ( ( pVB == NULL ) || !m_pD3DDevice.isValid() )
		return false;

	// Update the Vertex Buffer
	if ( false == pVB->UploadBuffers() )
		return false;

	// Cast the VertexBuffer into DX8VB
	// NOTE: This is unsafe, but we can't do a safe dynamic cast because
	//		 VertexBuffer inherits PROTECTED from RTTI
	DX8VertexBuffer * pDX8VB = static_cast<DX8VertexBuffer *>(pVB);
	if ( pDX8VB == NULL )
		return false;

	// Grab the native IDirect3DVertexBuffer8 from the wrapper
	IDirect3DVertexBuffer8 * pNativeVB = pDX8VB->GetNativeVB();
	if ( pNativeVB == NULL )
		return false;

	// TODO: The following block shouldn't be called per frame
		
	// If we're using a fixed function shader, set the vertex format into the vertex shader
	if ( true == m_useFVF )
	{
		// First Convert the Buffers to FVF Format
		long dwFVF;
		__ConvertBufferToFVF(pVB->getEnabledBuffers(), dwFVF);

		// Set the FVF Format into the Shader, this let's it know that we're using
		// a fixed pipeline.

		hr = m_pD3DDevice->SetVertexShader(dwFVF);
		if ( FAILED(hr) )
		{
			SetError(UNABLE_TO_RENDER_PRIMITIVE, "SetVertexShader (for fixed function shader) failed.");
			return false;
		}
	}

	// Setup source of next rendering (interleaved vertices, normals, textures)
	hr = m_pD3DDevice->SetStreamSource( 0, pNativeVB, pDX8VB->GetVertexStride() );
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
			DWORD dwCurrentVertex = 0;
			DWORD dwVertexStep = pDX8VB->getActiveVertexCount() / pDX8VB->getTriangleStripCount();
			DWORD dwPrimCount = dwVertexStep - 2;

			for (DWORD i = 0; i < pDX8VB->getTriangleStripCount(); i++)
			{
				hr = m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, dwCurrentVertex, dwPrimCount);
				if ( FAILED(hr) )
				{
					SetError(UNABLE_TO_RENDER_PRIMITIVE, "DrawPrimitive (D3DPT_TRIANLGE_STRIP) Failed.");
					return false;
				}

				dwCurrentVertex += dwVertexStep;
			}
		}
		else
		{
			// This is an ordinatary primitve being drawn
			hr = m_pD3DDevice->DrawPrimitive(primitiveType, 
											 0, 
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
		IDirect3DIndexBuffer8 * pNativeIB = pDX8VB->GetNativeIndices();
		if ( pNativeIB == NULL )
			return false;

		// Setup index source for next render
		hr = m_pD3DDevice->SetIndices(pNativeIB, 0);
		if ( FAILED(hr) )
			return false;

		// Grab the primitive stats
		unsigned int uiPrimitiveCount = pVB->getPrimitiveCount();
		unsigned int uiVertexCount = pVB->getActiveVertexCount();
		unsigned int uiIndexOffset = pVB->getIndexOffset();

		// Draw the primitive using indices
		hr = m_pD3DDevice->DrawIndexedPrimitive(primitiveType, 
												0, 
												uiVertexCount, 
												uiIndexOffset, 
												uiPrimitiveCount);

		if ( FAILED(hr) )
		{
			SetError(UNABLE_TO_RENDER_PRIMITIVE, "DrawIndexedPrimitive Failed.");
			return false;
		}
	}

	return true;
}

bool DX8Render::RenderVB(VertexBuffer * pVB, IndexBuffer * pIB)
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

	// Cast the VertexBuffer into DX8VB
	// NOTE: This is unsafe, but we can't do a safe dynamic cast because
	//		 VertexBuffer inherits PROTECTED from RTTI
	DX8VertexBuffer * pDX8VB = static_cast<DX8VertexBuffer *>(pVB);
	if ( pDX8VB == NULL ) return false;

	// Cast the IndexBuffer into DX8IB
	DX8IndexBuffer * pDX8IB = static_cast<DX8IndexBuffer *>( pIB );
	if ( pDX8IB == NULL ) return false;

	// Grab the native IDirect3DVertexBuffer8 from the wrapper
	IDirect3DVertexBuffer8 * pNativeVB = pDX8VB->GetNativeVB();
	if ( pNativeVB == NULL ) return false;

	// Grab the native IDirect3DIndexBuffer8 from the wrapper
	IDirect3DIndexBuffer8 * pNativeIB = pDX8IB->GetNativeIndices();
	if ( pNativeIB == NULL ) return false;

	// TODO: The following block shouldn't be called per frame

	// If we're using a fixed function shader, set the vertex format into the vertex shader
	if ( true == m_useFVF )
	{
		// First Convert the Buffers to FVF Format
		long dwFVF;
		__ConvertBufferToFVF(pVB->getEnabledBuffers(), dwFVF);

		// Set the FVF Format into the Shader, this let's it know that we're using
		// a fixed pipeline.

		hr = m_pD3DDevice->SetVertexShader(dwFVF);
		if ( FAILED(hr) )
		{
			SetError(UNABLE_TO_RENDER_PRIMITIVE, "SetVertexShader (for fixed function shader) failed.");
			return false;
		}
	}

	// Setup source of next rendering (interleaved vertices, normals, textures)
	hr = m_pD3DDevice->SetStreamSource( 0, pNativeVB, pDX8VB->GetVertexStride() );
	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_RENDER_PRIMITIVE, "SetStreamSource failed.");
		return false;
	}

	// Get the primitive type (the PrimitiveType enumeration corresponds to D3DPRIMITIVETYPE)
	D3DPRIMITIVETYPE primitiveType = (D3DPRIMITIVETYPE)pVB->getPrimitiveType();

	// Setup index source for next render
	hr = m_pD3DDevice->SetIndices(pNativeIB, 0);
	if ( FAILED(hr) )
		return false;

	// TODO: Store or calculate the number of primitives based on the
	//		 index count
	unsigned int uiPrimitiveCount = pIB->getActiveIndexCount();
	unsigned int uiVertexCount = pVB->getActiveVertexCount();
	unsigned int uiIndexOffset = pVB->getIndexOffset();

	// Draw the primitive using indices
	hr = m_pD3DDevice->DrawIndexedPrimitive(primitiveType, 
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
VertexBuffer * DX8Render::CreateVB(BufferTypes eEnabledBuffers,
								   BufferCreationFlags eCreationFlags,
								   unsigned int uiVertexCount,
								   unsigned int uiIndexCount)
{
	// Check the device
	if ( !m_pD3DDevice.isValid() )
		return false;

	// Create the DX8 "vertex buffer" Wrapper
	DX8VertexBuffer * pVB = new DX8VertexBuffer(uiVertexCount, uiIndexCount, eEnabledBuffers, eCreationFlags);
	if ( NULL == pVB )
	{
		SetError(UNABLE_TO_CREATE_VB, "Creation of DX8VertexBuffer failed.");
		return 0;
	}

	// Initialize the DX8 Vertex Buffer
	pVB->Initialize();

	// Determine usage flags
	long dwUsage;
	__ConvertCreationToUsageFlags(eCreationFlags, dwUsage);

	// Determine flexible vertex format
	long dwFVF;
	__ConvertBufferToFVF(eEnabledBuffers, dwFVF);

	// Create a native DirectX8 Vertex Buffer
	HRESULT hr;
	IDirect3DVertexBuffer8 * pDX8VB = 0;
	hr = m_pD3DDevice->CreateVertexBuffer(	uiVertexCount * pVB->GetVertexStride(), 
											dwUsage,
											dwFVF,
											D3DPOOL_DEFAULT,
											&pDX8VB);

	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_CREATE_VB, "Creation of IDirect3DVertexBuffer8 failed.");
		return 0;
	}

	// Setup the native DX8 VB inside the DX8 VB Wrapper
	pVB->SetNativeVB(pDX8VB);	

	// Are Index Buffers enabled?
	if ( CHECK_FLAG(eEnabledBuffers, INDEX) )
	{
		// Create a native DirectX8 Index Buffer
		IDirect3DIndexBuffer8 * pDX8IB = 0;
		hr = m_pD3DDevice->CreateIndexBuffer(	uiIndexCount * sizeof(short),
												dwUsage,
												D3DFMT_INDEX16,
												D3DPOOL_DEFAULT,
												&pDX8IB);
		if ( FAILED(hr) )
		{
			SetError(UNABLE_TO_CREATE_VB, "Creation of IDirect3DIndexBuffer8 failed.");
			return 0;
		}

		// Setup the native DX8 IndexBuffer inside the DX8 VB Wrapper
		pVB->SetNativeIndices(pDX8IB);
	}

	return pVB;
}

//
// CreateIB
//
IndexBuffer * DX8Render::CreateIB(BufferCreationFlags eCreationFlags,
								  unsigned int uiIndexCount)
{
	// Check the device
	if ( !m_pD3DDevice.isValid() )
		return false;

	// Create the DX8 "index buffer" Wrapper
	DX8IndexBuffer * pIB = new DX8IndexBuffer(uiIndexCount, eCreationFlags);
	if ( NULL == pIB )
	{
		SetError(UNABLE_TO_CREATE_VB, "Creation of DX8IndexBuffer failed.");
		return 0;
	}

	// Initialize the DX8 Vertex Buffer
	pIB->Initialize();

	// Determine usage flags
	long dwUsage;
	__ConvertCreationToUsageFlags(eCreationFlags, dwUsage);

	// Create a native DirectX8 Index Buffer
	IDirect3DIndexBuffer8 * pDX8IB = 0;
	HRESULT hr = m_pD3DDevice->CreateIndexBuffer(uiIndexCount * sizeof(short),
												 dwUsage,
												 D3DFMT_INDEX16,
												 D3DPOOL_DEFAULT,
												 &pDX8IB);
	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_CREATE_VB, "Creation of IDirect3DIndexBuffer8 failed.");
		return 0;
	}

	// Setup the native DX8 IndexBuffer inside the DX8 VB Wrapper
	pIB->SetNativeIndices(pDX8IB);

	return pIB;
}

//
// BindTexture
//
bool DX8Render::BindTexture(Texture * pTexture)
{
	// Check the device
	if ( !m_pD3DDevice.isValid() )
		return false;

	// Retrieve the associates D3DTexture from the texture manager
	IDirect3DBaseTexture8 * pDX8Texture = 0;
	if ( false == m_apTextureManager->BindTexture(pTexture, &pDX8Texture) )
		return false;

	// Check the Base Texture
	if ( pDX8Texture == NULL )
		return false;

	// Sets the Texture Parameters for the given texture stage
	unsigned int uiTextureStage = pTexture->getTextureStage();

	// Sets the Repeat Texture Parameters for this pass
	int iRepeatU, iRepeatV;
	__ConvertRepeatMode(pTexture->getRepeatMode(), iRepeatU, iRepeatV);
	m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_ADDRESSU, iRepeatU);
	m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_ADDRESSV, iRepeatV);

	// Sets the Filter Texture Parameters for this pass
	int iFilterMin, iFilterMax, iFilterMip;
	__ConvertFilterMode(pTexture->getFilterMode(), iFilterMin, iFilterMax, iFilterMip);
	m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_MINFILTER, iFilterMin);
	m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_MAGFILTER, iFilterMax);
	m_pD3DDevice->SetTextureStageState(uiTextureStage, D3DTSS_MIPFILTER, iFilterMip);

	// Sets the source texture for this stage
	HRESULT hr = m_pD3DDevice->SetTexture(uiTextureStage, pDX8Texture);
	if ( FAILED(hr) )
	{
		SetError(UNABLE_TO_BIND_TEXTURE, "IDirect3DDevice::SetTexture failed.");
		return false;
	}

	return true;
}

//
// getShader
//
shared_ptr<Shader> DX8Render::getShader( const char * szShaderName )
{
	return shared_ptr<Shader>();
}

//
// SetShader
//
bool DX8Render::SetShader( VertexShader * vertexShader, PixelShader * pixelShader )
{
/*
	// If the vertex shader is null, disable the vertex shader
	if ( !vertexShader )
	{
		// Allow the manager to disable the shaders
		m_apShaderManager->disableShaders();

		// Disable all the Texture Stages (from the previous operation)
		m_pD3DDevice->SetTexture(0, NULL);
		m_pD3DDevice->SetTexture(1, NULL);
		m_pD3DDevice->SetTexture(2, NULL);
		m_pD3DDevice->SetTexture(3, NULL);

		// Enable FVF (Fixed Vertex Format)
		return ( m_useFVF = true );
	}

	// Otherwise, setup the pixel and vertex shaders
	if ( !m_apShaderManager->enableShader( *vertexShader ) )
		return false;

	// If the pixel shader is active, enable this also
	if ( pixelShader )
		if ( !m_apShaderManager->enableShader( *pixelShader ) )
			return false;

	// Disable FVF (Fixed Vertex Format)
	return ( m_useFVF = false );
*/
	return false;
}

//
// SetShaderConstants
//
bool DX8Render::SetShaderConstants( std::vector<ShaderConstant> * vertexConstants, std::vector<ShaderConstant> * pixelConstants )
{
	return m_apShaderManager->setConstants( vertexConstants, pixelConstants );
}

//
// GrabScreenBuffer
//
bool DX8Render::GrabScreenBuffer(char * pBuffer, unsigned int uiLeft, unsigned uiTop, unsigned uiRight, unsigned int uiBottom)
{
	IDirect3DSurface8 * pFrontBuffer;

   // Create the image that our screen shot will be copied into
   // NOTE: Surface format of the front buffer is D3DFMT_A8R8G8B8 when it is returned
   m_pD3DDevice->CreateImageSurface(uiLeft, uiTop, D3DFMT_A8R8G8B8, &pFrontBuffer);

   // Copy the front buffer into our surface
   HRESULT hr = m_pD3DDevice->GetFrontBuffer(pFrontBuffer);
   if ( FAILED(hr) )
   {
		pFrontBuffer->Release();
		return false;
   }

	// Copy Bits to Buffer
	D3DLOCKED_RECT d3dlr;
	pFrontBuffer->LockRect( &d3dlr, 0, 0 );
	DWORD * pDst = (DWORD *)d3dlr.pBits;

	memcpy(pBuffer, pDst, 4 * (uiRight - uiLeft) * (uiBottom - uiTop) * sizeof(char));

	// Release the surface so there is no memory leak
	pFrontBuffer->Release();

	return false;
}

//
// SetBackgroundColor
//
void DX8Render::SetBackgroundColor(ColorA & color)
{
	m_backgroundColor = color;
}

//
// ClearBuffers
//
void DX8Render::ClearBuffer()
{
	// Convert float colors to bytes ( [0.0f - 1.0f] range to [0 -255])
	char r = char(m_backgroundColor.r * 255);
	char g = char(m_backgroundColor.g * 255);
	char b = char(m_backgroundColor.b * 255);

	// Clears the backbuffer
	HRESULT hr;
	hr = m_pD3DDevice->Clear(0, 
							 NULL, 
							 D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
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
void DX8Render::SwapBuffers()
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
// SetCurrentShader
bool DX8Render::SetCurrentShader(Shader * pShader)
{
	return false;
}

//
// QueryDevice
//
void DX8Render::QueryDevice()
{
	const D3DCAPS8 DeviceCaps = m_apUtility->GetDeviceCaps();

	// Setup Render Info
	m_RenderInfo.eDriver = RenderInfo::DIRECTX8;		
	m_RenderInfo.iMaximumTexutreSize = DeviceCaps.MaxTextureWidth;
	m_RenderInfo.iMaximumHardwareLights = DeviceCaps.MaxActiveLights;
	m_RenderInfo.iMaximumTextureUnits = DeviceCaps.MaxSimultaneousTextures;
	m_RenderInfo.iMaximumClipPlanes = DeviceCaps.MaxUserClipPlanes;
	m_RenderInfo.bSupportCubeMaps = ( DeviceCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP ? true : false );
	m_RenderInfo.bSupportProjectedTextures = ( DeviceCaps.TextureCaps & D3DPTEXTURECAPS_PROJECTED ? true : false );

	strcpy(m_RenderInfo.szDriverName, "UNKNOWN");	// TODO
	strcpy(m_RenderInfo.szVendorName, "UNKNOWN");	// TODO
/*
	// Setup Shader Capacities
	if ( DeviceCaps.VertexShaderVersion < D3DVS_VERSION(1,0) )
		Shader::SetDeviceSupportShaders(false);
	else
	{
		Shader::SetDeviceSupportShaders(true);
		Shader::SetMaximumConstants(DeviceCaps.MaxVertexShaderConst);
		PixelShader::SetShaderVersion(D3DSHADER_VERSION_MAJOR(DeviceCaps.PixelShaderVersion));
		VertexShader::SetShaderVersion(D3DSHADER_VERSION_MAJOR(DeviceCaps.VertexShaderVersion));
	}
*/
	kstring strRenderInfo;

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
		 "\tVertex Shader Version: %d\r\n"
		 "\tPixel Shader Version: %d\r\n"
		 "\tMaximum Shader Constants: %d",
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
		 0,		// Vertex Shader Version
		 0,		// Pixel Shader Version
		 0);	// Shader Constants #

}

//
// __ConvertCreationToUsageFlags
//
void __ConvertCreationToUsageFlags(BufferCreationFlags eFlags, long & dwUsage)
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
// __ConvertBufferToFVF
//
void __ConvertBufferToFVF(BufferTypes eEnabledBuffers, long & dwFVF)
{
	dwFVF = 0;

	if ( CHECK_FLAG(eEnabledBuffers, VERTEX) )
		dwFVF |= D3DFVF_XYZ;

	if ( CHECK_FLAG(eEnabledBuffers, TEXTURE_0) )
		dwFVF |= D3DFVF_TEX1;

	if ( CHECK_FLAG(eEnabledBuffers, TEXTURE_1) )
		dwFVF |= D3DFVF_TEX2;

	if ( CHECK_FLAG(eEnabledBuffers, NORMALS) )
		dwFVF |= D3DFVF_NORMAL;

	if ( CHECK_FLAG(eEnabledBuffers, COLOR) )
		dwFVF |= D3DFVF_DIFFUSE;
}

//
// __ConvertFilterMode
//
void __ConvertFilterMode(Texture::FilterMode eFilter, int & iFilterMin, int & iFilterMax, int & iFilterMip)
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
// __ConvertRepeatMode
//
void __ConvertRepeatMode(Texture::RepeatMode eRepeat, int & iRepeatU, int & iRepeatV)
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