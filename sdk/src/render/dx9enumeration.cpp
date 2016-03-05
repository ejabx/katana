/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		dx9enumeration.h
	Author:		Eric Bryant

	Enumerates D3D adapters, devices, modes, etc. Use source from
	DX9Enumeration
*/

#include <d3d9.h>
#include <d3dx9.h>
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "rendertypes.h"
#include "dx9enumeration.h"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }


//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------

static UINT ColorChannelBits( D3DFORMAT fmt );
static UINT AlphaChannelBits( D3DFORMAT fmt );
static UINT DepthBits( D3DFORMAT fmt );
static UINT StencilBits( D3DFORMAT fmt );
static int __cdecl SortModesCallback( const void* arg1, const void* arg2 );
static bool ConfirmDevice_SoftwareVP( D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat );
static bool ConfirmDevice_HardwareVP( D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat );

//-----------------------------------------------------------------------------
// DX9Enumeration
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// D3DAdapterInfo
//-----------------------------------------------------------------------------

D3DAdapterInfo::~D3DAdapterInfo( void )
{
	if( pDisplayModeList != NULL )
		delete pDisplayModeList;
	if( pDeviceInfoList != NULL )
	{
		for( UINT idi = 0; idi < pDeviceInfoList->Count(); idi++ )
			delete (D3DDeviceInfo*)pDeviceInfoList->GetPtr( idi );
		delete pDeviceInfoList;
	}
}


//-----------------------------------------------------------------------------
// D3DDeviceInfo
//-----------------------------------------------------------------------------

D3DDeviceInfo::~D3DDeviceInfo( void )
{
	if( pDeviceComboList != NULL )
	{
		for( UINT idc = 0; idc < pDeviceComboList->Count(); idc++ )
			delete (D3DDeviceCombo*)pDeviceComboList->GetPtr( idc );
		delete pDeviceComboList;
	}
}




//-----------------------------------------------------------------------------
// D3DDeviceCombo
//-----------------------------------------------------------------------------

D3DDeviceCombo::~D3DDeviceCombo( void )
{
	if( pDepthStencilFormatList != NULL )
		delete pDepthStencilFormatList;
	if( pMultiSampleTypeList != NULL )
		delete pMultiSampleTypeList;
	if( pMultiSampleQualityList != NULL )
		delete pMultiSampleQualityList;
	if( pDSMSConflictList != NULL )
		delete pDSMSConflictList;
	if( pVertexProcessingTypeList != NULL )
		delete pVertexProcessingTypeList;
	if( pPresentIntervalList != NULL )
		delete pPresentIntervalList;
}

//-----------------------------------------------------------------------------
// DX9EnumArrayList
//-----------------------------------------------------------------------------

//
// Constructor
//
DX9EnumArrayList::DX9EnumArrayList( ArrayListType Type, UINT BytesPerEntry )
{
	if( Type == AL_REFERENCE )
		BytesPerEntry = sizeof(void*);
	m_ArrayListType = Type;
	m_pData = NULL;
	m_BytesPerEntry = BytesPerEntry;
	m_NumEntries = 0;
	m_NumEntriesAllocated = 0;
}

//
// Destructor
//
DX9EnumArrayList::~DX9EnumArrayList( void )
{
	if( m_pData != NULL )
		delete[] m_pData;
}


//
// Add
// Adds pEntry to the list.
//
HRESULT DX9EnumArrayList::Add( void* pEntry )
{
	if( m_BytesPerEntry == 0 )
		return E_FAIL;
	if( m_pData == NULL || m_NumEntries + 1 > m_NumEntriesAllocated )
	{
		void* pDataNew;
		UINT NumEntriesAllocatedNew;
		if( m_NumEntriesAllocated == 0 )
			NumEntriesAllocatedNew = 16;
		else
			NumEntriesAllocatedNew = m_NumEntriesAllocated * 2;
		pDataNew = new BYTE[NumEntriesAllocatedNew * m_BytesPerEntry];
		if( pDataNew == NULL )
			return E_OUTOFMEMORY;
		if( m_pData != NULL )
		{
			CopyMemory( pDataNew, m_pData, m_NumEntries * m_BytesPerEntry );
			delete[] m_pData;
		}
		m_pData = pDataNew;
		m_NumEntriesAllocated = NumEntriesAllocatedNew;
	}

	if( m_ArrayListType == AL_VALUE )
		CopyMemory( (BYTE*)m_pData + (m_NumEntries * m_BytesPerEntry), pEntry, m_BytesPerEntry );
	else
		*(((void**)m_pData) + m_NumEntries) = pEntry;
	m_NumEntries++;

	return S_OK;
}


//
// Remove
// Remove the item at Entry in the list, and collapse the array. 
//
void DX9EnumArrayList::Remove( UINT Entry )
{
	// Decrement count
	m_NumEntries--;

	// Find the entry address
	BYTE* pData = (BYTE*)m_pData + (Entry * m_BytesPerEntry);

	// Collapse the array
	MoveMemory( pData, pData + m_BytesPerEntry, ( m_NumEntries - Entry ) * m_BytesPerEntry );
}


//
// GetPtr
// Returns a pointer to the Entry'th entry in the list.
//
void* DX9EnumArrayList::GetPtr( UINT Entry )
{
	if( m_ArrayListType == AL_VALUE )
		return (BYTE*)m_pData + (Entry * m_BytesPerEntry);
	else
		return *(((void**)m_pData) + Entry);
}


//
// Contains
// Returns whether the list contains an entry identical to the 
// specified entry data.
//
bool DX9EnumArrayList::Contains( void* pEntryData )
{
	for( UINT iEntry = 0; iEntry < m_NumEntries; iEntry++ )
	{
		if( m_ArrayListType == AL_VALUE )
		{
			if( memcmp( GetPtr(iEntry), pEntryData, m_BytesPerEntry ) == 0 )
				return true;
		}
		else
		{
			if( GetPtr(iEntry) == pEntryData )
				return true;
		}
	}
	return false;
}


