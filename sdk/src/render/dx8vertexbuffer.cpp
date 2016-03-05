/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8vertexbuffer.cpp
	Author:		Eric Bryant

	DX8 Vertex Buffer
*/

#include <d3d8.h>
#include <d3dx8.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "base/comptr.h"
#include "rendertypes.h"
#include "geometry.h"
#include "vertexbuffer.h"
#include "dx8vertexbuffer.h"

// Macros
#define CHECK_FLAG(var, flag) (var&flag) == flag

//
// Constructor
//
DX8VertexBuffer::DX8VertexBuffer(unsigned int uiVertexCount,
								 unsigned int uiIndexCount,
								 BufferTypes eEnabledBuffers,
								 BufferCreationFlags eCreationFlags) :
	VertexBuffer(uiVertexCount, uiIndexCount, eEnabledBuffers, eCreationFlags)
{
}

//
// Destructor
//
DX8VertexBuffer::~DX8VertexBuffer()
{
	// Unlock and cleanup the vertex buffer
	Unlock();
	Cleanup();
}

//
// Initialize
//
bool DX8VertexBuffer::Initialize()
{
	// Release the previous vertex buffer
	Cleanup();

	return true;
}

//
// Lock
//
bool DX8VertexBuffer::Lock()
{
	// Check if the vb is valid
	if ( !m_apDX8VertexBuffer.isValid() || ( isBufferEnabled(INDEX) && !m_apDX8IndexBuffer.isValid() ) )
		return false;

	// Lock the entire Vertex Buffer
	HRESULT hr;
	char * pVertexData = 0;
	hr = m_apDX8VertexBuffer->Lock( 0, 0, (BYTE**)&pVertexData, 0 );
	if ( FAILED(hr) )
		return false;

	// Store the address of the locked vertex buffer
	m_hwVertexBufferData.set( pVertexData, getActiveVertexCount() * GetVertexStride() );

	// Lock the entire Index Buffer
	unsigned short * pIndexData = 0;
	if ( true == isBufferEnabled(INDEX) )
	{
		hr = m_apDX8IndexBuffer->Lock( 0, 0, (BYTE**)&pIndexData, 0 );
		if ( FAILED(hr) )
			return false;

		// Store the address of the locked index buffer
		m_hwIndexBufferData.set( pIndexData, getActiveIndexCount() );
	}

	return true;
}

//
// LockRange
//
bool DX8VertexBuffer::LockRange( unsigned int uiStartVertex, unsigned int uiVertexCount,
								 unsigned int uiStartIndex,  unsigned int uiIndexCount )
{
	// Check if the vb is valid
	if ( !m_apDX8VertexBuffer.isValid()  )
		return false;

	// Lock the Vertex Buffer
	HRESULT hr;
	char * pVertexData = 0;
	hr = m_apDX8VertexBuffer->Lock( uiStartVertex * GetVertexStride(), 
									uiVertexCount * GetVertexStride(), 
									(BYTE**)&pVertexData, 
									0 );
	if ( FAILED(hr) )
		return false;

	// Store the address of the locked vertex buffer
	m_hwVertexBufferData.set( pVertexData, uiVertexCount * GetVertexStride() );

	// Lock the Index Buffer
	unsigned short * pIndexData = 0;
	if ( true == isBufferEnabled(INDEX) && uiIndexCount )
	{
		if ( !m_apDX8IndexBuffer.isValid() ) 
			return false;

		hr = m_apDX8IndexBuffer->Lock( uiStartIndex * sizeof(short),
									   uiIndexCount * sizeof(short),
									   (BYTE**)&pIndexData,
									   0 );
		if ( FAILED(hr) )
			return false;

		// Store the address of the locked index buffer
		m_hwIndexBufferData.set( pIndexData, uiIndexCount );
	}

	return true;
}

