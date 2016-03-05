/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		vertexbuffer.cpp
	Author:		Eric Bryant

	Interface class for all vertex buffers. This class is unusable
	by itself, it must be derived by a class which implements
	the pure methods, like Lock(), Unlock(), which is driver specific,
	e.g., opengl vs. directx8.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "rendertypes.h"
#include "geometry.h"
#include "vertexbuffer.h"

//
// Constructor
//
VertexBuffer::VertexBuffer(unsigned int vertexCount,
						   unsigned int indexCount,
						   BufferTypes enabledBuffers, 
						   BufferCreationFlags creationFlags) 
	: m_activeVertexCount( vertexCount )
	, m_activeIndexCount( indexCount )
	, m_creationFlags( creationFlags )
	, m_isLocked( false )
	, m_bUploaded( false )
	, m_vertexOffset( 0 )
	, m_indexOffset( 0 )
{
	m_vertexCount = vertexCount;
	m_indexCount = indexCount;
	m_primitiveType = TRIANGLE_LIST;
	m_triangleStripCount = 1;
	m_enabledBuffers = enabledBuffers;

	CreateBuffers();
	growBuffers(vertexCount, indexCount);
}

//
// Destructor
//
VertexBuffer::~VertexBuffer()
{
	// NOTE: This function must be called in the client because during
	// the destructor, the derived class is already destroying, so calling
	// <DERIVED_CLASS>::Unlock will fail.
	//
	// Unlock();
}

//
// enableBuffer
//
bool VertexBuffer::enableBuffer(BufferTypes enabledBuffers)
{
	// Set the buffer flags
	m_enabledBuffers |= enabledBuffers;

	// First, Disable the Complement of the Client States
	// (to remove the existing states from a previous operation)
	DisableClientStates(~enabledBuffers);

	// Have the driver specific VB enable client states
	return EnableClientStates(m_enabledBuffers);
}

//
// disableBuffer
//
bool VertexBuffer::disableBuffer(BufferTypes disabledBuffers)
{
	// Unset the buffer flags
	m_enabledBuffers &= disabledBuffers;

	// Have the driver specific VB disable client states
	return DisableClientStates(m_enabledBuffers);
}

//
// EnableClientStates
//

bool VertexBuffer::EnableClientStates(BufferTypes eEnabledBuffers)
{
	return true; /* NOP */
}

//
// DisableClientStates
//
bool VertexBuffer::DisableClientStates(BufferTypes eEnabledBuffers)
{
	return true; /* NOP */
}

//
// CreateBuffer
//
void VertexBuffer::CreateBuffers()
{
	// Allocate the Arrays
	m_vertexBuffer.reset( new vector< float > );
	m_normalBuffer.reset(  new vector< float > );
	m_texture0Buffer.reset(  new vector< float > );
	m_texture1Buffer.reset(  new vector< float > );
	m_colorBuffer.reset(  new vector< float > );
	m_indexBuffer.reset(  new vector< unsigned short > );
}

//
// growBuffers
//
void VertexBuffer::growBuffers(unsigned int vertexCount, unsigned int indexCount)
{
	m_vertexCount = vertexCount;
	
	if ( isBufferEnabled(VERTEX) )
		m_vertexBuffer->reserve( vertexCount * VERTEX_STRIDE );

	if ( isBufferEnabled(NORMALS) )
		m_normalBuffer->reserve(vertexCount * NORMAL_STRIDE);

	if ( isBufferEnabled(TEXTURE_0) )
		m_texture0Buffer->reserve(vertexCount * TEXTURE_STRIDE);

	if ( isBufferEnabled(TEXTURE_1) )
		m_texture1Buffer->reserve(vertexCount * TEXTURE_STRIDE);

	if ( isBufferEnabled(COLOR) )
		m_colorBuffer->reserve(vertexCount * COLOR_STRIDE);
	
	if ( isBufferEnabled(INDEX) && ( indexCount > 0 ) )
	{		
		m_indexCount = indexCount;
		m_indexBuffer->reserve(indexCount * INDEX_STRIDE);
	}
}

//
// shareBuffers
//
void VertexBuffer::shareBuffers(const Geometry & geometry)
{
	m_vertexBuffer = geometry.m_vertexBuffer;
	m_normalBuffer = geometry.m_normalBuffer;
	m_texture0Buffer = geometry.m_texture0Buffer;
	m_texture1Buffer = geometry.m_texture1Buffer;
	m_colorBuffer = geometry.m_colorBuffer;
	m_indexBuffer = geometry.m_indexBuffer;
	m_tangentBasisS = geometry.m_tangentBasisS;
	m_tangentBasisT = geometry.m_tangentBasisT;
	m_tangentBasisST = geometry.m_tangentBasisST;
}