//-----------------------------------------------------------------------------
// DX9Enumeration
//-----------------------------------------------------------------------------

//
// Constructor
//
DX9Enumeration::DX9Enumeration()
{
	m_pAdapterInfoList = NULL;
	m_pAllowedAdapterFormatList = NULL;
	AppMinFullscreenWidth = 640;
	AppMinFullscreenHeight = 480;
	AppMinColorChannelBits = 4;
	AppMinAlphaChannelBits = 0;
	AppMinDepthBits = 16;
	AppMinStencilBits = 4;
	AppUsesDepthBuffer = true;
	AppUsesMixedVP = false;
	AppRequiresWindowed = false;
	AppRequiresFullscreen = false;
	m_pD3D = NULL;
	m_pD3DDevice = NULL;
}

//
// Destructor
//
DX9Enumeration::~DX9Enumeration()
{
	if( m_pAdapterInfoList != NULL )
	{
		for( UINT iai = 0; iai < m_pAdapterInfoList->Count(); iai++ )
			delete (D3DAdapterInfo*)m_pAdapterInfoList->GetPtr( iai );
		delete m_pAdapterInfoList;
	}
	SAFE_DELETE( m_pAllowedAdapterFormatList );
}

//
// Create
//
HRESULT DX9Enumeration::Create( RenderInfo & renderInfo )
{
	// Store the Parameters
	AppMinFullscreenWidth = renderInfo.uiTargetWidth;
	AppMinFullscreenHeight = renderInfo.uiTargetHeight;
	AppMinColorChannelBits = renderInfo.uiTargetColorDepth == 16 ? 4 : 8;
	AppMinAlphaChannelBits = renderInfo.uiTargetColorDepth == 16 ? 4 : 8;
	AppRequiresWindowed = !renderInfo.bFullscreen;
	AppRequiresFullscreen = renderInfo.bFullscreen;
	AppRequiresReferenceDriver = renderInfo.bUseReferenceDriver;

#ifdef _DEBUG
	AppUsesMixedVP = true; // This is to allow debugging of HLSL
	ConfirmDeviceCallback = ConfirmDevice_SoftwareVP;
#else
	AppUsesMixedVP = false;
	ConfirmDeviceCallback = ConfirmDevice_HardwareVP;
#endif

	// Create the Direct3D Object
	m_pD3D = Direct3DCreate9( D3D_SDK_VERSION );
	if ( m_pD3D == NULL )
		return D3DAPPERR_NODIRECT3D;

	// Perform the enumeration
	if ( FAILED( Enumerate() ) )
		return E_FAIL;

	// Choose the best adapter based on if we want windowed or fullscreen mode
	if ( AppRequiresWindowed )
	{
		if ( !FindBestWindowedMode( !AppUsesMixedVP, AppRequiresReferenceDriver ) )
			return E_FAIL;
	}
	else
	{
		if ( !FindBestFullscreenMode( !AppUsesMixedVP, AppRequiresReferenceDriver  ) )
			return E_FAIL;
	}

	// From the D3DSettings, determine the parameters for the device
	D3DAdapterInfo* pAdapterInfo = m_d3dSettings.PAdapterInfo();
	D3DDeviceInfo* pDeviceInfo = m_d3dSettings.PDeviceInfo();
	bool bWindowed = m_d3dSettings.IsWindowed;
	HWND hWndFocus = (HWND)renderInfo.hMainWindow;

	DWORD behaviorFlags;
	if (m_d3dSettings.GetVertexProcessingType() == SOFTWARE_VP)
		behaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	else if (m_d3dSettings.GetVertexProcessingType() == MIXED_VP)
		behaviorFlags = D3DCREATE_MIXED_VERTEXPROCESSING;
	else if (m_d3dSettings.GetVertexProcessingType() == HARDWARE_VP)
		behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else if (m_d3dSettings.GetVertexProcessingType() == PURE_HARDWARE_VP)
		behaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE;
	else
		behaviorFlags = 0; // TODO: throw exception

	// Set up the presentation parameters
	BuildPresentParamsFromSettings( renderInfo );

	// Create the D3D9 Device and return the result
	return m_pD3D->CreateDevice( m_d3dSettings.AdapterOrdinal(), 
								 pDeviceInfo->DevType,
								 hWndFocus, 
								 behaviorFlags, 
								 &m_d3dpp,
								 &m_pD3DDevice 
	);
}

//
// Cleanup
//
bool DX9Enumeration::Cleanup()
{
	if ( m_pD3D ) m_pD3D->Release();
	if ( m_pD3DDevice ) m_pD3DDevice->Release();

	return true;
}