//
// Unlock
//
bool DX8VertexBuffer::Unlock()
{
	// Check if the vb is valid
	if ( !m_apDX8VertexBuffer.isValid()  )
		return false;

	// Unlock the Vertex Buffer
	HRESULT hr;
	hr = m_apDX8VertexBuffer->Unlock();

	if ( FAILED(hr) )
		return false;

	m_hwVertexBufferData.reset();

	// Unlock the Index Buffer
	if ( true == isBufferEnabled(INDEX) )
	{
		if ( !m_apDX8IndexBuffer.isValid() )
			return false;

		hr = m_apDX8IndexBuffer->Unlock();

		if ( FAILED(hr) )
			return false;

		m_hwIndexBufferData.reset();
	}

	// We can assume that we've successfully uploaded the vertex buffer
	m_bUploaded = true;

	return true;
}

//
// UploadBuffers
//
bool DX8VertexBuffer::UploadBuffers()
{
	
	// For static buffers, this only needs to happen once.
	if ( true == m_bUploaded )
		return true;

	// Lock the Buffer to get a pointer to the raw vertex buffer
	if ( false == Lock() )
		return false;

	// Packs the vertex and index buffers
	PackVertexBuffer();
	PackIndexBuffer();

	// Unlock the Buffer
	if ( false == Unlock() )
		return false;

	return true;
}

//
// SetNativeVB
//
void DX8VertexBuffer::SetNativeVB(IDirect3DVertexBuffer8 * pDX8VB)
{
	m_apDX8VertexBuffer = pDX8VB;
}

//
// SetNativeIndices
//
void DX8VertexBuffer::SetNativeIndices(IDirect3DIndexBuffer8 * pDX8IB)
{
	m_apDX8IndexBuffer = pDX8IB;
}

//
// GetNativeVB
//
IDirect3DVertexBuffer8 * DX8VertexBuffer::GetNativeVB()
{
	return m_apDX8VertexBuffer;
}

//
// GetNativeIndices
//
IDirect3DIndexBuffer8 * DX8VertexBuffer::GetNativeIndices()
{
	return m_apDX8IndexBuffer;
}

//
// GetVertexStride
//
unsigned int DX8VertexBuffer::GetVertexStride() const
{
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | NORMALS | COLOR)) )
		return sizeof(VertexFormat);

	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | COLOR)) )
		return sizeof(VertexFormat_PCT);

	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | TEXTURE_1 | TEXTURE_2 | NORMALS)) )
		return sizeof(VertexFormat_PNT3);

	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | TEXTURE_1 | NORMALS)) )
		return sizeof(VertexFormat_PNT2);

	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | NORMALS | TANGENTS)) )
		return sizeof(VertexFormat_PNTB);

	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | NORMALS)) )
		return sizeof(VertexFormat_PNT);

	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | TEXTURE_1 | TEXTURE_2)) )
		return sizeof(VertexFormat_PT3);

	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | TEXTURE_1)) )
		return sizeof(VertexFormat_PT2);

	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0)) )
		return sizeof(VertexFormat_PT);

	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | NORMALS)) )
		return sizeof(VertexFormat_PN);

	if ( CHECK_FLAG(m_enabledBuffers, VERTEX) )
		return sizeof(VertexFormat_P);

	return 0;
}

//
// Cleanup
//
void DX8VertexBuffer::Cleanup()
{
	m_apDX8VertexBuffer = 0;
	m_apDX8IndexBuffer = 0;
}

//
// PackVertexBuffer
//
void DX8VertexBuffer::PackVertexBuffer()
{
	// Check if the vb is valid
	if ( !m_apDX8VertexBuffer.isValid() )
		return;

	// Order is important
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | NORMALS | COLOR)) )
		PackVertexBuffer_PTNC();
	else
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | COLOR)) )
		PackVertexBuffer_PTC();
	else
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | TEXTURE_1 | TEXTURE_2 | NORMALS)) )
		PackVertexBuffer_PTN3();
	else
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | TEXTURE_1 | NORMALS)) )
		PackVertexBuffer_PTN2();
	else
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | NORMALS | TANGENTS)) )
		PackVertexBuffer_PTNB();
	else
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | NORMALS)) )
		PackVertexBuffer_PTN();
	else
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | TEXTURE_1 | TEXTURE_2)) )
		PackVertexBuffer_PT3();
	else
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0 | TEXTURE_1)) )
		PackVertexBuffer_PT2();
	else
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | TEXTURE_0)) )
		PackVertexBuffer_PT();
	else
	if ( CHECK_FLAG(m_enabledBuffers, (VERTEX | NORMALS)) )
		PackVertexBuffer_PN();
	else
	if ( CHECK_FLAG(m_enabledBuffers, VERTEX) )
		PackVertexBuffer_P();
}

