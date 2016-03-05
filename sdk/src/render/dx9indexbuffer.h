/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		dx9indexbuffer.h
	Author:		Eric Bryant

	DX9 Index Buffer
*/

#ifndef _DX9INDEXBUFFER_H
#define _DX9INDEXBUFFER_H

namespace Katana
{

class DX9IndexBuffer : public IndexBuffer
{
public:
	/// Constructor
	DX9IndexBuffer( unsigned int uiIndexCount, BufferCreationFlags eCreationFlags );

	/// Destructor
	virtual ~DX9IndexBuffer();

	/// Sets the internal DirectX8 index buffer
	void SetNativeIndices(IDirect3DIndexBuffer9 * pDX8IB);

	/// Retrieves the internal DirectX8 index buffer
	IDirect3DIndexBuffer9 * GetNativeIndices();

	/// Initialize the vertex buffer as necessary
	virtual bool Initialize();

	/// Lock/Unlock the vertex buffers
	virtual bool Lock();
	virtual bool LockRange(unsigned int uiStartIndex, unsigned int uiIndexCount);
	virtual bool Unlock();

	/// Uploads the arrays to the render device (ogl, dx8)
	virtual bool UploadBuffers();

private:
	/// Cleanup the index buffer
	void Cleanup();

	/// Packs the index buffer into a flat array
	void PackIndexBuffer();

private:
	/// Pointer to internal index buffer
	ComPtr<IDirect3DIndexBuffer9>	m_apDX9IndexBuffer;
};

}; // Katana

#endif // _DX9INDEXBUFFER_H