//
// Enumerate
// Enumerates available D3D adapters, devices, modes, etc.
//
HRESULT DX9Enumeration::Enumerate()
{
	HRESULT hr;
	DX9EnumArrayList adapterFormatList( AL_VALUE, sizeof(D3DFORMAT) );

	if( m_pD3D == NULL )
		return E_FAIL;

	m_pAdapterInfoList = new DX9EnumArrayList( AL_REFERENCE );
	if( m_pAdapterInfoList == NULL )
		return E_OUTOFMEMORY;

	m_pAllowedAdapterFormatList = new DX9EnumArrayList( AL_VALUE, sizeof(D3DFORMAT) );
	if( m_pAllowedAdapterFormatList == NULL )
		return E_OUTOFMEMORY;
	D3DFORMAT fmt;
	if( FAILED( hr = m_pAllowedAdapterFormatList->Add( &( fmt = D3DFMT_X8R8G8B8 ) ) ) )
		return hr;
	if( FAILED( hr = m_pAllowedAdapterFormatList->Add( &( fmt = D3DFMT_X1R5G5B5 ) ) ) )
		return hr;
	if( FAILED( hr = m_pAllowedAdapterFormatList->Add( &( fmt = D3DFMT_R5G6B5 ) ) ) )
		return hr;
	if( FAILED( hr = m_pAllowedAdapterFormatList->Add( &( fmt = D3DFMT_A2R10G10B10 ) ) ) )
		return hr;

	D3DAdapterInfo* pAdapterInfo = NULL;
	UINT numAdapters = m_pD3D->GetAdapterCount();

	for (UINT adapterOrdinal = 0; adapterOrdinal < numAdapters; adapterOrdinal++)
	{
		pAdapterInfo = new D3DAdapterInfo;
		if( pAdapterInfo == NULL )
			return E_OUTOFMEMORY;
		pAdapterInfo->pDisplayModeList = new DX9EnumArrayList( AL_VALUE, sizeof(D3DDISPLAYMODE)); 
		pAdapterInfo->pDeviceInfoList = new DX9EnumArrayList( AL_REFERENCE );
		if( pAdapterInfo->pDisplayModeList == NULL ||
			pAdapterInfo->pDeviceInfoList == NULL )
		{
			delete pAdapterInfo;
			return E_OUTOFMEMORY;
		}
		pAdapterInfo->AdapterOrdinal = adapterOrdinal;
		m_pD3D->GetAdapterIdentifier(adapterOrdinal, 0, &pAdapterInfo->AdapterIdentifier);

		// Get list of all display modes on this adapter.  
		// Also build a temporary list of all display adapter formats.
		adapterFormatList.Clear();
		for( UINT iaaf = 0; iaaf < m_pAllowedAdapterFormatList->Count(); iaaf++ )
		{
			D3DFORMAT allowedAdapterFormat = *(D3DFORMAT*)m_pAllowedAdapterFormatList->GetPtr( iaaf );
			UINT numAdapterModes = m_pD3D->GetAdapterModeCount( adapterOrdinal, allowedAdapterFormat );
			for (UINT mode = 0; mode < numAdapterModes; mode++)
			{
				D3DDISPLAYMODE displayMode;
				m_pD3D->EnumAdapterModes( adapterOrdinal, allowedAdapterFormat, mode, &displayMode );
				if( displayMode.Width < AppMinFullscreenWidth ||
					displayMode.Height < AppMinFullscreenHeight ||
					ColorChannelBits(displayMode.Format) < AppMinColorChannelBits )
				{
					continue;
				}
				pAdapterInfo->pDisplayModeList->Add( &displayMode );
				if( !adapterFormatList.Contains( &displayMode.Format ) )
					adapterFormatList.Add( &displayMode.Format );
			}
		}

		// Sort displaymode list
		qsort( pAdapterInfo->pDisplayModeList->GetPtr( 0 ), 
			pAdapterInfo->pDisplayModeList->Count(), sizeof( D3DDISPLAYMODE ),
			SortModesCallback );

		// Get info for each device on this adapter
		if( FAILED( hr = EnumerateDevices( pAdapterInfo, &adapterFormatList ) ) )
		{
			delete pAdapterInfo;
			return hr;
		}

		// If at least one device on this adapter is available and compatible
		// with the app, add the adapterInfo to the list
		if (pAdapterInfo->pDeviceInfoList->Count() == 0)
			delete pAdapterInfo;
		else
			m_pAdapterInfoList->Add(pAdapterInfo);
	}
	return S_OK;
}

//
// EnumerateDevices
// Enumerates D3D devices for a particular adapter.
//
HRESULT DX9Enumeration::EnumerateDevices( D3DAdapterInfo* pAdapterInfo, 
										  DX9EnumArrayList* pAdapterFormatList )
{
	const D3DDEVTYPE devTypeArray[] = { D3DDEVTYPE_HAL, D3DDEVTYPE_SW, D3DDEVTYPE_REF };
	const UINT devTypeArrayCount = sizeof(devTypeArray) / sizeof(devTypeArray[0]);
	HRESULT hr;

	D3DDeviceInfo* pDeviceInfo = NULL;
	for( UINT idt = 0; idt < devTypeArrayCount; idt++ )
	{
		pDeviceInfo = new D3DDeviceInfo;
		if( pDeviceInfo == NULL )
			return E_OUTOFMEMORY;
		pDeviceInfo->pDeviceComboList = new DX9EnumArrayList( AL_REFERENCE ); 
		if( pDeviceInfo->pDeviceComboList == NULL )
		{
			delete pDeviceInfo;
			return E_OUTOFMEMORY;
		}
		pDeviceInfo->AdapterOrdinal = pAdapterInfo->AdapterOrdinal;
		pDeviceInfo->DevType = devTypeArray[idt];
		if( FAILED( m_pD3D->GetDeviceCaps( pAdapterInfo->AdapterOrdinal, 
			pDeviceInfo->DevType, &pDeviceInfo->Caps ) ) )
		{
			delete pDeviceInfo;
			continue;
		}

		// Get info for each devicecombo on this device
		if( FAILED( hr = EnumerateDeviceCombos(pDeviceInfo, pAdapterFormatList) ) )
		{
			delete pDeviceInfo;
			return hr;
		}

		// If at least one devicecombo for this device is found, 
		// add the deviceInfo to the list
		if (pDeviceInfo->pDeviceComboList->Count() == 0)
		{
			delete pDeviceInfo;
			continue;
		}
		pAdapterInfo->pDeviceInfoList->Add(pDeviceInfo);
	}
	return S_OK;
}

