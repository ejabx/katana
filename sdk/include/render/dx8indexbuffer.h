/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8indexbuffer.h
	Author:		Eric Bryant

	DX8 Index Buffer
*/

#ifndef _DX8INDEXBUFFER_H
#define _DX8INDEXBUFFER_H

namespace Katana
{

class DX8IndexBuffer : public IndexBuffer
{
public:
	/// Constructor
	DX8IndexBuffer(unsigned int uiIndexCount, BufferCreationFlags eCreationFlags);

	/// Destructor
	virtual ~DX8IndexBuffer();

	/// Sets the internal DirectX8 index buffer
	void SetNativeIndices(IDirect3DIndexBuffer8 * pDX8IB);

	/// Retrieves the internal DirectX8 index buffer
	IDirect3DIndexBuffer8 * GetNativeIndices();

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
	ComPtr<IDirect3DIndexBuffer8>	m_apDX8IndexBuffer;
};

}; // Katana

#endif // _DX8INDEXBUFFER_H