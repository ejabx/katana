/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		indexbuffer.h
	Author:		Eric Bryant

	Stores the Indices of a Mesh, it is only used for the purpose
	of sharing index buffers with vertex buffers, in order to
	render a vertex buffer, you must use Render::CreateIB() which
	will give you a concrete IndexBuffer.
*/

#ifndef _INDEXBUFFER_H
#define _INDEXBUFFER_H

namespace Katana
{

class IndexBuffer
{
public:
	/// Constructor
	IndexBuffer();

	/// Constructor with default index values
	IndexBuffer(unsigned int uiIndexCount, BufferCreationFlags eCreationFlags);

	/// Destructor
	virtual ~IndexBuffer();

	/// Sets the active index count
	void setActiveIndexCount(unsigned int uiIndexCount)			{ m_activeIndexCount = uiIndexCount; }

	/// Get information about the vertex buffer
	unsigned int getIndexCount() const							{ return m_indexCount; }
	unsigned int getActiveIndexCount() const					{ return m_activeIndexCount; }

	/// Retrieve a safe array pointing to the hardware index buffer. It is assumed the client has sets this variable
	/// during Lock() and resets it during Unlock().
	ksafearray<unsigned short>   getIndexBufferData()			{ return m_hwIndexBufferData; }

	/// Grow the arrays as needed (NOTE: Use this method to grow the
	/// arrays, never use UIntArray's SetSize(), this class will
	/// be unaware of the new array size).
	void growBuffers(unsigned int uiIndexCount);

	/// Adds a single index to the array
	void addIndex(short sIndex);

	/// Performs a shallow copy of the array simply by setting the new reference
	void shareBuffer(shared_ptr< vector<short> > apBuffer);

public:
	/// Initialize the vertex buffer as necessary
	virtual bool Initialize()=0;

	/// Lock/Unlock the vertex buffers
	virtual bool Lock()=0;
	virtual bool LockRange(unsigned int uiStartIndex, unsigned int uiIndexCount)=0;
	virtual bool Unlock()=0;

	/// Uploads the buffer arrays to the render device (ogl, dx8)
	virtual bool UploadBuffers()=0;

private:
	/// Create the Index Buffer Arrays
	void CreateIndexBuffers();

protected:
	// The Index Buffer
	shared_ptr< vector<short> >	 m_spIndexBuffer;

	// Current index buffer size
	unsigned int	m_indexCount;
	unsigned int	m_activeIndexCount;

	// Vertex buffer creation flags
	BufferCreationFlags m_CreationFlags;

	// Are the arrays locked?
	bool			m_bLocked;

	// Has the VB already been uploaded to the device?
	// Static VB only need to be uploaded once because
	// they keep a reference to the native VB (OGL/DX8)
	bool			m_bUploaded;

	/// Safe pointer to the internal index buffer. This pointer is set during Lock() and released during Unlock().
	/// Clients can use this class to upload data to the index buffer directly instead of relying on UploadBuffers().
	ksafearray<unsigned short>	m_hwIndexBufferData;
};

}; // Katana

#endif // _INDEXBUFFER_H