//
// EnumerateDeviceCombos
// Enumerates DeviceCombos for a particular device.
//
HRESULT DX9Enumeration::EnumerateDeviceCombos( D3DDeviceInfo* pDeviceInfo, 
											   DX9EnumArrayList* pAdapterFormatList )
{
	const D3DFORMAT backBufferFormatArray[] = 
	{   D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_A2R10G10B10, 
	D3DFMT_R5G6B5, D3DFMT_A1R5G5B5, D3DFMT_X1R5G5B5 };
	const UINT backBufferFormatArrayCount = sizeof(backBufferFormatArray) / sizeof(backBufferFormatArray[0]);
	bool isWindowedArray[] = { false, true };

	// See which adapter formats are supported by this device
	D3DFORMAT adapterFormat;
	for( UINT iaf = 0; iaf < pAdapterFormatList->Count(); iaf++ )
	{
		adapterFormat = *(D3DFORMAT*)pAdapterFormatList->GetPtr(iaf);
		D3DFORMAT backBufferFormat;
		for( UINT ibbf = 0; ibbf < backBufferFormatArrayCount; ibbf++ )
		{
			backBufferFormat = backBufferFormatArray[ibbf];
			if (AlphaChannelBits(backBufferFormat) < AppMinAlphaChannelBits)
				continue;
			bool isWindowed;
			for( UINT iiw = 0; iiw < 2; iiw++)
			{
				isWindowed = isWindowedArray[iiw];
				if (!isWindowed && AppRequiresWindowed)
					continue;
				if (isWindowed && AppRequiresFullscreen)
					continue;
				if (FAILED(m_pD3D->CheckDeviceType(pDeviceInfo->AdapterOrdinal, pDeviceInfo->DevType, 
					adapterFormat, backBufferFormat, isWindowed)))
				{
					continue;
				}
				// At this point, we have an adapter/device/adapterformat/backbufferformat/iswindowed
				// DeviceCombo that is supported by the system.  We still need to confirm that it's 
				// compatible with the app, and find one or more suitable depth/stencil buffer format,
				// multisample type, vertex processing type, and present interval.
				D3DDeviceCombo* pDeviceCombo = NULL;
				pDeviceCombo = new D3DDeviceCombo;
				if( pDeviceCombo == NULL )
					return E_OUTOFMEMORY;
				pDeviceCombo->pDepthStencilFormatList = new DX9EnumArrayList( AL_VALUE, sizeof( D3DFORMAT ) );
				pDeviceCombo->pMultiSampleTypeList = new DX9EnumArrayList( AL_VALUE, sizeof( D3DMULTISAMPLE_TYPE ) );
				pDeviceCombo->pMultiSampleQualityList = new DX9EnumArrayList( AL_VALUE, sizeof( DWORD ) );
				pDeviceCombo->pDSMSConflictList = new DX9EnumArrayList( AL_VALUE, sizeof( D3DDSMSConflict ) );
				pDeviceCombo->pVertexProcessingTypeList = new DX9EnumArrayList( AL_VALUE, sizeof( VertexProcessingType ) );
				pDeviceCombo->pPresentIntervalList = new DX9EnumArrayList( AL_VALUE, sizeof( UINT ) );
				if( pDeviceCombo->pDepthStencilFormatList == NULL ||
					pDeviceCombo->pMultiSampleTypeList == NULL || 
					pDeviceCombo->pMultiSampleQualityList == NULL || 
					pDeviceCombo->pDSMSConflictList == NULL || 
					pDeviceCombo->pVertexProcessingTypeList == NULL ||
					pDeviceCombo->pPresentIntervalList == NULL )
				{
					delete pDeviceCombo;
					return E_OUTOFMEMORY;
				}
				pDeviceCombo->AdapterOrdinal = pDeviceInfo->AdapterOrdinal;
				pDeviceCombo->DevType = pDeviceInfo->DevType;
				pDeviceCombo->AdapterFormat = adapterFormat;
				pDeviceCombo->BackBufferFormat = backBufferFormat;
				pDeviceCombo->IsWindowed = isWindowed;
				if (AppUsesDepthBuffer)
				{
					BuildDepthStencilFormatList(pDeviceCombo);
					if (pDeviceCombo->pDepthStencilFormatList->Count() == 0)
					{
						delete pDeviceCombo;
						continue;
					}
				}
				BuildMultiSampleTypeList(pDeviceCombo);
				if (pDeviceCombo->pMultiSampleTypeList->Count() == 0)
				{
					delete pDeviceCombo;
					continue;
				}
				BuildDSMSConflictList(pDeviceCombo);
				BuildVertexProcessingTypeList(pDeviceInfo, pDeviceCombo);
				if (pDeviceCombo->pVertexProcessingTypeList->Count() == 0)
				{
					delete pDeviceCombo;
					continue;
				}
				BuildPresentIntervalList(pDeviceInfo, pDeviceCombo);

				pDeviceInfo->pDeviceComboList->Add(pDeviceCombo);
			}
		}
	}

	return S_OK;
}


//
// BuildDepthStencilFormatList
// Adds all depth/stencil formats that are compatible with the device 
// and app to the given D3DDeviceCombo.
//
void DX9Enumeration::BuildDepthStencilFormatList( D3DDeviceCombo* pDeviceCombo )
{
	const D3DFORMAT depthStencilFormatArray[] = 
	{
		D3DFMT_D16,
			D3DFMT_D15S1,
			D3DFMT_D24X8,
			D3DFMT_D24S8,
			D3DFMT_D24X4S4,
			D3DFMT_D32,
	};
	const UINT depthStencilFormatArrayCount = sizeof(depthStencilFormatArray) / 
		sizeof(depthStencilFormatArray[0]);

	D3DFORMAT depthStencilFmt;
	for( UINT idsf = 0; idsf < depthStencilFormatArrayCount; idsf++ )
	{
		depthStencilFmt = depthStencilFormatArray[idsf];
		if (DepthBits(depthStencilFmt) < AppMinDepthBits)
			continue;
		if (StencilBits(depthStencilFmt) < AppMinStencilBits)
			continue;
		if (SUCCEEDED(m_pD3D->CheckDeviceFormat(pDeviceCombo->AdapterOrdinal, 
			pDeviceCombo->DevType, pDeviceCombo->AdapterFormat, 
			D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, depthStencilFmt)))
		{
			if (SUCCEEDED(m_pD3D->CheckDepthStencilMatch(pDeviceCombo->AdapterOrdinal, 
				pDeviceCombo->DevType, pDeviceCombo->AdapterFormat, 
				pDeviceCombo->BackBufferFormat, depthStencilFmt)))
			{
				pDeviceCombo->pDepthStencilFormatList->Add(&depthStencilFmt);
			}
		}
	}
}

