/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		render.cpp
	Author:		Eric Bryant

	Defines the interface for any render device driver
*/

#include "katana_config.h"
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "base/comptr.h"
#include "rendertypes.h"
#include "render.h"
#include "dx8render.h"
#include "dx9render.h"

// Static Types
std::string					Render::m_strLastErrorString;
Render::RenderErrorValues	Render::m_eLastError;

//
// Constructor
//
Render::Render()
{
};

//
// Destructor
//
Render::~Render()
{
}

/// Creates a render with the specified parameters, clients must
/// use this method to create a valid renderer.
Render * Render::CreateRenderer( RenderInfo & Info )
{
#ifdef RENDER_DIRECTX9_DEVICE
	return new DX9Render();
#else
#ifdef RENDER_DIRECTX8_DEVICE
	return new DX8Render();
#else
#error Undefined Renderer: choose RENDER_DIRECTX8_DEVICE and/or RENDER_DIRECTX9_DEVICE in katana_config.h
#endif
#endif
}

//
// GetLastError
//
Render::RenderErrorValues Render::GetLastError()
{
	return m_eLastError;
}

Render::RenderErrorValues Render::GetLastError(std::string & strErrorDescription)
{
	strErrorDescription = m_strLastErrorString;
	return m_eLastError;
}

//
// SetError
//
void Render::SetError(RenderErrorValues eError, const char * szErrorString)
{
	m_eLastError = eError;
	m_strLastErrorString = szErrorString;
}

//
// GetRenderInfo
//
const RenderInfo & Render::GetRenderInfo()
{
	return m_RenderInfo;
}