//
// PackVertexBuffer_*
//
void DX8VertexBuffer::PackVertexBuffer_P()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat_P * pData = &getVertexBufferData<VertexFormat_P>()[0];

	// We only care about the positional element, a memcpy should suffice
	memcpy(pData, &(*m_vertexBuffer.get())[0], getActiveVertexCount() * GetVertexStride());
}

void DX8VertexBuffer::PackVertexBuffer_PT()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat_PT * pData = &getVertexBufferData<VertexFormat_PT>()[0];

	for (unsigned int uiIndex = 0; uiIndex < getActiveVertexCount(); uiIndex++)
	{
		pData->Position = D3DXVECTOR3(m_vertexBuffer->at( (uiIndex*3)+0 ),
									  m_vertexBuffer->at( (uiIndex*3)+1 ),
									  m_vertexBuffer->at( (uiIndex*3)+2 ));

		pData->Texture =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									  m_texture0Buffer->at( (uiIndex*2)+1 ));
		pData++;
	}
}

void DX8VertexBuffer::PackVertexBuffer_PN()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat_PN * pData = &getVertexBufferData<VertexFormat_PN>()[0];

	for (unsigned int uiIndex = 0; uiIndex < getActiveVertexCount(); uiIndex++)
	{
		pData->Position = D3DXVECTOR3(m_vertexBuffer->at( (uiIndex*3)+0 ),
									  m_vertexBuffer->at( (uiIndex*3)+1 ),
									  m_vertexBuffer->at( (uiIndex*3)+2 ));

		pData->Normal = D3DXVECTOR3(m_normalBuffer->at( (uiIndex*3)+0 ),
									m_normalBuffer->at( (uiIndex*3)+1 ),
									m_normalBuffer->at( (uiIndex*3)+2 ));

		pData++;
	}
}

void DX8VertexBuffer::PackVertexBuffer_PT2()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat_PT2 * pData = &getVertexBufferData<VertexFormat_PT2>()[0];

	for (unsigned int uiIndex = 0; uiIndex < getActiveVertexCount(); uiIndex++)
	{
		pData->Position = D3DXVECTOR3(m_vertexBuffer->at( (uiIndex*3)+0 ),
									  m_vertexBuffer->at( (uiIndex*3)+1 ),
									  m_vertexBuffer->at( (uiIndex*3)+2 ));

		pData->Texture0 =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									   m_texture0Buffer->at( (uiIndex*2)+1 ));

		pData->Texture1 =  D3DXVECTOR2(m_texture1Buffer->at( (uiIndex*2)+0 ),
									   m_texture1Buffer->at( (uiIndex*2)+1 ));

		pData++;
	}
}

void DX8VertexBuffer::PackVertexBuffer_PT3()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat_PT3 * pData = &getVertexBufferData<VertexFormat_PT3>()[0];

	for (unsigned int uiIndex = 0; uiIndex < getActiveVertexCount(); uiIndex++)
	{
		pData->Position = D3DXVECTOR3(m_vertexBuffer->at( (uiIndex*3)+0 ),
									  m_vertexBuffer->at( (uiIndex*3)+1 ),
									  m_vertexBuffer->at( (uiIndex*3)+2 ));

		pData->Texture0 =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									   m_texture0Buffer->at( (uiIndex*2)+1 ));

		pData->Texture1 =  D3DXVECTOR2(m_texture1Buffer->at( (uiIndex*2)+0 ),
									   m_texture1Buffer->at( (uiIndex*2)+1 ));

		pData->Texture2 =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									   m_texture0Buffer->at( (uiIndex*2)+1 ));

		pData++;
	}
}