//
// BuildMultiSampleTypeList
// Adds all multisample types that are compatible with the device and app to
// the given D3DDeviceCombo.
//
void DX9Enumeration::BuildMultiSampleTypeList( D3DDeviceCombo* pDeviceCombo )
{
	const D3DMULTISAMPLE_TYPE msTypeArray[] = { 
		D3DMULTISAMPLE_NONE,
			D3DMULTISAMPLE_NONMASKABLE,
			D3DMULTISAMPLE_2_SAMPLES,
			D3DMULTISAMPLE_3_SAMPLES,
			D3DMULTISAMPLE_4_SAMPLES,
			D3DMULTISAMPLE_5_SAMPLES,
			D3DMULTISAMPLE_6_SAMPLES,
			D3DMULTISAMPLE_7_SAMPLES,
			D3DMULTISAMPLE_8_SAMPLES,
			D3DMULTISAMPLE_9_SAMPLES,
			D3DMULTISAMPLE_10_SAMPLES,
			D3DMULTISAMPLE_11_SAMPLES,
			D3DMULTISAMPLE_12_SAMPLES,
			D3DMULTISAMPLE_13_SAMPLES,
			D3DMULTISAMPLE_14_SAMPLES,
			D3DMULTISAMPLE_15_SAMPLES,
			D3DMULTISAMPLE_16_SAMPLES,
	};
	const UINT msTypeArrayCount = sizeof(msTypeArray) / sizeof(msTypeArray[0]);

	D3DMULTISAMPLE_TYPE msType;
	DWORD msQuality;
	for( UINT imst = 0; imst < msTypeArrayCount; imst++ )
	{
		msType = msTypeArray[imst];
		if (SUCCEEDED(m_pD3D->CheckDeviceMultiSampleType(pDeviceCombo->AdapterOrdinal, pDeviceCombo->DevType, 
			pDeviceCombo->BackBufferFormat, pDeviceCombo->IsWindowed, msType, &msQuality)))
		{
			pDeviceCombo->pMultiSampleTypeList->Add(&msType);
			pDeviceCombo->pMultiSampleQualityList->Add( &msQuality );
		}
	}
}




//
// BuildDSMSConflictList
// Find any conflicts between the available depth/stencil formats and
// multisample types.
//
void DX9Enumeration::BuildDSMSConflictList( D3DDeviceCombo* pDeviceCombo )
{
	D3DDSMSConflict DSMSConflict;

	for( UINT ids = 0; ids < pDeviceCombo->pDepthStencilFormatList->Count(); ids++ )
	{
		D3DFORMAT dsFmt = *(D3DFORMAT*)pDeviceCombo->pDepthStencilFormatList->GetPtr(ids);
		for( UINT ims = 0; ims < pDeviceCombo->pMultiSampleTypeList->Count(); ims++ )
		{
			D3DMULTISAMPLE_TYPE msType = *(D3DMULTISAMPLE_TYPE*)pDeviceCombo->pMultiSampleTypeList->GetPtr(ims);
			if( FAILED( m_pD3D->CheckDeviceMultiSampleType( pDeviceCombo->AdapterOrdinal, pDeviceCombo->DevType,
				dsFmt, pDeviceCombo->IsWindowed, msType, NULL ) ) )
			{
				DSMSConflict.DSFormat = dsFmt;
				DSMSConflict.MSType = msType;
				pDeviceCombo->pDSMSConflictList->Add( &DSMSConflict );
			}
		}
	}
}

