/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		vertexbuffer.h
	Author:		Eric Bryant

	Interface class for all vertex buffers. This class is unusable
	by itself, it must be derived by a class which implements
	the pure methods, like Lock(), Unlock(), which is driver specific,
	e.g., opengl vs. directx8.
*/

#ifndef _VERTEXBUFFER_H
#define _VERTEXBUFFER_H

namespace Katana
{

//
// Forward Declarations
//
struct Geometry;

///
/// VertexBuffer
/// Interface class for all vertex buffers. It derives from
/// Geometry because it could be considered "renderable", or rather,
/// "render-specific" Geometry. Also note it is protected inheritance,
/// so clients cannot direectly access the internals of Geometry
/// (which could violate the integrity of the vertex buffer).
///
class VertexBuffer : protected Geometry
{
public:
	enum CONSTANTS
	{
		VERTEX_STRIDE = 3,
		TEXTURE_STRIDE = 2,
		COLOR_STRIDE = 4,
		NORMAL_STRIDE = 3,
		INDEX_STRIDE = 1,

		DEFAULT_VERTEX_COUNT = 512,
		DEFAULT_INDEX_COUNT = 512,
	};

public:
	/// Constructor
	VertexBuffer(unsigned int uiVertexCount = DEFAULT_VERTEX_COUNT,
				 unsigned int uiIndexCount = DEFAULT_INDEX_COUNT,
				 BufferTypes eEnabledBuffers = VERTEX | TEXTURE_0 | INDEX,
				 BufferCreationFlags eCreationFlags = STATIC | WRITE_ONLY);

	/// Destructor
	virtual ~VertexBuffer();

	/// Sets the primitive count
	void setPrimitveCount(unsigned int primitiveCount)			{ m_primitiveCount = primitiveCount; }

	/// Sets the active vertex count
	void setActiveVertexCount(unsigned int vertexCount)			{ m_activeIndexCount = vertexCount; }

	/// Sets the active index count
	void setActiveIndexCount(unsigned int indexCount)			{ m_activeIndexCount = indexCount; }

	/// Sets the primitive type
	void setPrimitveType(PrimitiveType eType)					{ m_primitiveType = eType; }

	/// Retrieves the primitive type
	PrimitiveType getPrimitiveType() const						{ return m_primitiveType; }

	/// Allows you to send multiple triangle strips to the renderer
	void setTriangleStripCount(unsigned int strips)				{ m_triangleStripCount = strips; }

	/// Retrieve the triangle strip count
	unsigned int getTriangleStripCount() const					{ return m_triangleStripCount; }

	/// Sets the vertex offset
	void setVertexOffset( unsigned int offset )					{ m_vertexOffset = offset; }

	/// Sets the index offset
	void setIndexOffset( unsigned int offset )					{ m_indexOffset = offset; }

	/// Enable buffers
	bool enableBuffer(BufferTypes eEnabledBuffers);

	/// Disable the buffers
	bool disableBuffer(BufferTypes eEnabledBuffers);

	/// Is a buffer enabled?
	bool isBufferEnabled(BufferTypes eBuffer)					{ return ( ( m_enabledBuffers & eBuffer ) == eBuffer ); }

	/// Sets the contents of the VB as dirty, it will be Upload during
	/// the next Render::RenderVB(). This is significant for STATIC
	/// buffers which only Upload it's vertex data once.
	void setDirty( bool dirty = true )							{ m_bUploaded = !dirty; }

	/// Retrieve a safe array pointing to the hardware vertex buffer. It is assumed the client has sets this variable
	/// during Lock() and resets it during Unlock().
	template <typename T>
	ksafearray<T>	  	getVertexBufferData()					{ return m_hwVertexBufferData; }

	/// Retrieve a safe array pointing to the hardware index buffer. It is assumed the client has sets this variable
	/// during Lock() and resets it during Unlock().
	ksafearray<short>   getIndexBufferData()					{ return m_hwIndexBufferData; }

	/// Get information about the vertex buffer
	unsigned int		getPrimitiveCount() const				{ return m_primitiveCount; }
	unsigned int		getVertexCount() const					{ return m_vertexCount; }
	unsigned int		getIndexCount() const					{ return m_indexCount; }
	unsigned int		getActiveVertexCount() const			{ return m_activeVertexCount; }
	unsigned int		getActiveIndexCount() const				{ return m_activeIndexCount; }
	unsigned int		getIndexOffset() const					{ return m_indexOffset; }
	unsigned int		getVertexOffset() const					{ return m_vertexOffset; }
	BufferTypes			getEnabledBuffers() const				{ return m_enabledBuffers; }
	BufferCreationFlags getCreationFlags() const				{ return m_creationFlags; }
	bool				isLocked() const						{ return m_isLocked; }
	bool				isDirty() const							{ return !m_bUploaded; }

	/// Grow the arrays as needed (NOTE: Use this method to grow the
	/// arrays, never use FloatArray's SetSize(), this class will
	/// be unaware of the new array size).
	void growBuffers(unsigned int uiVertexCount, unsigned int uiIndexCount = 0);

	/// Shares existing buffers from a geometry object with this vertex buffer
	void shareBuffers(const Geometry & geometry);

public:
	/// Returns a pointer to the RTTI object. Protected inheritance hides Geometry's version
	/// so we provide a proxy here.
	const RTTI_TYPE * GetRTTI() { return Geometry::GetRTTI(); }

public:
	/*
		The following pure virtual functions must be implemented by
		the vertex buffer driver (opengl or directx)
	*/

	/// Initialize the vertex buffer as necessary
	virtual bool Initialize()=0;

	/// Lock/Unlock the vertex buffers
	virtual bool Lock()=0;
	virtual bool LockRange(unsigned int uiStartVertex, unsigned int uiVertexCount, unsigned int uiStartIndex = 0,  unsigned int uiIndexCount = 0) = 0;
	virtual bool Unlock()=0;

	/// Uploads the buffer arrays to the locked arrays.
	virtual bool UploadBuffers()=0;

protected:
	/// Enable client states (if applicable)
	virtual bool EnableClientStates(BufferTypes eEnabledBuffers);

	/// Disable client states (if applicable)
	virtual bool DisableClientStates(BufferTypes eEnabledBuffers);

	/// Create the Vertex Buffer Arrays
	void CreateBuffers();

protected:
	/// Vertex buffer creation flags
	BufferCreationFlags m_creationFlags;

	/// Are the arrays locked?
	bool			m_isLocked;

	/// Has the VB already been uploaded to the device?
	/// Static VB only need to be uploaded once because
	/// they keep a reference to the native VB (OGL/DX8)
	bool			m_bUploaded;

	/// Current buffer size
	unsigned int	m_activeVertexCount;	/// For Dynamic VBs
	unsigned int	m_activeIndexCount;		/// For Dynamic VBs
	unsigned int	m_vertexOffset;			/// Offset into the vertex buffer to start rendering
	unsigned int	m_indexOffset;			/// Offset into the index buffer to start rendering

	/// Pointer to the internal vertex buffer. This pointer is set during Lock() and released during Unlock().
	/// Clients can use this class to upload data to the vertex buffer directly instead of relying on UploadBuffers().
	ksafearray<char>	m_hwVertexBufferData;

	/// Safe pointer to the internal index buffer. This pointer is set during Lock() and released during Unlock().
	/// Clients can use this class to upload data to the index buffer directly instead of relying on UploadBuffers().
	ksafearray<unsigned short>	m_hwIndexBufferData;
};

}; // Katana

#endif //  _VERTEXBUFFER_H