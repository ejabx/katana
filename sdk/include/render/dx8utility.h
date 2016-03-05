/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		dx8utility.h
	Author:		Eric Bryant

	Utility classes for DirectX8. Mimics the functionality of
	DX8App for the DirectX8 SDK.
*/

#ifndef _DX8UTILITY_H
#define _DX8UTILITY_H



namespace Katana
{


//
// Forward Declarations
//
struct RenderInfo;

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

struct D3DModeInfo
{
    DWORD      Width;      // Screen width in this mode
    DWORD      Height;     // Screen height in this mode
    D3DFORMAT  Format;     // Pixel format in this mode
    DWORD      dwBehavior; // Hardware / Software / Mixed vertex processing
    D3DFORMAT  DepthStencilFormat; // Which depth/stencil format to use with this mode
};

struct D3DDeviceInfo
{
    // Device data
    D3DDEVTYPE   DeviceType;      // Reference, HAL, etc.
    D3DCAPS8     d3dCaps;         // Capabilities of this device
    const TCHAR* strDesc;         // Name of this device
    BOOL         bCanDoWindowed;  // Whether this device can work in windowed mode

    // Modes for this device
    DWORD        dwNumModes;
    D3DModeInfo  modes[150];

    // Current state
    DWORD        dwCurrentMode;
    BOOL         bWindowed;
    D3DMULTISAMPLE_TYPE MultiSampleType;
};

struct D3DAdapterInfo
{
    // Adapter data
    D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
    D3DDISPLAYMODE d3ddmDesktop;      // Desktop display mode for this adapter

    // Devices for this adapter
    DWORD          dwNumDevices;
    D3DDeviceInfo  devices[5];

    // Current state
    DWORD          dwCurrentDevice;
};

///
/// DX8Utility
/// Utility classes for DirectX8. Mimics the functionality of
/// DX8App for the DirectX8 SDK.
class DX8Utility
{
public:
	/// Constructor
	DX8Utility();

	/// Destructor
	virtual ~DX8Utility();

	/// Creates a new render by initializing DirectX8 and the DX8Device
    HRESULT Create(RenderInfo * pInfo);

	/// Cleans up the render
	BOOL	Cleanup();

	/// Retrieve the Direct3D class
	LPDIRECT3D8			GetDirect3D();

	/// Retrieve the Direct3D Device
	LPDIRECT3DDEVICE8	GetDirect3DDevice();

	/// Retrieve the selected device caps
	const D3DCAPS8 &	GetDeviceCaps();

	/// Retrieve the selected adapter
	const D3DADAPTER_IDENTIFIER8 & GetAdapter();


private:
	// THE FOLLOWED WAS TAKEN FROM DXAPP

	/// Helper functions used by the class for DX8
    HRESULT		Initialize3DEnvironment();
    HRESULT		Resize3DEnvironment();
    HRESULT		ToggleFullscreen();
    HRESULT		ForceWindowed();
    HRESULT		UserSelectNewDevice();
    VOID		Cleanup3DEnvironment();
    HRESULT		Render3DEnvironment();
	HRESULT		BuildDeviceList();
    BOOL		FindDepthStencilFormat( UINT iAdapter, D3DDEVTYPE DeviceType,
										D3DFORMAT TargetFormat, D3DFORMAT* pDepthStencilFormat );
    HRESULT		ConfirmDevice(D3DCAPS8*,DWORD,D3DFORMAT);
    HRESULT		AdjustWindowForChange();

private:
	// THE FOLLOWED WAS TAKEN FROM DXAPP

    // Internal variables for the state of the app
    D3DAdapterInfo    m_Adapters[10];
    DWORD             m_dwNumAdapters;
    DWORD             m_dwAdapter;
    BOOL              m_bWindowed;
    BOOL              m_bActive;
    BOOL              m_bReady;

    // Internal variables used for timing
    BOOL              m_bFrameMoving;
    BOOL              m_bSingleStep;

    // Main objects used for creating and rendering the 3D scene
    D3DPRESENT_PARAMETERS m_d3dpp;         /// Parameters for CreateDevice/Reset
    HWND              m_hWnd;              /// The main app window
    HWND              m_hWndFocus;         /// The D3D focus window (usually same as m_hWnd)
    LPDIRECT3D8       m_pD3D;              /// The main D3D object
    LPDIRECT3DDEVICE8 m_pd3dDevice;        /// The D3D rendering device
    D3DCAPS8          m_d3dCaps;           /// Caps for the device
    D3DSURFACE_DESC   m_d3dsdBackBuffer;   /// Surface desc of the backbuffer
    DWORD             m_dwCreateFlags;     /// Indicate sw or hw vertex processing
    DWORD             m_dwWindowStyle;     /// Saved window style for mode switches
    RECT              m_rcWindowBounds;    /// Saved window bounds for mode switches
    RECT              m_rcWindowClient;    /// Saved client area size for mode switches

    // Variables for timing
    FLOAT             m_fTime;             // Current time in seconds
    FLOAT             m_fElapsedTime;      // Time elapsed since last frame
    FLOAT             m_fFPS;              // Instanteous frame rate
    TCHAR             m_strDeviceStats[90];// String to hold D3D device stats
    TCHAR             m_strFrameStats[40]; // String to hold frame stats

    /// Overridable variables for the app
    TCHAR*            m_strWindowTitle;    /// Title for the app's window
    BOOL              m_bUseDepthBuffer;   /// Whether to autocreate depthbuffer
    DWORD             m_dwMinDepthBits;    /// Minimum number of bits needed in depth buffer
    DWORD             m_dwMinStencilBits;  /// Minimum number of bits needed in stencil buffer
    DWORD             m_dwCreationWidth;   /// Width used to create window
    DWORD             m_dwCreationHeight;  /// Height used to create window
    BOOL              m_bShowCursorWhenFullscreen; // Whether to show cursor when fullscreen
};

} // Katana

#endif // _DX8UTILITY_H