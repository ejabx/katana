/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8render.h
	Author:		Eric Bryant

	Concrete Render class which supports DirectX8
*/

#ifndef _DX8RENDER_H
#define _DX8RENDER_H

//
// Forward Declarations
//
struct IDirect3D8;
struct IDirect3DDevice8;

namespace Katana
{

//
// Forward Declarations
//
class DX8Utility;
class DX8StateManager;
class DX8TextureManager;
class DX8ShaderManager;
class Shader;
class VertexShader;
class PixelShader;

///
/// DX8Render
/// Concrete Render class which supports DirectX8
class DX8Render : public Render
{
public:
	/// Constructor
	DX8Render();

	/// Destructor
	virtual ~DX8Render();

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

	/// Retrieves a shader
	virtual shared_ptr<Shader> getShader( const char * szShaderName );

	/// Sets up the current vertex and pixel shader. Pass NULL in order to reenable the fixed function pipeline.
	virtual bool SetShader( VertexShader * vertexShader, PixelShader * pixelShader = NULL );

	/// Sets up the shader constants for the vertex and pixel shaders.
	virtual bool SetShaderConstants( std::vector<ShaderConstant> * vertexConstants, std::vector<ShaderConstant> * pixelConstants = NULL );

	/// Grabs an image from the current screen buffer
	virtual bool GrabScreenBuffer(char * pBuffer, unsigned int uiLeft, unsigned uiTop, unsigned uiRight, unsigned int uiBottom);

	/// Sets the color of the background
	virtual void SetBackgroundColor(ColorA & color);

private:
	/// Clears the current front buffer
	void ClearBuffer();

	/// Swaps the back buffer to the front buffer
	void SwapBuffers();

	/// Sets the current pixel/vertex/combine shader
	bool SetCurrentShader(Shader * pShader);

	/// Queries the Render Device and fills m_RenderInfo with the statistics
	void QueryDevice();

	/// Helper Functions
	void ConvertCreationToUsageFlags(BufferCreationFlags eFlags, long & dwUsage);
	void ConvertBufferToFVF(BufferTypes eEnabledBuffers, long & dwFVF);

private:
	/// DX8 Utility class for device creation, enumeration, and shutdown
	shared_ptr<DX8Utility>			m_apUtility;

	/// Render State Manager
	shared_ptr<DX8StateManager>		m_apStateManager;

	/// Texture Manager
	shared_ptr<DX8TextureManager>	m_apTextureManager;

	/// Shader Manager
	shared_ptr<DX8ShaderManager>	m_apShaderManager;

	/// DirectX8 Devices
	ComPtr<IDirect3D8>				m_pD3D;
	ComPtr<IDirect3DDevice8> 		m_pD3DDevice;

	// Is DirectX initialized
	bool							m_DXinit;

	/// Should we use the FVF Fixed Function Shader?
	bool							m_useFVF;

	/// The background color (default is grey for debug builds)
	ColorA							m_backgroundColor;
};

} // Katana

#endif // _DX8RENDER