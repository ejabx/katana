/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		dx9enumeration.h
	Author:		Eric Bryant

	Enumerates D3D adapters, devices, modes, etc. Use source from
	DX9Enumeration
*/

#ifndef _DX9ENUMERATION_H
#define _DX9ENUMERATION_H

namespace Katana
{

// Definitions
#define D3DAPPERR_NODIRECT3D          0x82000001
#define D3DAPPERR_NOWINDOW            0x82000002
#define D3DAPPERR_NOCOMPATIBLEDEVICES 0x82000003
#define D3DAPPERR_NOWINDOWABLEDEVICES 0x82000004
#define D3DAPPERR_NOHARDWAREDEVICE    0x82000005
#define D3DAPPERR_HALNOTCOMPATIBLE    0x82000006
#define D3DAPPERR_NOWINDOWEDHAL       0x82000007
#define D3DAPPERR_NODESKTOPHAL        0x82000008
#define D3DAPPERR_NOHALTHISMODE       0x82000009
#define D3DAPPERR_NONZEROREFCOUNT     0x8200000a
#define D3DAPPERR_MEDIANOTFOUND       0x8200000b
#define D3DAPPERR_RESIZEFAILED        0x8200000c

///
/// VertexProcessingType
/// Enumeration of all possible D3D vertex processing types.
///
enum VertexProcessingType
{
	SOFTWARE_VP,
	MIXED_VP,
	HARDWARE_VP,
	PURE_HARDWARE_VP
};

///
/// ArrayListType
/// Indicates how data should be stored in a DX9EnumArrayList
///
enum ArrayListType { AL_VALUE, AL_REFERENCE };

///
/// DX9Array
///
class DX9EnumArrayList
{
public:

protected:
	ArrayListType m_ArrayListType;
	void* m_pData;
	UINT m_BytesPerEntry;
	UINT m_NumEntries;
	UINT m_NumEntriesAllocated;

public:
	DX9EnumArrayList( ArrayListType Type, UINT BytesPerEntry = 0 );
	~DX9EnumArrayList( void );
	HRESULT Add( void* pEntry );
	void Remove( UINT Entry );
	void* GetPtr( UINT Entry );
	UINT Count( void ) { return m_NumEntries; }
	bool Contains( void* pEntryData );
	void Clear( void ) { m_NumEntries = 0; }
};

///
/// D3DDeviceCombo
/// A combination of adapter format, back buffer format, and windowed/fullscreen 
/// that is compatible with a particular D3D device (and the app).
///
struct D3DDeviceCombo
{
	int AdapterOrdinal;
	D3DDEVTYPE DevType;
	D3DFORMAT AdapterFormat;
	D3DFORMAT BackBufferFormat;
	bool IsWindowed;
	DX9EnumArrayList* pDepthStencilFormatList; // List of D3DFORMATs
	DX9EnumArrayList* pMultiSampleTypeList; // List of D3DMULTISAMPLE_TYPEs
	DX9EnumArrayList* pMultiSampleQualityList; // List of DWORDs (number of quality 
	// levels for each multisample type)
	DX9EnumArrayList* pDSMSConflictList; // List of D3DDSMSConflicts
	DX9EnumArrayList* pVertexProcessingTypeList; // List of VertexProcessingTypes
	DX9EnumArrayList* pPresentIntervalList; // List of D3DPRESENT_INTERVALs