//
// BuildVertexProcessingTypeList
// Adds all vertex processing types that are compatible with the device 
// and app to the given D3DDeviceCombo.
//
void DX9Enumeration::BuildVertexProcessingTypeList( D3DDeviceInfo* pDeviceInfo, 
													D3DDeviceCombo* pDeviceCombo )
{
	VertexProcessingType vpt;
	if ((pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0)
	{
		if ((pDeviceInfo->Caps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0)
		{
			if (ConfirmDeviceCallback == NULL ||
				ConfirmDeviceCallback(&pDeviceInfo->Caps, PURE_HARDWARE_VP, 
				pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat))
			{
				vpt = PURE_HARDWARE_VP;
				pDeviceCombo->pVertexProcessingTypeList->Add(&vpt);
			}
		}
		if (ConfirmDeviceCallback == NULL ||
			ConfirmDeviceCallback(&pDeviceInfo->Caps, HARDWARE_VP, 
			pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat))
		{
			vpt = HARDWARE_VP;
			pDeviceCombo->pVertexProcessingTypeList->Add(&vpt);
		}
		if (AppUsesMixedVP && (ConfirmDeviceCallback == NULL ||
			ConfirmDeviceCallback(&pDeviceInfo->Caps, MIXED_VP, 
			pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat)))
		{
			vpt = MIXED_VP;
			pDeviceCombo->pVertexProcessingTypeList->Add(&vpt);
		}
	}
	if (ConfirmDeviceCallback == NULL ||
		ConfirmDeviceCallback(&pDeviceInfo->Caps, SOFTWARE_VP, 
		pDeviceCombo->AdapterFormat, pDeviceCombo->BackBufferFormat))
	{
		vpt = SOFTWARE_VP;
		pDeviceCombo->pVertexProcessingTypeList->Add(&vpt);
	}
}


//
// BuildPresentIntervalList
// Adds all present intervals that are compatible with the device and app 
// to the given D3DDeviceCombo.
//
void DX9Enumeration::BuildPresentIntervalList( D3DDeviceInfo* pDeviceInfo, 
											   D3DDeviceCombo* pDeviceCombo )
{
	const UINT piArray[] = { 
		D3DPRESENT_INTERVAL_IMMEDIATE,
			D3DPRESENT_INTERVAL_DEFAULT,
			D3DPRESENT_INTERVAL_ONE,
			D3DPRESENT_INTERVAL_TWO,
			D3DPRESENT_INTERVAL_THREE,
			D3DPRESENT_INTERVAL_FOUR,
	};
	const UINT piArrayCount = sizeof(piArray) / sizeof(piArray[0]);

	UINT pi;
	for( UINT ipi = 0; ipi < piArrayCount; ipi++ )
	{
		pi = piArray[ipi];
		if( pDeviceCombo->IsWindowed )
		{
			if( pi == D3DPRESENT_INTERVAL_TWO ||
				pi == D3DPRESENT_INTERVAL_THREE ||
				pi == D3DPRESENT_INTERVAL_FOUR )
			{
				// These intervals are not supported in windowed mode.
				continue;
			}
		}
		// Note that D3DPRESENT_INTERVAL_DEFAULT is zero, so you
		// can't do a caps check for it -- it is always available.
		if( pi == D3DPRESENT_INTERVAL_DEFAULT ||
			(pDeviceInfo->Caps.PresentationIntervals & pi) )
		{
			pDeviceCombo->pPresentIntervalList->Add( &pi );
		}
	}
}

//
// BuildPresentParamsFromSettings
//
void DX9Enumeration::BuildPresentParamsFromSettings( RenderInfo & renderInfo )
{
	m_d3dpp.Windowed               = m_d3dSettings.IsWindowed;
	m_d3dpp.BackBufferCount        = 1;
	m_d3dpp.MultiSampleType        = m_d3dSettings.MultisampleType();
	m_d3dpp.MultiSampleQuality     = m_d3dSettings.MultisampleQuality();
	m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	m_d3dpp.EnableAutoDepthStencil = AppUsesDepthBuffer;
	m_d3dpp.hDeviceWindow          = (HWND)renderInfo.hMainWindow;
	if( AppUsesDepthBuffer )
	{
		m_d3dpp.Flags              = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
		m_d3dpp.AutoDepthStencilFormat = m_d3dSettings.DepthStencilBufferFormat();
	}
	else
	{
		m_d3dpp.Flags              = 0;
	}

	if( m_d3dSettings.IsWindowed )
	{
		m_d3dpp.BackBufferWidth  = renderInfo.uiTargetWidth;
		m_d3dpp.BackBufferHeight = renderInfo.uiTargetHeight;
		m_d3dpp.BackBufferFormat = m_d3dSettings.PDeviceCombo()->BackBufferFormat;
		m_d3dpp.FullScreen_RefreshRateInHz = 0;
		m_d3dpp.PresentationInterval = m_d3dSettings.PresentInterval();
	}
	else
	{
		m_d3dpp.BackBufferWidth  = m_d3dSettings.DisplayMode().Width;
		m_d3dpp.BackBufferHeight = m_d3dSettings.DisplayMode().Height;
		m_d3dpp.BackBufferFormat = m_d3dSettings.PDeviceCombo()->BackBufferFormat;
		m_d3dpp.FullScreen_RefreshRateInHz = m_d3dSettings.Fullscreen_DisplayMode.RefreshRate;
		m_d3dpp.PresentationInterval = m_d3dSettings.PresentInterval();
	}
}

//
// FindBestWindowedMode
//
bool DX9Enumeration::FindBestWindowedMode( bool bRequireHAL, bool bRequireREF )
{
	// Both can't be true, reference driver trumps both
	if ( bRequireHAL && bRequireREF ) bRequireHAL = false;

	// Get display mode of primary adapter (which is assumed to be where the window 
	// will appear)
	D3DDISPLAYMODE primaryDesktopDisplayMode;
	m_pD3D->GetAdapterDisplayMode(0, &primaryDesktopDisplayMode);

	D3DAdapterInfo* pBestAdapterInfo = NULL;
	D3DDeviceInfo* pBestDeviceInfo = NULL;
	D3DDeviceCombo* pBestDeviceCombo = NULL;

	for( UINT iai = 0; iai < m_pAdapterInfoList->Count(); iai++ )
	{
		D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_pAdapterInfoList->GetPtr(iai);
		for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
		{
			D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
			if (bRequireHAL && pDeviceInfo->DevType != D3DDEVTYPE_HAL)
				continue;
			if (bRequireREF && pDeviceInfo->DevType != D3DDEVTYPE_REF)
				continue;
			for( UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++ )
			{
				D3DDeviceCombo* pDeviceCombo = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr(idc);
				bool bAdapterMatchesBB = (pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat);
				if (!pDeviceCombo->IsWindowed)
					continue;
				if (pDeviceCombo->AdapterFormat != primaryDesktopDisplayMode.Format)
					continue;
				// If we haven't found a compatible DeviceCombo yet, or if this set
				// is better (because it's a HAL, and/or because formats match better),
				// save it
				if( pBestDeviceCombo == NULL || 
					pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDeviceCombo->DevType == D3DDEVTYPE_HAL ||
					pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesBB )
				{
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;
					if( pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesBB )
					{
						// This windowed device combo looks great -- take it
						goto EndWindowedDeviceComboSearch;
					}
					// Otherwise keep looking for a better windowed device combo
				}
			}
		}
	}
EndWindowedDeviceComboSearch:
	if (pBestDeviceCombo == NULL )
		return false;


	m_d3dSettings.pWindowed_AdapterInfo = pBestAdapterInfo;
	m_d3dSettings.pWindowed_DeviceInfo = pBestDeviceInfo;
	m_d3dSettings.pWindowed_DeviceCombo = pBestDeviceCombo;
	m_d3dSettings.IsWindowed = true;
	m_d3dSettings.Windowed_DisplayMode = primaryDesktopDisplayMode;
//	m_d3dSettings.Windowed_Width = m_rcWindowClient.right - m_rcWindowClient.left;
//	m_d3dSettings.Windowed_Height = m_rcWindowClient.bottom - m_rcWindowClient.top;
	if ( AppUsesDepthBuffer )
		m_d3dSettings.Windowed_DepthStencilBufferFormat = *(D3DFORMAT*)pBestDeviceCombo->pDepthStencilFormatList->GetPtr(0);
	m_d3dSettings.Windowed_MultisampleType = *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(0);
	m_d3dSettings.Windowed_MultisampleQuality = 0;
	m_d3dSettings.Windowed_VertexProcessingType = *(VertexProcessingType*)pBestDeviceCombo->pVertexProcessingTypeList->GetPtr(0);
	m_d3dSettings.Windowed_PresentInterval = *(UINT*)pBestDeviceCombo->pPresentIntervalList->GetPtr(0); 

	return true;
}

//
// FindBestFullscreenMode
//
bool DX9Enumeration::FindBestFullscreenMode( bool bRequireHAL, bool bRequireREF )
{
	// Both can't be true, reference driver trumps both
	if ( bRequireHAL && bRequireREF ) bRequireHAL = false;

	// For fullscreen, default to first HAL DeviceCombo that supports the current desktop 
	// display mode, or any display mode if HAL is not compatible with the desktop mode, or 
	// non-HAL if no HAL is available
	D3DDISPLAYMODE adapterDesktopDisplayMode;
	D3DDISPLAYMODE bestAdapterDesktopDisplayMode;
	D3DDISPLAYMODE bestDisplayMode;
	bestAdapterDesktopDisplayMode.Width = 0;
	bestAdapterDesktopDisplayMode.Height = 0;
	bestAdapterDesktopDisplayMode.Format = D3DFMT_UNKNOWN;
	bestAdapterDesktopDisplayMode.RefreshRate = 0;

	D3DAdapterInfo* pBestAdapterInfo = NULL;
	D3DDeviceInfo* pBestDeviceInfo = NULL;
	D3DDeviceCombo* pBestDeviceCombo = NULL;

	for( UINT iai = 0; iai < m_pAdapterInfoList->Count(); iai++ )
	{
		D3DAdapterInfo* pAdapterInfo = (D3DAdapterInfo*)m_pAdapterInfoList->GetPtr(iai);
		m_pD3D->GetAdapterDisplayMode( pAdapterInfo->AdapterOrdinal, &adapterDesktopDisplayMode );
		for( UINT idi = 0; idi < pAdapterInfo->pDeviceInfoList->Count(); idi++ )
		{
			D3DDeviceInfo* pDeviceInfo = (D3DDeviceInfo*)pAdapterInfo->pDeviceInfoList->GetPtr(idi);
			if (bRequireHAL && pDeviceInfo->DevType != D3DDEVTYPE_HAL)
				continue;
			if (bRequireREF && pDeviceInfo->DevType != D3DDEVTYPE_REF)
				continue;
			for( UINT idc = 0; idc < pDeviceInfo->pDeviceComboList->Count(); idc++ )
			{
				D3DDeviceCombo* pDeviceCombo = (D3DDeviceCombo*)pDeviceInfo->pDeviceComboList->GetPtr(idc);
				bool bAdapterMatchesBB = (pDeviceCombo->BackBufferFormat == pDeviceCombo->AdapterFormat);
				bool bAdapterMatchesDesktop = (pDeviceCombo->AdapterFormat == adapterDesktopDisplayMode.Format);
				if (pDeviceCombo->IsWindowed)
					continue;
				// If we haven't found a compatible set yet, or if this set
				// is better (because it's a HAL, and/or because formats match better),
				// save it
				if (pBestDeviceCombo == NULL ||
					pBestDeviceCombo->DevType != D3DDEVTYPE_HAL && pDeviceInfo->DevType == D3DDEVTYPE_HAL ||
					pDeviceCombo->DevType == D3DDEVTYPE_HAL && pBestDeviceCombo->AdapterFormat != adapterDesktopDisplayMode.Format && bAdapterMatchesDesktop ||
					pDeviceCombo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && bAdapterMatchesBB )
				{
					bestAdapterDesktopDisplayMode = adapterDesktopDisplayMode;
					pBestAdapterInfo = pAdapterInfo;
					pBestDeviceInfo = pDeviceInfo;
					pBestDeviceCombo = pDeviceCombo;
					if (pDeviceInfo->DevType == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && bAdapterMatchesBB)
					{
						// This fullscreen device combo looks great -- take it
						goto EndFullscreenDeviceComboSearch;
					}
					// Otherwise keep looking for a better fullscreen device combo
				}
			}
		}
	}
EndFullscreenDeviceComboSearch:
	if (pBestDeviceCombo == NULL)
		return false;

	// Need to find a display mode on the best adapter that uses pBestDeviceCombo->AdapterFormat
	// and is as close to bestAdapterDesktopDisplayMode's res as possible
	bestDisplayMode.Width = 0;
	bestDisplayMode.Height = 0;
	bestDisplayMode.Format = D3DFMT_UNKNOWN;
	bestDisplayMode.RefreshRate = 0;

	for( UINT idm = 0; idm < pBestAdapterInfo->pDisplayModeList->Count(); idm++ )
	{
		D3DDISPLAYMODE* pdm = (D3DDISPLAYMODE*)pBestAdapterInfo->pDisplayModeList->GetPtr(idm);
		if( pdm->Format != pBestDeviceCombo->AdapterFormat )
			continue;
		if( pdm->Width == bestAdapterDesktopDisplayMode.Width &&
			pdm->Height == bestAdapterDesktopDisplayMode.Height && 
			pdm->RefreshRate == bestAdapterDesktopDisplayMode.RefreshRate )
		{
			// found a perfect match, so stop
			bestDisplayMode = *pdm;
			break;
		}
		else if( pdm->Width == bestAdapterDesktopDisplayMode.Width &&
			pdm->Height == bestAdapterDesktopDisplayMode.Height && 
			pdm->RefreshRate > bestDisplayMode.RefreshRate )
		{
			// refresh rate doesn't match, but width/height match, so keep this
			// and keep looking
			bestDisplayMode = *pdm;
		}
		else if( pdm->Width == bestAdapterDesktopDisplayMode.Width )
		{
			// width matches, so keep this and keep looking
			bestDisplayMode = *pdm;
		}
		else if( bestDisplayMode.Width == 0 )
		{
			// we don't have anything better yet, so keep this and keep looking
			bestDisplayMode = *pdm;
		}
	}

	m_d3dSettings.pFullscreen_AdapterInfo = pBestAdapterInfo;
	m_d3dSettings.pFullscreen_DeviceInfo = pBestDeviceInfo;
	m_d3dSettings.pFullscreen_DeviceCombo = pBestDeviceCombo;
	m_d3dSettings.IsWindowed = false;
	m_d3dSettings.Fullscreen_DisplayMode = bestDisplayMode;
	if ( AppUsesDepthBuffer )
		m_d3dSettings.Fullscreen_DepthStencilBufferFormat = *(D3DFORMAT*)pBestDeviceCombo->pDepthStencilFormatList->GetPtr(0);
	m_d3dSettings.Fullscreen_MultisampleType = *(D3DMULTISAMPLE_TYPE*)pBestDeviceCombo->pMultiSampleTypeList->GetPtr(0);
	m_d3dSettings.Fullscreen_MultisampleQuality = 0;
	m_d3dSettings.Fullscreen_VertexProcessingType = *(VertexProcessingType*)pBestDeviceCombo->pVertexProcessingTypeList->GetPtr(0);
	m_d3dSettings.Fullscreen_PresentInterval = D3DPRESENT_INTERVAL_DEFAULT;

	return true;
}

//-----------------------------------------------------------------------------

//
// ColorChannelBits
// Returns the number of color channel bits in the specified D3DFORMAT
//
static UINT ColorChannelBits( D3DFORMAT fmt )
{
	switch( fmt )
	{
	case D3DFMT_R8G8B8:
		return 8;
	case D3DFMT_A8R8G8B8:
		return 8;
	case D3DFMT_X8R8G8B8:
		return 8;
	case D3DFMT_R5G6B5:
		return 5;
	case D3DFMT_X1R5G5B5:
		return 5;
	case D3DFMT_A1R5G5B5:
		return 5;
	case D3DFMT_A4R4G4B4:
		return 4;
	case D3DFMT_R3G3B2:
		return 2;
	case D3DFMT_A8R3G3B2:
		return 2;
	case D3DFMT_X4R4G4B4:
		return 4;
	case D3DFMT_A2B10G10R10:
		return 10;
	case D3DFMT_A2R10G10B10:
		return 10;
	default:
		return 0;
	}
}



//
// AlphaChannelBits
// Returns the number of alpha channel bits in the specified D3DFORMAT
//
static UINT AlphaChannelBits( D3DFORMAT fmt )
{
	switch( fmt )
	{
	case D3DFMT_R8G8B8:
		return 0;
	case D3DFMT_A8R8G8B8:
		return 8;
	case D3DFMT_X8R8G8B8:
		return 0;
	case D3DFMT_R5G6B5:
		return 0;
	case D3DFMT_X1R5G5B5:
		return 0;
	case D3DFMT_A1R5G5B5:
		return 1;
	case D3DFMT_A4R4G4B4:
		return 4;
	case D3DFMT_R3G3B2:
		return 0;
	case D3DFMT_A8R3G3B2:
		return 8;
	case D3DFMT_X4R4G4B4:
		return 0;
	case D3DFMT_A2B10G10R10:
		return 2;
	case D3DFMT_A2R10G10B10:
		return 2;
	default:
		return 0;
	}
}


//
// DepthBits
// Returns the number of depth bits in the specified D3DFORMAT
//
static UINT DepthBits( D3DFORMAT fmt )
{
	switch( fmt )
	{
	case D3DFMT_D16:
		return 16;
	case D3DFMT_D15S1:
		return 15;
	case D3DFMT_D24X8:
		return 24;
	case D3DFMT_D24S8:
		return 24;
	case D3DFMT_D24X4S4:
		return 24;
	case D3DFMT_D32:
		return 32;
	default:
		return 0;
	}
}


//
// StencilBits
// Returns the number of stencil bits in the specified D3DFORMAT
//
static UINT StencilBits( D3DFORMAT fmt )
{
	switch( fmt )
	{
	case D3DFMT_D16:
		return 0;
	case D3DFMT_D15S1:
		return 1;
	case D3DFMT_D24X8:
		return 0;
	case D3DFMT_D24S8:
		return 8;
	case D3DFMT_D24X4S4:
		return 4;
	case D3DFMT_D32:
		return 0;
	default:
		return 0;
	}
}

//
// SortModesCallback
// Used to sort D3DDISPLAYMODEs
//
static int __cdecl SortModesCallback( const void* arg1, const void* arg2 )
{
	D3DDISPLAYMODE* pdm1 = (D3DDISPLAYMODE*)arg1;
	D3DDISPLAYMODE* pdm2 = (D3DDISPLAYMODE*)arg2;

	if (pdm1->Width > pdm2->Width)
		return 1;
	if (pdm1->Width < pdm2->Width)
		return -1;
	if (pdm1->Height > pdm2->Height)
		return 1;
	if (pdm1->Height < pdm2->Height)
		return -1;
	if (pdm1->Format > pdm2->Format)
		return 1;
	if (pdm1->Format < pdm2->Format)
		return -1;
	if (pdm1->RefreshRate > pdm2->RefreshRate)
		return 1;
	if (pdm1->RefreshRate < pdm2->RefreshRate)
		return -1;
	return 0;
}

//
// ConfirmDevice_*
// Various confirmation functions
//
static bool ConfirmDevice_SoftwareVP( D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType, 
										 D3DFORMAT adapterFormat, D3DFORMAT backBufferFormatt )
{
	UNREFERENCED_PARAMETER( pCaps );

	// Only allow software vertex processing
	if ( vertexProcessingType != SOFTWARE_VP )
		return false;
	return true;
}

static bool ConfirmDevice_HardwareVP( D3DCAPS9* pCaps, VertexProcessingType vertexProcessingType, 
										 D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat )
{
	UNREFERENCED_PARAMETER( pCaps );

	// Only allow hardware vertex processing
	if ( vertexProcessingType != HARDWARE_VP )
		return false;
	return true;
}