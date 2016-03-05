/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8shadermanager.h
	Author:		Eric Bryant

	Concrete shader manager for DX8/CG Shaders
*/

#ifndef _DX8SHADERMANAGER
#define _DX8SHADERMANAGER

namespace Katana
{

//
// Forward Declarations
//
class Shader;

///
/// DX8ShaderManager
///
class DX8ShaderManager
{
public:
	/// Setup the DX8 Direct Device
	bool Initialize(IDirect3DDevice8 * pD3DDevice);

	/// Terminates the shader manager
	bool Terminate();

	/// Sets the current shader as active, and compiled it if necessary
	bool enableShader( Shader & shader );

	/// Disables the current shader
	bool disableShader( Shader & shader );

	/// Disables all shaders
	bool disableShaders();

	/// Sets the shader constants
	bool setConstants( std::vector<ShaderConstant> * vertexConstants, std::vector<ShaderConstant> * pixelConstants );

private:
	/// Compiles the shader
	bool CompileShader( Shader & shader );

private:
	ComPtr<IDirect3DDevice8>	m_pD3DDevice;				/// D3D Device
};

} // Katana

#endif // _DX8SHADERMANAGER