	~D3DDeviceCombo( void );
};

///
/// D3DAdapterInfo
/// Info about a display adapter.
///
struct D3DAdapterInfo
{
	int AdapterOrdinal;
	D3DADAPTER_IDENTIFIER9 AdapterIdentifier;
	DX9EnumArrayList * pDisplayModeList; // List of D3DDISPLAYMODEs
	DX9EnumArrayList * pDeviceInfoList; // List of D3DDeviceInfo pointers
	~D3DAdapterInfo( void );
};

///
/// D3DDeviceInfo
/// Info about a D3D device, including a list of D3DDeviceCombos (see below) 
/// that work with the device.
///
struct D3DDeviceInfo
{
	int AdapterOrdinal;
	D3DDEVTYPE DevType;
	D3DCAPS9 Caps;
	DX9EnumArrayList* pDeviceComboList; // List of D3DDeviceCombo pointers
	~D3DDeviceInfo( void );
};


///
/// D3DDSMSConflict
/// A depth/stencil buffer format that is incompatible with a multisample type.
///
struct D3DDSMSConflict
{
	D3DFORMAT DSFormat;
	D3DMULTISAMPLE_TYPE MSType;
};

///
/// CONFIRMDEVICECALLBACK
///
typedef bool(* CONFIRMDEVICECALLBACK)( D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType, 
	D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat );

///
/// D3DSettings
/// Current D3D settings: adapter, device, mode, formats, etc.
///
struct D3DSettings 
{
	bool IsWindowed;
	D3DAdapterInfo* pWindowed_AdapterInfo;
	D3DDeviceInfo* pWindowed_DeviceInfo;
	D3DDeviceCombo* pWindowed_DeviceCombo;

	D3DDISPLAYMODE Windowed_DisplayMode; // not changable by the user
	D3DFORMAT Windowed_DepthStencilBufferFormat;
	D3DMULTISAMPLE_TYPE Windowed_MultisampleType;
	DWORD Windowed_MultisampleQuality;
	VertexProcessingType Windowed_VertexProcessingType;
	UINT Windowed_PresentInterval;
	int Windowed_Width;
	int Windowed_Height;

	D3DAdapterInfo* pFullscreen_AdapterInfo;
	D3DDeviceInfo* pFullscreen_DeviceInfo;
	D3DDeviceCombo* pFullscreen_DeviceCombo;

	D3DDISPLAYMODE Fullscreen_DisplayMode; // changable by the user
	D3DFORMAT Fullscreen_DepthStencilBufferFormat;
	D3DMULTISAMPLE_TYPE Fullscreen_MultisampleType;
	DWORD Fullscreen_MultisampleQuality;
	VertexProcessingType Fullscreen_VertexProcessingType;
	UINT Fullscreen_PresentInterval;

	D3DAdapterInfo* PAdapterInfo() { return IsWindowed ? pWindowed_AdapterInfo : pFullscreen_AdapterInfo; }
	D3DDeviceInfo* PDeviceInfo() { return IsWindowed ? pWindowed_DeviceInfo : pFullscreen_DeviceInfo; }
	D3DDeviceCombo* PDeviceCombo() { return IsWindowed ? pWindowed_DeviceCombo : pFullscreen_DeviceCombo; }

	int AdapterOrdinal() { return PDeviceCombo()->AdapterOrdinal; }
	D3DDEVTYPE DevType() { return PDeviceCombo()->DevType; }
	D3DFORMAT BackBufferFormat() { return PDeviceCombo()->BackBufferFormat; }

	D3DDISPLAYMODE DisplayMode() { return IsWindowed ? Windowed_DisplayMode : Fullscreen_DisplayMode; }
	void SetDisplayMode(D3DDISPLAYMODE value) { if (IsWindowed) Windowed_DisplayMode = value; else Fullscreen_DisplayMode = value; }

	D3DFORMAT DepthStencilBufferFormat() { return IsWindowed ? Windowed_DepthStencilBufferFormat : Fullscreen_DepthStencilBufferFormat; }
	void SetDepthStencilBufferFormat(D3DFORMAT value) { if (IsWindowed) Windowed_DepthStencilBufferFormat = value; else Fullscreen_DepthStencilBufferFormat = value; }

	D3DMULTISAMPLE_TYPE MultisampleType() { return IsWindowed ? Windowed_MultisampleType : Fullscreen_MultisampleType; }
	void SetMultisampleType(D3DMULTISAMPLE_TYPE value) { if (IsWindowed) Windowed_MultisampleType = value; else Fullscreen_MultisampleType = value; }