void DX8VertexBuffer::PackVertexBuffer_PTN()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat_PNT * pData = &getVertexBufferData<VertexFormat_PNT>()[0];

	for (unsigned int uiIndex = 0; uiIndex < getActiveVertexCount(); uiIndex++)
	{
		pData->Position = D3DXVECTOR3(m_vertexBuffer->at( (uiIndex*3)+0 ),
									  m_vertexBuffer->at( (uiIndex*3)+1 ),
									  m_vertexBuffer->at( (uiIndex*3)+2 ));

		pData->Texture =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									  m_texture0Buffer->at( (uiIndex*2)+1 ));

		pData->Normal =   D3DXVECTOR3(m_normalBuffer->at( (uiIndex*3)+0 ),
									  m_normalBuffer->at( (uiIndex*3)+1 ),
									  m_normalBuffer->at( (uiIndex*3)+2 ));

		pData++;
	}
}

void DX8VertexBuffer::PackVertexBuffer_PTN2()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat_PNT2 * pData = &getVertexBufferData<VertexFormat_PNT2>()[0];

	for (unsigned int uiIndex = 0; uiIndex < getActiveVertexCount(); uiIndex++)
	{
		pData->Position = D3DXVECTOR3(m_vertexBuffer->at( (uiIndex*3)+0 ),
									  m_vertexBuffer->at( (uiIndex*3)+1 ),
									  m_vertexBuffer->at( (uiIndex*3)+2 ));

		pData->Texture0 =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									   m_texture0Buffer->at( (uiIndex*2)+1 ));

		pData->Texture1 =  D3DXVECTOR2(m_texture1Buffer->at( (uiIndex*2)+0 ),
									   m_texture1Buffer->at( (uiIndex*2)+1 ));

		pData->Normal =   D3DXVECTOR3(m_normalBuffer->at( (uiIndex*3)+0 ),
									  m_normalBuffer->at( (uiIndex*3)+1 ),
									  m_normalBuffer->at( (uiIndex*3)+2 ));

		pData++;
	}
}

void DX8VertexBuffer::PackVertexBuffer_PTN3()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat_PNT3 * pData = &getVertexBufferData<VertexFormat_PNT3>()[0];

	for (unsigned int uiIndex = 0; uiIndex < getActiveVertexCount(); uiIndex++)
	{
		pData->Position = D3DXVECTOR3(m_vertexBuffer->at( (uiIndex*3)+0 ),
									  m_vertexBuffer->at( (uiIndex*3)+1 ),
									  m_vertexBuffer->at( (uiIndex*3)+2 ));

		pData->Texture0 =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									   m_texture0Buffer->at( (uiIndex*2)+1 ));

		pData->Texture1 =  D3DXVECTOR2(m_texture1Buffer->at( (uiIndex*2)+0 ),
									   m_texture1Buffer->at( (uiIndex*2)+1 ));

		pData->Texture2 =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									   m_texture0Buffer->at( (uiIndex*2)+1 ));

		pData->Normal =   D3DXVECTOR3(m_normalBuffer->at( (uiIndex*3)+0 ),
									  m_normalBuffer->at( (uiIndex*3)+1 ),
									  m_normalBuffer->at( (uiIndex*3)+2 ));

		pData++;
	}
}

void DX8VertexBuffer::PackVertexBuffer_PTC()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat * pData = &getVertexBufferData<VertexFormat>()[0];

	for (unsigned int uiIndex = 0; uiIndex < getActiveVertexCount(); uiIndex++)
	{
		pData->Position = D3DXVECTOR3(m_vertexBuffer->at( (uiIndex*3)+0 ),
									  m_vertexBuffer->at( (uiIndex*3)+1 ),
									  m_vertexBuffer->at( (uiIndex*3)+2 ));

		pData->Texture =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									  m_texture0Buffer->at( (uiIndex*2)+1 ));

		unsigned char r = (unsigned char)m_colorBuffer->at( (uiIndex*4)+0 ) * 255;
		unsigned char g = (unsigned char)m_colorBuffer->at( (uiIndex*4)+1 ) * 255;
		unsigned char b = (unsigned char)m_colorBuffer->at( (uiIndex*4)+2 ) * 255;
		unsigned char a = (unsigned char)m_colorBuffer->at( (uiIndex*4)+3 ) * 255;

		pData->Diffuse = (a << 24 | b << 16 | g << 8 | r );

		pData++;
	}
}

