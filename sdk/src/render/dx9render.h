/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		dx9render.h
	Author:		Eric Bryant

	Concrete Render class which supports DirectX9
*/

#ifndef _DX9RENDER_H
#define _DX9RENDER_H

//
// Forward Declarations
//
struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DVertexDeclaration9;

namespace Katana
{

//
// Forward Declarations
//
class DX9Enumeration;
class DX9TextureManager;
class DX9StateManager;

///
/// DX9Render
/// Concrete Render class which supports DirectX9
///
class DX9Render : public Render
{
public:
	/// Constructor
	DX9Render();

	/// Destructor
	virtual ~DX9Render();

	/// Startup the renderer
	virtual bool Initialize(RenderInfo & Info);

	/// Shutdown the renderer
	virtual bool Shutdown();

	/// Begin frame (analogous to OGL glBegin/glEnd)
	virtual bool BeginFrame();

	/// End frame (analogous to OGL glBegin/glEnd)
	virtual bool EndFrame();

	/// Setup a new viewport
	virtual bool SetViewport(Camera & Cam);

	/// Setup the parameters of a matrix
	virtual bool SetMatrix(MatrixType eType, MatrixFunction eFunct, const Matrix4 & Mat);

	/// Sets a render state (alpha blend, etc.)
	virtual bool SetState(RenderState * pState);

	/// Renders a geometry primitive
	virtual bool RenderGeometry(Geometry * geom);

	/// Renders a Vertex Buffer
	virtual bool RenderVB(VertexBuffer * pVB);

	/// Renders a Vertex Buffer with an explicit Index Buffer
	virtual bool RenderVB(VertexBuffer * pVB, IndexBuffer * pIB);

	/// Creates a blank vertex buffer
	virtual VertexBuffer * CreateVB(BufferTypes eEnabledBuffers = VERTEX | TEXTURE_0 | INDEX,
		BufferCreationFlags eCreationFlags = STATIC | WRITE_ONLY,
		unsigned int uiVertexCount = 512,
		unsigned int uiIndexCount = 512);

	/// Creates a blank index buffer
	virtual IndexBuffer *  CreateIB(BufferCreationFlags eCreationFlags = STATIC | WRITE_ONLY,
		unsigned int uiIndexCount = 512);

	/// Binds the texture for the next render pass
	virtual bool BindTexture(Texture * pTexture);

	/// Grabs an image from the current screen buffer
	virtual bool GrabScreenBuffer(char * pBuffer, unsigned int uiLeft, unsigned uiTop, unsigned uiRight, unsigned int uiBottom);

	/// Sets the color of the background
	virtual void SetBackgroundColor(ColorA & color);

public:

	/// Gets the D3D
	IDirect3D9 *		getD3D();

	/// Gets the D3D Device
	IDirect3DDevice9 *	getD3DDevice();

private:
	/// Clears the current front buffer
	void ClearBuffer();

	/// Swaps the back buffer to the front buffer
	void SwapBuffers();

	/// Queries the Render Device and fills m_RenderInfo with the statistics
	void QueryDevice();

	/// Converts the enabled buffer type into a FVF flag and set it, or if
	/// the vertex declaration is incompatable with FVF, explicitly set the
	/// vertex declaration
	bool ConvertBufferToFVF( BufferTypes eEnabledBuffers );

	/// Converts the enabled buffer type into a FVF flag and returns it
	bool ConvertBufferToFVF( BufferTypes eEnabledBuffers, long & dwFVF );

private:
	/// DX9 utility class for device creation, enumeration, and shutdown
	shared_ptr<DX9Enumeration>		m_spEnumeration;

	/// Render State Manager
	shared_ptr<DX9StateManager>		m_apStateManager;

	/// Texture Manager
	shared_ptr<DX9TextureManager>	m_apTextureManager;

	/// DirectX9 Devices
	ComPtr<IDirect3D9>					m_pD3D;
	ComPtr<IDirect3DDevice9> 			m_pD3DDevice;
	ComPtr<IDirect3DVertexDeclaration9>	m_pD3DVertexDeclaration;

	// Is DirectX initialized
	bool							m_bDXinit;

	/// The background color (default is grey for debug builds)
	ColorA							m_backgroundColor;
};

} // Katana

#endif // _DX9RENDER