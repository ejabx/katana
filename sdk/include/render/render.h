/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		render.h
	Author:		Eric Bryant

	Defines the interface for any render device driver
*/

#ifndef _RENDER_H
#define _RENDER_H

namespace Katana
{

// Forward Declarations
class Camera;
class ColorA;
class RenderState;
class Texture;
class TriangleList;
class VertexBuffer;
class IndexBuffer;
class Matrix4;
struct Geometry;

/// Enumeration of matrix types
enum MatrixType
{
	TEXTURE,
	MODELVIEW,
	PROJECTION,
};

///
/// Render
/// Pure Interface for any renderable device
class Render
	: public enable_shared_from_this<Render>
{
	KDECLARE_SCRIPT;

public:
	/// Constructor
	Render();

	/// Destructor
	virtual ~Render();

	/// Startup the renderer
	virtual bool Initialize(RenderInfo & Info)=0;

	/// Shutdown the renderer
	virtual bool Shutdown()=0;

	/// Begin frame (analogous to OGL glBegin/glEnd)
	virtual bool BeginFrame()=0;

	/// End frame (analogous to OGL glBegin/glEnd)
	virtual bool EndFrame()=0;

	/// Setup a new viewport
	virtual bool SetViewport(Camera & Cam)=0;

	/// Setup the parameters of a matrix
	virtual bool SetMatrix(MatrixType eType, MatrixFunction eFunct, const Matrix4 & Mat)=0;

	/// Sets a render state (alpha blend, etc.)
	virtual bool SetState(RenderState * pState)=0;

	/// Renders a geometry primitive
	virtual bool RenderGeometry(Geometry * geom)=0;

	/// Renders a Vertex Buffer
	virtual bool RenderVB(VertexBuffer * pVB)=0;

	/// Renders a Vertex Buffer with an explicit Index Buffer
	virtual bool RenderVB(VertexBuffer * pVB, IndexBuffer * pIB)=0;

	/// Creates a blank vertex buffer
	virtual VertexBuffer * CreateVB(BufferTypes eEnabledBuffers = VERTEX | TEXTURE_0 | INDEX,
									BufferCreationFlags eCreationFlags = STATIC | WRITE_ONLY,
									unsigned int uiVertexCount = 512,
									unsigned int uiIndexCount = 512)=0;

	/// Creates a blank index buffer
	virtual IndexBuffer *  CreateIB(BufferCreationFlags eCreationFlags = STATIC | WRITE_ONLY,
									unsigned int uiIndexCount = 512)=0;

	/// Binds the texture for the next render pass
	virtual bool BindTexture(Texture * pTexture)=0;

	/// Grabs an image from the current screen buffer
	virtual bool GrabScreenBuffer(char * pBuffer, unsigned int uiLeft, unsigned uiTop, unsigned uiRight, unsigned int uiBottom)=0;

	/// Sets the color of the background
	virtual void SetBackgroundColor(ColorA & color)=0;

public:
	enum RenderErrorValues
	{
		STATUS_OK,
		INITIALIZE_FAILED,
		STATE_MANAGER_FAILED,
		UNKNOWN_ERROR,
		UNABLE_TO_CREATE_RENDERER,
		UNABLE_TO_SET_RENDER_STATE,
		UNABLE_TO_LOCK_BUFFER,
		UNABLE_TO_CREATE_VB,
		UNABLE_TO_CLEAR_BUFFER,
		UNABLE_TO_SWAP_BUFFERS,
		UNABLE_TO_BIND_TEXTURE,
		UNABLE_TO_RENDER_PRIMITIVE,
		UNABLE_TO_CONFIGURE_TEXTURE_PASS,
	};

public:

	/// Creates a render with the specified parameters, clients must
	/// use this method to create a valid renderer.
	static Render * CreateRenderer(RenderInfo & Info);

	/// Get the last error
	static RenderErrorValues GetLastError();
	static RenderErrorValues GetLastError(std::string & strErrorDescription);

	/// Get the render info
	const RenderInfo & GetRenderInfo();

protected:

	/// Sets the current error condition
	static void SetError(RenderErrorValues eError, const char * szErrorString);

protected:
	/// Has the renderer been initialized properly
	bool m_isInitialized;	

	/// Information about the render driver
	RenderInfo	m_RenderInfo;

	/// Last Error Message
	static std::string			m_strLastErrorString;
	static RenderErrorValues	m_eLastError;

	/// Pointer to current renderer
	static Render *			m_pCurrentRenderer;

};

KIMPLEMENT_SCRIPT( Render );

}; // Katana

#endif // _RENDER_H