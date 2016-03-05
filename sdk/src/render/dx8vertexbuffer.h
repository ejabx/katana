/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8vertexbuffer.h
	Author:		Eric Bryant

	DX8 Vertex Buffer
*/

#ifndef _DX8VERTEXBUFFER_H
#define _DX8VERTEXBUFFER_H

namespace Katana
{


typedef struct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Normal;
	DWORD		Diffuse;
	D3DXVECTOR2 Texture;

} VertexFormat;

typedef struct
{
	D3DXVECTOR3 Position;

} VertexFormat_P;

typedef struct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Texture;

} VertexFormat_PT;

typedef struct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Normal;

} VertexFormat_PN;

typedef struct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Normal;
	D3DXVECTOR2 Texture;

} VertexFormat_PNT;

typedef struct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Normal;
	D3DXVECTOR2 Texture;
	D3DXVECTOR3 T;
	D3DXVECTOR3 B;
	D3DXVECTOR3 N;

} VertexFormat_PNTB;

typedef struct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Normal;
	D3DXVECTOR2 Texture0;
	D3DXVECTOR2 Texture1;

} VertexFormat_PNT2;

typedef struct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Normal;
	D3DXVECTOR2 Texture0;
	D3DXVECTOR2 Texture1;
	D3DXVECTOR2 Texture2;

} VertexFormat_PNT3;

typedef struct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Texture0;
	D3DXVECTOR2 Texture1;

} VertexFormat_PT2;

typedef struct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Texture0;
	D3DXVECTOR2 Texture1;
	D3DXVECTOR2 Texture2;

} VertexFormat_PT3;

typedef struct
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 Texture;
	DWORD		Diffuse;

} VertexFormat_PCT;


//
// DX8VertexBuffer
//
class DX8VertexBuffer : public VertexBuffer
{
public:
	/// Constructor
	DX8VertexBuffer(unsigned int uiVertexCount,
					unsigned int uiIndexCount,
					BufferTypes eEnabledBuffers,
					BufferCreationFlags eCreationFlags);

	/// Destructor
	virtual ~DX8VertexBuffer();

	// Initialize the vertex buffer as necessary
	virtual bool Initialize();

	// Lock/Unlock the vertex buffers
	virtual bool Lock();
	virtual bool LockRange(unsigned int uiStartVertex, unsigned int uiVertexCount, unsigned int uiStartIndex = 0,  unsigned int uiIndexCount = 0);
	virtual bool Unlock();

	// Uploads the arrays to the render device (ogl, dx8)
	virtual bool UploadBuffers();

public: // Public Interfaces

	/// Sets the internal DirectX8 vertex buffer
	void SetNativeVB(IDirect3DVertexBuffer8 * pDX8VB);

	/// Sets the internal DirectX8 index buffer
	void SetNativeIndices(IDirect3DIndexBuffer8 * pDX8IB);

	/// Retrieves the internal DirectX8 vertex buffer
	IDirect3DVertexBuffer8 * GetNativeVB();

	/// Retrieves the internal DirectX8 index buffer
	IDirect3DIndexBuffer8 * GetNativeIndices();

	/// Returns the size of one FVF structure
	unsigned int GetVertexStride() const;

private:
	/// Cleanup the vertex buffer
	void Cleanup();

	/// Packs the vertex buffer into a flat interleaved array
	void PackVertexBuffer();

	/// Packs flexible vertex formats
	void PackVertexBuffer_P();				// (Position)
	void PackVertexBuffer_PT();				// (Position, Texture)
	void PackVertexBuffer_PN();				// (Position, Normal)
	void PackVertexBuffer_PT2();			// (Position, Texture0, Texture1)
	void PackVertexBuffer_PT3();			// (Position, Texture0, Texture1, Texture2)
	void PackVertexBuffer_PTN();			// (Position, Texture, Normal)
	void PackVertexBuffer_PTN2();			// (Position, Texture0, Texture1, Normal)
	void PackVertexBuffer_PTN3();			// (Position, Texture0, Texture1, Texture2, Normal)
	void PackVertexBuffer_PTC();			// (Position, Texture, Color)
	void PackVertexBuffer_PTNC();			// (Position, Texture, Normal, Color)
	void PackVertexBuffer_PTNB();			// (Position, Texture, Normal, TangentS, TangentT, TangentST);

	/// Packs the index buffer into a flat array
	void PackIndexBuffer();

private:
	/// Pointer to internal vertex buffer
	ComPtr<IDirect3DVertexBuffer8>	m_apDX8VertexBuffer;
	
	/// Pointer to internal index buffer
	ComPtr<IDirect3DIndexBuffer8>	m_apDX8IndexBuffer;
};

} // Katana

#endif // _DX8VERTEXBUFFER_H