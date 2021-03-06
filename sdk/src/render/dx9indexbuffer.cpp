/*
	Katana Engine
	Copyright � 2001-2004 Eric Bryant, Inc.

	File:		dx9indexbuffer.cpp
	Author:		Eric Bryant

	DX9 Index Buffer
*/

#include <d3d9.h>

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "base/comptr.h"
#include "rendertypes.h"
#include "geometry.h"
#include "indexbuffer.h"
#include "vertexbuffer.h"
#include "rendertypes.h"
#include "dx9indexbuffer.h"

// -------------------------------------------
// Macros
// -------------------------------------------

#define CHECK_FLAG(var, flag) (var&flag) == flag

// -------------------------------------------
// DX9IndexBuffer
// -------------------------------------------

//
// Constructor
//
DX9IndexBuffer::DX9IndexBuffer(unsigned int uiIndexCount, BufferCreationFlags eCreationFlags) :
	IndexBuffer(uiIndexCount, eCreationFlags)
{
}

//
// Destructor
//
DX9IndexBuffer::~DX9IndexBuffer()
{
	// Unlock and cleanup the index buffer
	Unlock();
	Cleanup();
}

//
// Initialize
//
bool DX9IndexBuffer::Initialize()
{
	// Release the previous vertex buffer
	Cleanup();

	return true;
}

//
// Lock
//
bool DX9IndexBuffer::Lock()
{
	if ( !m_apDX9IndexBuffer.isValid() )
		return false;

	// Lock the entire Index Buffer
	unsigned short * pIndexData = 0;
	HRESULT hr = m_apDX9IndexBuffer->Lock( 0, 0, (void**)&pIndexData, 0 );
	if ( FAILED(hr) )
		return false;

	// Store the address of the locked index buffer
	m_hwIndexBufferData.set( pIndexData, getActiveIndexCount() );

	return true;
}

//
// LockRange
//
bool DX9IndexBuffer::LockRange(unsigned int uiStartIndex, unsigned int uiIndexCount)
{
	// Lock the Index Buffer
	unsigned short * pIndexData = 0;
	HRESULT hr = m_apDX9IndexBuffer->Lock( uiStartIndex * sizeof(unsigned short),
										   uiIndexCount * sizeof(unsigned short),
										   (void**)&pIndexData,
										   0 );
	if ( FAILED(hr) )
		return false;

	// Store the address of the locked index buffer
	m_hwIndexBufferData.set( pIndexData, uiIndexCount );

	return true;
}

//
// Unlock
//
bool DX9IndexBuffer::Unlock()
{
	if ( !m_apDX9IndexBuffer.isValid() )
		return false;

	HRESULT hr = m_apDX9IndexBuffer->Unlock();

	if ( FAILED(hr) )
		return false;

	m_hwIndexBufferData.reset();

	// We can assume that we've successfully uploaded the vertex buffer
	m_bUploaded = true;

	return true;
}

//
// UploadBuffers
//
bool DX9IndexBuffer::UploadBuffers()
{	
	// For static buffers, this only needs to happen once.
	if ( true == m_bUploaded )
		return true;

	// Lock the Buffer to get a pointer to the raw vertex buffer
	if ( false == Lock() )
		return false;

	// Packs the index buffers
	PackIndexBuffer();

	// Unlock the Buffer
	if ( false == Unlock() )
		return false;

	// We've successfully uploaded the vertex buffer
	m_bUploaded = true;

	return true;
}

//
// SetNativeIndices
//
void DX9IndexBuffer::SetNativeIndices(IDirect3DIndexBuffer9 * pDX8IB)
{
	m_apDX9IndexBuffer = pDX8IB;
}

//
// GetNativeIndices
//
IDirect3DIndexBuffer9 * DX9IndexBuffer::GetNativeIndices()
{
	return m_apDX9IndexBuffer;
}

//
// Cleanup
//
void DX9IndexBuffer::Cleanup()
{
	m_apDX9IndexBuffer = 0;
}

//
// PackIndexBuffer
//

void DX9IndexBuffer::PackIndexBuffer()
{
	if ( !m_apDX9IndexBuffer.isValid() )
		return;

	// Grab the Total Index Count
	unsigned int uiIndexCount = m_indexCount;

	// If the IndexCount is greater than the Buffer, this is probably
	// a dynamic Index buffer, set the active index count
	if ( uiIndexCount > m_spIndexBuffer->size() )
		uiIndexCount = getActiveIndexCount();

	// Store the Index Data inside the buffer
	for (unsigned int uiIndex = 0; uiIndex < uiIndexCount; uiIndex++)
	{
		m_hwIndexBufferData[uiIndex] = m_spIndexBuffer->at(uiIndex);
	}
}