void DX8VertexBuffer::PackVertexBuffer_PTNC()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat * pData = &getVertexBufferData<VertexFormat>()[0];

	for (unsigned int uiIndex = 0; uiIndex < getActiveVertexCount(); uiIndex++)
	{
		pData->Position = D3DXVECTOR3(m_vertexBuffer->at( (uiIndex*3)+0 ),
									  m_vertexBuffer->at( (uiIndex*3)+1 ),
									  m_vertexBuffer->at( (uiIndex*3)+2 ));

		pData->Texture =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									  m_texture0Buffer->at( (uiIndex*2)+1 ));

		pData->Normal =   D3DXVECTOR3(m_normalBuffer->at( (uiIndex*3)+0 ),
									  m_normalBuffer->at( (uiIndex*3)+1 ),
									  m_normalBuffer->at( (uiIndex*3)+2 ));

		// TODO: Prepack the float Colors (RGBA) into a DWORD during
		//		 Mesh::AddColor().

		unsigned char r = (unsigned char)m_colorBuffer->at( (uiIndex*4)+0 ) * 255;
		unsigned char g = (unsigned char)m_colorBuffer->at( (uiIndex*4)+1 ) * 255;
		unsigned char b = (unsigned char)m_colorBuffer->at( (uiIndex*4)+2 ) * 255;
		unsigned char a = (unsigned char)m_colorBuffer->at( (uiIndex*4)+3 ) * 255;

		pData->Diffuse = (a << 24 | b << 16 | g << 8 | r );

		pData++;
	}
}

void DX8VertexBuffer::PackVertexBuffer_PTNB()
{
	// Local Buffer; used mainly for type safety.
	VertexFormat_PNTB * pData = &getVertexBufferData<VertexFormat_PNTB>()[0];

	for (unsigned int uiIndex = 0; uiIndex < getActiveVertexCount(); uiIndex++)
	{
		pData->Position = D3DXVECTOR3(m_vertexBuffer->at( (uiIndex*3)+0 ),
									  m_vertexBuffer->at( (uiIndex*3)+1 ),
									  m_vertexBuffer->at( (uiIndex*3)+2 ));

		pData->Texture =  D3DXVECTOR2(m_texture0Buffer->at( (uiIndex*2)+0 ),
									  m_texture0Buffer->at( (uiIndex*2)+1 ));

		pData->Normal =   D3DXVECTOR3(m_normalBuffer->at( (uiIndex*3)+0 ),
									  m_normalBuffer->at( (uiIndex*3)+1 ),
									  m_normalBuffer->at( (uiIndex*3)+2 ));

		pData->T  = D3DXVECTOR3( m_tangentBasisS->at( (uiIndex*3)+0 ),
								 m_tangentBasisS->at( (uiIndex*3)+1 ),
								 m_tangentBasisS->at( (uiIndex*3)+2 ));

		pData->B  = D3DXVECTOR3( m_tangentBasisT->at( (uiIndex*3)+0 ),
								 m_tangentBasisT->at( (uiIndex*3)+1 ),
								 m_tangentBasisT->at( (uiIndex*3)+2 ));

		pData->N = D3DXVECTOR3( m_tangentBasisST->at( (uiIndex*3)+0 ),
								m_tangentBasisST->at( (uiIndex*3)+1 ),
								m_tangentBasisST->at( (uiIndex*3)+2 ));
		pData++;
	}
}

//
// PackIndexBuffer
//
void DX8VertexBuffer::PackIndexBuffer()
{
	// Is the index buffer valid
	if ( !m_apDX8IndexBuffer.isValid() )
		return;

	// Index buffer is enabled?
	if ( false == isBufferEnabled(INDEX) )
		return;

	// Grab the Total Index Count
	unsigned int uiIndexCount = m_indexCount;

	// If the IndexCount is greater than the Buffer, this is probably
	// a dynamic Index buffer, set the active index count
	if ( uiIndexCount > m_indexBuffer->size() )
		uiIndexCount = getActiveIndexCount();

	// Store the Index Data inside the buffer
	for (unsigned int uiIndex = 0; uiIndex < uiIndexCount; uiIndex++)
	{
		m_hwIndexBufferData[uiIndex] = m_indexBuffer->at(uiIndex);
	}
}