	DWORD MultisampleQuality() { return IsWindowed ? Windowed_MultisampleQuality : Fullscreen_MultisampleQuality; }
	void SetMultisampleQuality(DWORD value) { if (IsWindowed) Windowed_MultisampleQuality = value; else Fullscreen_MultisampleQuality = value; }

	VertexProcessingType GetVertexProcessingType() { return IsWindowed ? Windowed_VertexProcessingType : Fullscreen_VertexProcessingType; }
	void SetVertexProcessingType(VertexProcessingType value) { if (IsWindowed) Windowed_VertexProcessingType = value; else Fullscreen_VertexProcessingType = value; }

	UINT PresentInterval() { return IsWindowed ? Windowed_PresentInterval : Fullscreen_PresentInterval; }
	void SetPresentInterval(UINT value) { if (IsWindowed) Windowed_PresentInterval = value; else Fullscreen_PresentInterval = value; }
};

///
/// DX9Enumeration
/// Enumerates available D3D adapters, devices, modes, etc.
///
class DX9Enumeration
{
private:
	 
	IDirect3D9*				m_pD3D;
	IDirect3DDevice9 *		m_pD3DDevice;
	D3DSettings				m_d3dSettings;
	D3DPRESENT_PARAMETERS	m_d3dpp;

private:
	HRESULT EnumerateDevices( D3DAdapterInfo* pAdapterInfo, DX9EnumArrayList* pAdapterFormatList );
	HRESULT EnumerateDeviceCombos( D3DDeviceInfo* pDeviceInfo, DX9EnumArrayList* pAdapterFormatList );
	void BuildDepthStencilFormatList( D3DDeviceCombo* pDeviceCombo );
	void BuildMultiSampleTypeList( D3DDeviceCombo* pDeviceCombo );
	void BuildDSMSConflictList( D3DDeviceCombo* pDeviceCombo );
	void BuildVertexProcessingTypeList( D3DDeviceInfo* pDeviceInfo, D3DDeviceCombo* pDeviceCombo );
	void BuildPresentIntervalList( D3DDeviceInfo* pDeviceInfo, D3DDeviceCombo* pDeviceCombo );
	void BuildPresentParamsFromSettings( RenderInfo & renderInfo );
	HRESULT Enumerate();
	bool FindBestWindowedMode( bool bRequireHAL, bool bRequireREF );
	bool FindBestFullscreenMode( bool bRequireHAL, bool bRequireREF );

public:
	DX9EnumArrayList* m_pAdapterInfoList;
	// The following variables can be used to limit what modes, formats, 
	// etc. are enumerated.  Set them to the values you want before calling
	// Enumerate().
	CONFIRMDEVICECALLBACK ConfirmDeviceCallback;
	UINT AppMinFullscreenWidth;
	UINT AppMinFullscreenHeight;
	UINT AppMinColorChannelBits; // min color bits per channel in adapter format
	UINT AppMinAlphaChannelBits; // min alpha bits per pixel in back buffer format
	UINT AppMinDepthBits;
	UINT AppMinStencilBits;
	bool AppUsesDepthBuffer;
	bool AppUsesMixedVP; // whether app can take advantage of mixed vp mode
	bool AppRequiresWindowed;
	bool AppRequiresFullscreen;
	bool AppRequiresReferenceDriver;
	DX9EnumArrayList* m_pAllowedAdapterFormatList; // list of D3DFORMATs

	DX9Enumeration();
	~DX9Enumeration();
	HRESULT Create( RenderInfo & renderInfo );
	bool	Cleanup();
	IDirect3D9 * GetDirect3D()				{ if( m_pD3D ) m_pD3D->AddRef(); return m_pD3D; }
	IDirect3DDevice9 * GetDirect3DDevice()	{ if ( m_pD3DDevice ) m_pD3DDevice->AddRef(); return m_pD3DDevice; }
	D3DSettings GetDirect3DSettings()		{ return m_d3dSettings; }
};

}; // Katana

#endif // _DX9ENUMERATION_H