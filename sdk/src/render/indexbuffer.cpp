/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		indexbuffer.cpp
	Author:		Eric Bryant

	Stores the Indices of a Mesh, it is only used for the purpose
	of sharing index buffers with vertex buffers, in order to
	render a vertex buffer, you must use Render::CreateIB() which
	will give you a concrete IndexBuffer.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "rendertypes.h"
#include "geometry.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"

//
// Constructor
//
IndexBuffer::IndexBuffer()
{
	CreateIndexBuffers();
}

IndexBuffer::IndexBuffer(unsigned int uiIndexCount, BufferCreationFlags eCreationFlags) :
	m_indexCount(uiIndexCount),
	m_activeIndexCount(uiIndexCount),
	m_CreationFlags(eCreationFlags)
{
	CreateIndexBuffers();
	growBuffers(uiIndexCount);
}

//
// Destructor
//
IndexBuffer::~IndexBuffer()
{
}

//
// CreateIndexBuffer
//
void IndexBuffer::CreateIndexBuffers()
{
	m_spIndexBuffer.reset( new vector<short> );
}

//
// GrowBuffers
//
void IndexBuffer::growBuffers(unsigned int uiIndexCount)
{
	m_spIndexBuffer->reserve(uiIndexCount);
}

//
// addIndex
//
void IndexBuffer::addIndex(short sIndex)
{
	if ( !m_spIndexBuffer ) 
		CreateIndexBuffers();

	m_spIndexBuffer->push_back( sIndex );
}

//
// shareBuffer
//
void IndexBuffer::shareBuffer( shared_ptr< vector<short> > spBuffer )
{
	m_spIndexBuffer = spBuffer;
}