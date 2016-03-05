/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		systemdialog.cpp
	Author:		Eric Bryant

	System Dialog
*/

#include <windows.h>
#include <winuser.h>
#include <tchar.h>
#include "katana_core_includes.h"
#include "base/kstring.h"
#include "base/kexport.h"
#include "script/scriptengine.h"
#include "engine/gamesettings.h"
#include "engine/console.h"
#include "engine/application.h"
#include "systeminfo.h"
#include "systemdialog.h"

//
// Extern Variables
//
extern shared_ptr<GameSettings>	katana_settings;
extern shared_ptr<Console>		katana_console;
extern shared_ptr<ScriptEngine>	katana_script;
extern shared_ptr<Application>	katana_app;

//
// Local Functions
//
INT_PTR CALLBACK DefaultDialogProc(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK KatanaStartupDialogProc(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK KatanaConsoleDialogProc(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK ScriptCommandComboBoxProc(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK KatanaExportDialogProc(HWND,UINT,WPARAM,LPARAM);

//
// Dialog Control Constant IDs
//
#define ID_OK						1000
#define ID_CANCEL					1001
#define ID_STATIC					1002
#define ID_RENDER_ADAPTER			1003
#define ID_RENDER_RESOLUTION		1004
#define ID_RENDER_REFERENCE			1005
#define ID_RENDER_FULLSCREEN		1006
#define ID_STARTUP_SCRIPT			1007
#define ID_SAVE						1008
#define ID_PREVIEW					1009

#define ID_HIDE							1100
#define ID_ALWAYS_ON_TOP				1101
#define ID_CONSOLE_COMMAND				1102
#define ID_CONSOLE_OUTPUT				1103
#define ID_CONSOLE_EXECUTE				1104

#define ID_EXPORT_OBJECTS				1200
#define ID_EXPORT_BSP					1201
#define ID_EXPORT_BIPED					1202
#define ID_EXPORT_ANIMATION				1203
#define ID_ADVANCED_SETTINGS			1204
#define ID_EXPORT_SELECTION_ONLY		1205
#define ID_EXPORT_ENTIRE_SCENE			1206
#define ID_EXPORT_PATH_SOURCE_EDIT		1207
#define ID_EXPORT_PATH_SOURCE_ELLIPSE	1208
#define ID_EXPORT_PATH_DEST_EDIT		1209
#define ID_EXPORT_PATH_DEST_ELLIPSE		1210

#define ID_EXPORT_TEXTURES				1300
#define ID_EXPORT_NORMALS				1301
#define ID_EXPORT_TANGENTS				1302
#define ID_EXPORT_COLORS				1303
#define ID_EXPORT_TEXTURE2				1304
#define ID_EXPORT_ANIMATION_FRAMES		1305

#define ID_EXPORT_ANIMATION_FROM_EDIT	1306
#define ID_EXPORT_ANIMATION_FROM_SPIN	1307
#define ID_EXPORT_ANIMATION_TO_EDIT		1308
#define ID_EXPORT_ANIMATION_TO_SPIN		1309
#define ID_EXPORT_IN_WORLD_SPACE		1310
#define ID_EXPORT_STRIPIFY				1311
#define ID_EXPORT_LODS					1312
#define ID_EXPORT_LODS_EDIT				1313
#define ID_EXPORT_LODS_SPIN				1314


//
// Global Variables
//
WNDPROC g_lpfnScriptCommandComboBoxProc; // original wndproc for the combo box 

/////////////////////////////////////////////////////////////
// SystemDialog
/////////////////////////////////////////////////////////////

//
// Constructor
//
SystemDialog::SystemDialog(const char * caption, unsigned long style, 
						   int x, int y, int w, int h,
						   const char * font, unsigned short fontSize) :
	m_dialogWindow( NULL )
{

    m_usedBufferLength = sizeof(DLGTEMPLATE);
    m_totalBufferLength = m_usedBufferLength;

    m_dialogTemplate = (DialogHandle*)malloc(m_totalBufferLength);

	DLGTEMPLATE * dialogTemplate = (DLGTEMPLATE*)m_dialogTemplate;
    dialogTemplate->style = style;

    if (font != NULL)
    {
        dialogTemplate->style |= DS_SETFONT;
    }

    dialogTemplate->x     = x;
    dialogTemplate->y     = y;
    dialogTemplate->cx    = w;
    dialogTemplate->cy    = h;
    dialogTemplate->cdit  = 0;

    dialogTemplate->dwExtendedStyle = 0;

    // The dialog box doesn't have a menu or a special class

    AppendData(_T("\0"), 2);
    AppendData(_T("\0"), 2);

    // Add the dialog's caption to the template

    AppendString(caption);

    if (font != NULL)
    {
        AppendData(&fontSize, sizeof(unsigned short));
        AppendString(font);
    }
}

//
// Destructor
//
SystemDialog::~SystemDialog()
{
	if ( m_dialogWindow ) DestroyWindow( (HWND)m_dialogWindow );
    free(m_dialogTemplate);
}


//
// Operator DLGTEMPLATE*
// Returns a pointer to the Win32 dialog template which the object
// represents. This pointer may become invalid if additional
// components are added to the template.
//
/*
void operator const DLGTEMPLATE*() const
{
    return m_dialogTemplate;
}
*/

//
// addComponent
//
void SystemDialog::addComponent(const char * type, const char * caption, unsigned long style, unsigned long exStyle,
								int x, int y, int w, int h, unsigned short id)
{
    DLGITEMTEMPLATE item;

    item.style = style;
    item.x     = x;
    item.y     = y;
    item.cx    = w;
    item.cy    = h;
    item.id    = id;

    item.dwExtendedStyle = exStyle;

    AppendData(&item, sizeof(DLGITEMTEMPLATE));

    AppendString(type);
    AppendString(caption);

    unsigned short creationDataLength = 0;
    AppendData(&creationDataLength, sizeof(unsigned short));

    // Increment the component count
	DLGTEMPLATE * dialogTemplate = (DLGTEMPLATE*)m_dialogTemplate;
    dialogTemplate->cdit++;
}

//
// addButton
//
void SystemDialog::addButton(const char * caption, unsigned long style, unsigned long exStyle, int x, int y,
    int w, int h, unsigned short id)
{
    AddStandardComponent(0x0080, caption, style, exStyle, x, y, w, h, id);

    unsigned short creationDataLength = 0;
    AppendData(&creationDataLength, sizeof(unsigned short));
}

//
// addEditBox
//
void SystemDialog::addEditBox(const char * caption, unsigned long style, unsigned long exStyle, int x, int y,
    int w, int h, unsigned short id)
{
    AddStandardComponent(0x0081, caption, style, exStyle, x, y, w, h, id);

    unsigned short creationDataLength = 0;
    AppendData(&creationDataLength, sizeof(unsigned short));
}

//
// addStatic
//
void SystemDialog::addStatic(const char * caption, unsigned long style, unsigned long exStyle, int x, int y,
    int w, int h, unsigned short id)
{
    AddStandardComponent(0x0082, caption, style, exStyle, x, y, w, h, id);

    unsigned short creationDataLength = 0;
    AppendData(&creationDataLength, sizeof(unsigned short));
}

//
// addListBox
//
void SystemDialog::addListBox(const char * caption, unsigned long style, unsigned long exStyle, int x, int y,
    int w, int h, unsigned short id)
{
    AddStandardComponent(0x0083, caption, style, exStyle, x, y, w, h, id);

    unsigned short creationDataLength = 0;
    AppendData(&creationDataLength, sizeof(unsigned short));
}

//
// addScrollBar
//
void SystemDialog::addScrollBar(const char * caption, unsigned long style, unsigned long exStyle, int x, int y,
    int w, int h, unsigned short id)
{
    AddStandardComponent(0x0084, caption, style, exStyle, x, y, w, h, id);

    unsigned short creationDataLength = 0;
    AppendData(&creationDataLength, sizeof(unsigned short));
}

//
// addComboBox
//
void SystemDialog::addComboBox(const char * caption, unsigned long style, unsigned long exStyle, int x, int y,
    int w, int h, unsigned short id)
{
    AddStandardComponent(0x0085, caption, style, exStyle, x, y, w, h, id);

    unsigned short creationDataLength = 0;
    AppendData(&creationDataLength, sizeof(unsigned short));
}

//
// AddStandardComponent
//
void SystemDialog::AddStandardComponent(unsigned short type, const char * caption, unsigned long style,
										unsigned long exStyle, int x, int y, int w, int h, unsigned short id)
{

    DLGITEMTEMPLATE item;

    // unsigned long algin the beginning of the component data

    AlignData(sizeof(unsigned long));

    item.style = style;
    item.x     = x;
    item.y     = y;
    item.cx    = w;
    item.cy    = h;
    item.id    = id;

    item.dwExtendedStyle = exStyle;

    AppendData(&item, sizeof(DLGITEMTEMPLATE));

    unsigned short preType = 0xFFFF;

    AppendData(&preType, sizeof(unsigned short));
    AppendData(&type, sizeof(unsigned short));

    AppendString(caption);

    // Increment the component count
	DLGTEMPLATE * dialogTemplate = (DLGTEMPLATE*)m_dialogTemplate;
    dialogTemplate->cdit++;
}

//
// AlignData
//
void SystemDialog::AlignData(int size)
{
    int paddingSize = m_usedBufferLength % size;

    if (paddingSize != 0)
    {
        EnsureSpace(paddingSize);
        m_usedBufferLength += paddingSize;
    }
}

//
// AppendString
//
void SystemDialog::AppendString(const char * string)
{
    int length = MultiByteToWideChar(CP_ACP, 0, string, -1, NULL, 0);

    WCHAR* wideString = (WCHAR*)malloc(sizeof(WCHAR) * length);
    MultiByteToWideChar(CP_ACP, 0, string, -1, wideString, length);

    AppendData(wideString, length * sizeof(WCHAR));
    free(wideString);

}

//
// AppendData
//
void SystemDialog::AppendData(void* data, int dataLength)
{
    EnsureSpace(dataLength);
	DLGTEMPLATE * dialogTemplate = (DLGTEMPLATE*)m_dialogTemplate;
    memcpy((char*)dialogTemplate + m_usedBufferLength, data, dataLength);
    m_usedBufferLength += dataLength;

}

//
// EnsureSpace
//
void SystemDialog::EnsureSpace(int length)
{
    if (length + m_usedBufferLength > m_totalBufferLength)
    {
		DLGTEMPLATE * dialogTemplate = (DLGTEMPLATE*)m_dialogTemplate;
        m_totalBufferLength += length * 2;

        void* newBuffer = malloc(m_totalBufferLength);
        memcpy(newBuffer, dialogTemplate, m_usedBufferLength);

        free(m_dialogTemplate);
        m_dialogTemplate = (DialogHandle*)newBuffer;
    }
}

//
// showDialog
//
bool SystemDialog::showDialog( void * pfDialogProcedure )
{
	if ( !pfDialogProcedure )
	{
		return DialogBoxIndirect( GetModuleHandle( 0 ),
					  			 (DLGTEMPLATE *) m_dialogTemplate,
								 NULL,
								 DefaultDialogProc ) ? TRUE : FALSE;
	}
	else
	{
		return DialogBoxIndirect( GetModuleHandle( 0 ),
					  			 (DLGTEMPLATE *) m_dialogTemplate,
								 NULL,
								 (DLGPROC)pfDialogProcedure ) ? TRUE : FALSE;
	}
}

/////////////////////////////////////////////////////////////
// KatanaStartupDialog
/////////////////////////////////////////////////////////////

//
// Globals
//
std::vector<std::string> g_deviceNames;
std::vector<std::string> g_deviceModeNames[ SystemInfo::MAX_DISPLAY_MODES ];
std::vector<DisplayMode> g_deviceModes[ SystemInfo::MAX_DISPLAY_MODES ];

//
// Constructor
//
KatanaStartupDialog::KatanaStartupDialog( SystemInfo & sysInfo ) :
	SystemDialog( "Katana Engine", WS_CAPTION | DS_CENTER, 0, 0, 232, 151, "MS Shell Dlg" )
{
	// Add the OK/Cancel Buttons
	addButton( "OK", WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP, 0, 120, 130, 50, 14, ID_OK );
	addButton( "Cancel", WS_VISIBLE | WS_TABSTOP, 0, 175, 130, 50, 14, ID_CANCEL );
	
	// Add the display parameters
	addStatic( "Render Adapter:", WS_VISIBLE, 0, 14, 21, 55, 8, ID_STATIC );
	addComboBox( "", WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 0, 82, 19, 136, 64, ID_RENDER_ADAPTER );
//	addButton( "Use Reference Driver", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 81, 36, 83, 9, ID_RENDER_REFERENCE );

	addStatic( "Screen Resolution:", WS_VISIBLE, 0, 14, 54, 63, 8, ID_STATIC );
	addComboBox( "", WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 0, 82, 54, 136, 64, ID_RENDER_RESOLUTION );
	addButton( "Fullscreen", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 81, 70, 83, 9, ID_RENDER_FULLSCREEN );

	// Add the startup scrip combo box
	addComboBox( "", WS_VISIBLE | CBS_DROPDOWN | WS_VSCROLL | WS_TABSTOP, 0, 14, 102, 203, 64, ID_STARTUP_SCRIPT );

	// Add the groups
	addButton( "Display", WS_VISIBLE | BS_GROUPBOX | WS_GROUP, 0, 7, 7, 218, 78, ID_STATIC );
	addButton( "Startup Script", WS_VISIBLE | BS_GROUPBOX | WS_GROUP, 0, 7, 88, 218, 36, ID_STATIC );

	// Enumerate all the display devices/modes and fill in the vectors
	g_deviceNames = sysInfo.getRenderDevices();

	for( unsigned int device = 0; device < g_deviceNames.size(); device++ )
	{
		g_deviceModes[device] = sysInfo.getDisplayModes( device );

		for( unsigned int mode = 0; mode < g_deviceModes[device].size(); mode++ )
		{
			kstring displayMode;
			displayMode.format( "%d x %d x %d", 
				g_deviceModes[device][mode].width, 
				g_deviceModes[device][mode].height,
				g_deviceModes[device][mode].bitDepth );

			g_deviceModeNames[device].push_back( displayMode );
		}
	}
}

//
// showDialog
//
bool KatanaStartupDialog::showDialog( void * pfDialogProcedure )
{
	return SystemDialog::showDialog( KatanaStartupDialogProc );
}

//
// SaveKatanaSettings
// 
void SaveKatanaSettings( HWND hwndDlg )
{
	TCHAR szStartupScript[1024];
	TCHAR szDeviceName[1024];

	// Retrieve the parameters from the dialog
	GetWindowText( GetDlgItem( hwndDlg, ID_STARTUP_SCRIPT ), szStartupScript, 1024 );
	GetWindowText( GetDlgItem( hwndDlg, ID_RENDER_ADAPTER ), szDeviceName, 1024 );
	int iDevice = (int)SendMessage( GetDlgItem( hwndDlg, ID_RENDER_ADAPTER ), CB_GETCURSEL, 0, (LPARAM) 0 );
	int iResolutionIndex = (int)SendMessage( GetDlgItem( hwndDlg, ID_RENDER_RESOLUTION ), CB_GETCURSEL, 0, (LPARAM) 0 );
	bool bFullscreen = SendMessage( GetDlgItem( hwndDlg, ID_RENDER_FULLSCREEN ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bReference = SendMessage( GetDlgItem( hwndDlg, ID_RENDER_REFERENCE ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;

	// Only add the startup script to the history if it isn't already contained
	std::vector<string>::iterator iter = std::find( katana_settings->startupScriptHistory.begin(),
													katana_settings->startupScriptHistory.end(),
													string( szStartupScript ) );
	if ( iter == katana_settings->startupScriptHistory.end() )
	{
		katana_settings->startupScriptHistory.insert( katana_settings->startupScriptHistory.begin(), szStartupScript );
	}
	else
	{
		// Otherwise, if we've selected an script already in history, 
		// swap it so that it's in the front of the history list
		std::swap( *katana_settings->startupScriptHistory.begin() , *iter);
	}

	// Store them in the script
	katana_settings->deviceName = szDeviceName;
	katana_settings->startupScript = szStartupScript;
	katana_settings->resolutionWidth = g_deviceModes[iDevice][iResolutionIndex].width;
	katana_settings->resolutionHeight= g_deviceModes[iDevice][iResolutionIndex].height;
	katana_settings->resolutionBitDepth = g_deviceModes[iDevice][iResolutionIndex].bitDepth;
	katana_settings->fullscreen = bFullscreen;
	katana_settings->useReferenceDriver = bReference;
}

//
// KatanaStartupDialogProc
//
INT_PTR CALLBACK KatanaStartupDialogProc( HWND hwndDlg,  // handle to dialog box
										  UINT uMsg,     // message
										  WPARAM wParam, // first message parameter
										  LPARAM lParam  // second message parameter
)
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			HWND hAdapter = GetDlgItem( hwndDlg, ID_RENDER_ADAPTER );
			HWND hResolution = GetDlgItem( hwndDlg, ID_RENDER_RESOLUTION );
			HWND hStartup = GetDlgItem( hwndDlg, ID_STARTUP_SCRIPT);
			HWND hFullscreen = GetDlgItem( hwndDlg, ID_RENDER_FULLSCREEN );
			HWND hReference = GetDlgItem( hwndDlg, ID_RENDER_REFERENCE );

			// Fill in the adapter listbox
			int iAdapterIndex = 0;
			for( unsigned int device = 0; device < g_deviceNames.size(); device++ )
			{
				if ( g_deviceNames[device] == katana_settings->deviceName )
					iAdapterIndex = device;

				SendMessage( hAdapter, CB_ADDSTRING, 0, (LPARAM) g_deviceNames[device].c_str() ); 
			}

			// Fill in the resolution listbox
			int iResolutionIndex = 0;
			for( unsigned int mode = 0; mode < g_deviceModes[0].size(); mode++ )
			{
				if ( g_deviceModes[0][mode].width == katana_settings->resolutionWidth && 
					 g_deviceModes[0][mode].height == katana_settings->resolutionHeight && 
					 g_deviceModes[0][mode].bitDepth == katana_settings->resolutionBitDepth )
				{
					iResolutionIndex = mode;
				}

				SendMessage( hResolution, CB_ADDSTRING, 0, (LPARAM) g_deviceModeNames[0][mode].c_str() ); 
			}

			// Fill in the history lisbox
			for( unsigned int history = 0; history < katana_settings->startupScriptHistory.size(); history++ )
			{
				SendMessage( hStartup, CB_ADDSTRING, 0, (LPARAM) katana_settings->startupScriptHistory[history].c_str() );
			}

			// Set/Unset the check boxes
			SendMessage( hReference, BM_SETCHECK, katana_settings->useReferenceDriver, (LPARAM) 0 );
			SendMessage( hFullscreen, BM_SETCHECK, katana_settings->fullscreen, (LPARAM) 0 );

			// If necessary, disable the adapter window
			if ( katana_settings->useReferenceDriver ) EnableWindow( hAdapter, FALSE );

			// Setup the current selections
			SendMessage( hAdapter, CB_SETCURSEL, iAdapterIndex, (LPARAM) 0 );
			SendMessage( hResolution, CB_SETCURSEL, iResolutionIndex, (LPARAM) 0 );

			// Set current script
			SetWindowText( hStartup, katana_settings->startupScript.c_str() );

			return TRUE;
		}

		case WM_COMMAND:
		{
            switch ( LOWORD( wParam ) )
            {
				case ID_RENDER_REFERENCE:
				{
					bool refOn = SendMessage( GetDlgItem( hwndDlg, ID_RENDER_REFERENCE ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? false : true;
					refOn = !refOn;
					if ( refOn )
						EnableWindow( GetDlgItem( hwndDlg, ID_RENDER_ADAPTER ), FALSE );
					else
						EnableWindow( GetDlgItem( hwndDlg, ID_RENDER_ADAPTER ), TRUE );
					return TRUE;
				}
				case ID_CANCEL:
				{
					EndDialog( hwndDlg, FALSE );
					return TRUE;
				}
				case ID_OK:
				{
					SaveKatanaSettings( hwndDlg );
					EndDialog( hwndDlg, TRUE );
					return TRUE;
				}
			}
		}
	};

	return FALSE;
}

/////////////////////////////////////////////////////////////
// KatanaExportDialog
/////////////////////////////////////////////////////////////

//
// Globals
//

kexport *			g_exporter			= NULL;
ExportSettings *	g_exportSettings;
const char *		g_ExportTextureStrings[] = { "No Textures",
												 "Export Textures (included within file)",
												 "Export Full Texture Path",
											     "Export Texture Names Only" };

//
// Function Prototypes
//
INT_PTR CALLBACK Advanced_SceneObject_SettingsDialogProc(HWND,UINT,WPARAM ,LPARAM);
bool DoExport( HWND hDlgHwnd, bool bPreview );
void DoAdvancedSceneObjectSettings( HWND hDlgHwnd );
//
// Advanced_SceneObject_Settings
//
class Advanced_SceneObject_Settings : public SystemDialog
{
public:
	// Constructor
	Advanced_SceneObject_Settings() :
		SystemDialog( "Advanced Settings: Scene Object(s)", WS_CAPTION | DS_CENTER, 0, 0, 269, 220, "MS Shell Dlg" )
	{
		// Add the OK/Cancel Buttons
		addButton( "OK", WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP, 0, 152, 199, 50, 14, ID_OK );
		addButton( "Cancel", WS_VISIBLE | WS_TABSTOP, 0, 212, 199, 50, 14, ID_CANCEL );

		// Add the Texture List Selection
		addComboBox( "", WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP, 0, 15, 18, 239, 64, ID_EXPORT_TEXTURES );

		// Add Vertex Information Checkboxes
		addButton( "Export Normals", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 15, 56, 76, 8, ID_EXPORT_NORMALS );
		addButton( "Export Tangents", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 15, 68, 76, 8, ID_EXPORT_TANGENTS );
		addButton( "Export Colors", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 96, 56, 76, 8, ID_EXPORT_COLORS );
		addButton( "Export Secondardy Texture Coordinates", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 96, 68, 150, 8, ID_EXPORT_TEXTURE2 );

		// Add Animation Controls
		addButton( "Export Animation from Frames:", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 15, 100, 116, 16, ID_EXPORT_ANIMATION_FRAMES );
		addEditBox( "", WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, WS_EX_CLIENTEDGE, 136, 102, 25, 13, ID_EXPORT_ANIMATION_FROM_EDIT );
		addEditBox( "", WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, WS_EX_CLIENTEDGE, 207, 102, 25, 13, ID_EXPORT_ANIMATION_TO_EDIT );
		addStatic( "To:", WS_VISIBLE, 0, 183, 104, 16, 11, ID_STATIC );

		// Add Extra Checkboxes
		addButton( "Export Objects in World Space", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 16, 145, 114, 8, ID_EXPORT_IN_WORLD_SPACE );
		addButton( "Stripify Geometry", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 16, 158, 76, 8, ID_EXPORT_STRIPIFY );
		addButton( "Generate Static LODs:", WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 0, 16, 171, 84, 8, ID_EXPORT_LODS );
		addEditBox( "", WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP, WS_EX_CLIENTEDGE, 103, 169, 25, 13, ID_EXPORT_LODS_EDIT );

		// Add the groups
		addButton( "Texture Information", WS_VISIBLE | BS_GROUPBOX | WS_GROUP, 0, 7, 7, 255, 31, ID_STATIC );
		addButton( "Vertex Information", WS_VISIBLE | BS_GROUPBOX | WS_GROUP, 0, 7, 42, 255, 43, ID_STATIC );
		addButton( "Animation", WS_VISIBLE | BS_GROUPBOX | WS_GROUP, 0, 7, 89, 255, 37, ID_STATIC );
		addButton( "Extras", WS_VISIBLE | BS_GROUPBOX | WS_GROUP, 0, 7, 131, 255, 59, ID_STATIC );

/*
	CONTROL         "",IDC_SPIN2,"msctls_updown32",UDS_ARROWKEYS,161,101,12,15
	CONTROL         "",IDC_SPIN3,"msctls_updown32",UDS_ARROWKEYS,232,101,11,15
	CONTROL         "",IDC_SPIN4,"msctls_updown32",UDS_ARROWKEYS,128,168,11,15
 */
	}
};

//
// Constructor
//
KatanaExportDialog::KatanaExportDialog( kexport & exportImpl, ExportSettings & exportSettings ) :
	SystemDialog( "Katana Export Settings", WS_CAPTION | DS_CENTER, 0, 0, 287, 236, "MS Shell Dlg" )
{
	// Add the Preview/Save/Cancel Buttons
	addButton( "Preview", WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP, 0, 7, 208, 62, 16, ID_PREVIEW	 );
	addButton( "Save", WS_VISIBLE | BS_DEFPUSHBUTTON | WS_TABSTOP, 0, 152, 208, 62, 16, ID_SAVE );
	addButton( "Close", WS_VISIBLE | WS_TABSTOP, 0, 218, 208, 62, 16, ID_CANCEL );

	// Add Export Type Radio Buttons
	addButton( "Scene Object(s)", WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, 0, 16, 19, 80,  12, ID_EXPORT_OBJECTS );
	addButton( "Scene BSP", WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 0, 16, 32, 80, 12, ID_EXPORT_BSP );
	addButton( "Biped Skin and Skeleton", WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP | WS_DISABLED, 0, 16, 44, 100, 12, ID_EXPORT_BIPED );
	addButton( "Animation", WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP | WS_DISABLED, 0, 16, 58, 80, 12, ID_EXPORT_ANIMATION );

	// Add Advanced Settings Button
	addButton( "Advanced Settings...", WS_VISIBLE | WS_TABSTOP, 0, 15, 89, 84, 14, ID_ADVANCED_SETTINGS );

	// Add Export Scope Radio Buttons
	addButton( "Export Selection Only", WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, 0, 16, 122, 100, 12, ID_EXPORT_SELECTION_ONLY );
	addButton( "Export Entire Scene", WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 0, 16, 136, 80, 12, ID_EXPORT_ENTIRE_SCENE );

	// Add Path Text Box and File Selection Button
	addComboBox( "", WS_VISIBLE | CBS_DROPDOWN | WS_HSCROLL | WS_TABSTOP, 0, 61, 168, 195, 50, ID_EXPORT_PATH_SOURCE_EDIT );
	addButton( "...", WS_VISIBLE | WS_TABSTOP, 0, 257, 168, 16, 12, ID_EXPORT_PATH_SOURCE_ELLIPSE );
	addComboBox( "", WS_VISIBLE | CBS_DROPDOWN | WS_HSCROLL | WS_TABSTOP, 0, 61, 184, 195, 50, ID_EXPORT_PATH_DEST_EDIT );
	addButton( "...", WS_VISIBLE | WS_TABSTOP, 0, 257, 184, 16, 12, ID_EXPORT_PATH_DEST_ELLIPSE );
	addStatic( "Source:", WS_VISIBLE, 0, 16, 170, 25, 9, ID_STATIC );
	addStatic( "Destination:", WS_VISIBLE, 0, 15, 185, 38, 9, ID_STATIC );

	// Add the groups
	addButton( "1. Choose Export Type", WS_VISIBLE | BS_GROUPBOX | WS_GROUP, 0, 7, 7, 273, 68, ID_STATIC );
	addButton( "1a. Configure Advanced Settings", WS_VISIBLE | BS_GROUPBOX | WS_GROUP, 0, 7, 77, 273, 32, ID_STATIC );
	addButton( "2. Choose Export Scope", WS_VISIBLE | BS_GROUPBOX | WS_GROUP, 0, 7, 111, 273, 41, ID_STATIC );
	addButton( "3. Enter Export Paths", WS_VISIBLE | BS_GROUPBOX | WS_GROUP, 0, 7, 157, 273, 47, ID_STATIC );

	// Store the exporter
	g_exporter = &exportImpl;

	// Store the export settings
	g_exportSettings = &exportSettings;
}

//
// showDialog
//
bool KatanaExportDialog::showDialog( void * pfDialogProcedure )
{
	return SystemDialog::showDialog( KatanaExportDialogProc );
}

//
// KatanaStartupDialogProc
//
INT_PTR CALLBACK KatanaExportDialogProc( HWND hwndDlg,  // handle to dialog box
										 UINT uMsg,     // message
										 WPARAM wParam, // first message parameter
										 LPARAM lParam  // second message parameter
										 )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			// Setup the defaults for the radio buttons
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_OBJECTS ), BM_SETCHECK, TRUE, (LPARAM) 0 );
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_SELECTION_ONLY ), BM_SETCHECK, g_exportSettings->selectionOnly, (LPARAM) 0 );
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_ENTIRE_SCENE ), BM_SETCHECK, !g_exportSettings->selectionOnly, (LPARAM) 0 );

			return TRUE;
		}

		case WM_COMMAND:
		{
			switch ( LOWORD( wParam ) )
			{
				case ID_EXPORT_PATH_SOURCE_ELLIPSE:
				case ID_EXPORT_PATH_DEST_ELLIPSE:
				{
					OPENFILENAME ofn;       // common dialog box structure
					TCHAR szFile[1024];       // buffer for file name

					// Initialize OPENFILENAME
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = hwndDlg;
					ofn.lpstrFile = szFile;

					// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
					// use the contents of szFile to initialize itself.
					//
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(szFile);
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;

					if ( LOWORD( wParam ) == ID_EXPORT_PATH_SOURCE_ELLIPSE ) {
						ofn.lpstrFilter = "Maya Files\0*.ma;*.mb\0";
						ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
					} else {
						ofn.lpstrFilter = "Katana Files\0*.kf\0";
						ofn.Flags = OFN_NOCHANGEDIR;
					}

					if ( GetOpenFileName( &ofn ) )
					{
						if ( LOWORD( wParam ) == ID_EXPORT_PATH_SOURCE_ELLIPSE )
							SetWindowText( GetDlgItem( hwndDlg, ID_EXPORT_PATH_SOURCE_EDIT ), ofn.lpstrFile );
						else
							SetWindowText( GetDlgItem( hwndDlg, ID_EXPORT_PATH_DEST_EDIT ), ofn.lpstrFile );
					}
					break;
				}
				case ID_PREVIEW:
				{
					DoExport( hwndDlg, TRUE );
					return TRUE;
				}
				case ID_SAVE:
				{
					DoExport( hwndDlg, FALSE );
					return TRUE;
				}
				case ID_CANCEL:
				{
					EndDialog( hwndDlg, TRUE );
					return TRUE;
				}
				case ID_ADVANCED_SETTINGS:
				{
					// TODO: Support the other advanced settings
					Advanced_SceneObject_Settings advancedGeometry;
					advancedGeometry.showDialog( Advanced_SceneObject_SettingsDialogProc );
				}
			}
		}
	};

	return FALSE;
}

//
// Advanced_SceneObject_SettingsDialogProc
//
INT_PTR CALLBACK Advanced_SceneObject_SettingsDialogProc( HWND hwndDlg,  // handle to dialog box
														  UINT uMsg,     // message
														  WPARAM wParam, // first message parameter
														  LPARAM lParam  // second message parameter
)
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
		{
			HWND hExportTexutres = GetDlgItem( hwndDlg, ID_EXPORT_TEXTURES );

			// Add Strings to the Texture Listbox
			SendMessage( hExportTexutres, CB_ADDSTRING, 0, (LPARAM)g_ExportTextureStrings[0] );
			SendMessage( hExportTexutres, CB_ADDSTRING, 0, (LPARAM)g_ExportTextureStrings[1] );
			SendMessage( hExportTexutres, CB_ADDSTRING, 0, (LPARAM)g_ExportTextureStrings[2] );
			SendMessage( hExportTexutres, CB_ADDSTRING, 0, (LPARAM)g_ExportTextureStrings[3] );

			// Setup the current selections
			int nTextureOption;
			if ( !g_exportSettings->includeTextures ) nTextureOption = 0;
			else if ( g_exportSettings->embeddedTextures ) nTextureOption = 1;
			else if ( !g_exportSettings->textureNameOnly ) nTextureOption = 2;
			else nTextureOption = 3;

			SendMessage( hExportTexutres, CB_SETCURSEL, nTextureOption , (LPARAM)0);

			// Setup the default checkboxes
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_NORMALS ), BM_SETCHECK, g_exportSettings->computeNormals, (LPARAM) 0 );
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_TANGENTS ), BM_SETCHECK, g_exportSettings->computeTangents, (LPARAM) 0 );
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_COLORS ), BM_SETCHECK, g_exportSettings->computeColors, (LPARAM) 0 );
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_TEXTURE2 ), BM_SETCHECK, g_exportSettings->computeSecondaryTextureCoordinates, (LPARAM) 0 );
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_FRAMES ), BM_SETCHECK, g_exportSettings->includeAnimation, (LPARAM) 0 );
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_LODS ), BM_SETCHECK, g_exportSettings->computeNumLODs > 0, (LPARAM) 0 );
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_IN_WORLD_SPACE ), BM_SETCHECK, g_exportSettings->exportInWorldSpace, (LPARAM) 0 );
			SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_STRIPIFY ), BM_SETCHECK, g_exportSettings->stripifyGeometry, (LPARAM) 0 );

			// Setup the default text
			char szFromFrame[32], szToFrame[32], szLOD[32];
			SetWindowText( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_FROM_EDIT ), itoa( g_exportSettings->animStartFrame, szFromFrame, 10 ) );
			SetWindowText( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_TO_EDIT ), itoa( g_exportSettings->animEndFrame, szToFrame, 10 ) );
			SetWindowText( GetDlgItem( hwndDlg, ID_EXPORT_LODS_EDIT ), itoa( g_exportSettings->computeNumLODs, szLOD, 10 ) );

			// Enable the appropiate windows
			if ( !g_exportSettings->includeAnimation ) {
				EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_FROM_EDIT ), FALSE );
				EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_TO_EDIT ), FALSE );
			}

			if ( !g_exportSettings->computeNumLODs ) {
				EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_LODS_EDIT ), FALSE );
			}

			return TRUE;
		}

		case WM_COMMAND:
		{
			switch ( LOWORD( wParam ) )
			{
				case ID_EXPORT_ANIMATION_FRAMES:
				{
					bool bExportAnimation = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_FRAMES ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
					EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_FROM_EDIT ), bExportAnimation );
					EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_TO_EDIT ), bExportAnimation );
					break;
				}
				case ID_EXPORT_LODS:
					{
						bool bLODs = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_LODS ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
						EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_LODS_EDIT ), bLODs );
						break;
					}
				case ID_CANCEL:
				{
					EndDialog( hwndDlg, FALSE );
					return TRUE;
				}
				case ID_OK:
				{
					DoAdvancedSceneObjectSettings( hwndDlg );
					EndDialog( hwndDlg, TRUE );
					return TRUE;
				}
			}
		}
	};

	return FALSE;
}


bool DoExport( HWND hwndDlg, bool bPreview )
{
	// Gather the parameters
	TCHAR szSourcePath[1024];
	TCHAR szDestinationPath[1024];

	// Retrieve the parameters from the dialog
	GetWindowText( GetDlgItem( hwndDlg, ID_EXPORT_PATH_SOURCE_EDIT ), szSourcePath, 1024 );
	GetWindowText( GetDlgItem( hwndDlg, ID_EXPORT_PATH_DEST_EDIT ), szDestinationPath, 1024 );
	bool bExportObjects = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_OBJECTS ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bExportBSP = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_BSP ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bExportBiped = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_BIPED ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bExportAnimation = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bExportScene = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_SELECTION_ONLY ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bExportSelection = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_ENTIRE_SCENE ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;

	// Replace "\" with "/" (Maya doesn't like "\"'s)
	char * replaceCh;
	while ( ( replaceCh = strchr( szSourcePath, '\\' ) ) != 0 ) { *replaceCh = '/'; }

	// Fill the export settings
	g_exportSettings->exportFileSourcePath = szSourcePath;
	g_exportSettings->exportFileDestinationPath = szDestinationPath;
	g_exportSettings->exportType = ( bExportObjects ) ? EXPORT_GEOMETRY :
								  ( bExportBSP ) ? EXPORT_BSP :
								  ( bExportBiped ) ? EXPORT_BIPED : EXPORT_ANIMATION;
	g_exportSettings->selectionOnly = bExportSelection;
	g_exportSettings->previewOnly = bPreview;

	// Setup the Dialog Window Text as Exporting
	SetWindowText( hwndDlg, _T( "Exporting...") );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_OBJECTS ), FALSE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_BSP ), FALSE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_PATH_SOURCE_EDIT ), FALSE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_PATH_DEST_EDIT ), FALSE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_PATH_SOURCE_ELLIPSE ), FALSE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_PATH_DEST_ELLIPSE ), FALSE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_SELECTION_ONLY ), FALSE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_ENTIRE_SCENE ), FALSE );
	EnableWindow( GetDlgItem( hwndDlg, ID_ADVANCED_SETTINGS ), FALSE );
	EnableWindow( GetDlgItem( hwndDlg, ID_PREVIEW ), FALSE );
	EnableWindow( GetDlgItem( hwndDlg, ID_SAVE ), FALSE );
	UpdateWindow( hwndDlg );

	// Execute the export
	bool bResult = g_exporter->enumerateScene( *g_exportSettings );

	if ( !bPreview )
	{
		if ( bResult )
			MessageBox( hwndDlg, "Successful.", "Export Operation", MB_OK );
		else
			MessageBox( hwndDlg, g_exportSettings->lastExportError.c_str(), "ERROR", MB_ICONERROR );
	}

	// Restore the Window
	SetWindowText( hwndDlg, _T( "Katana Export Settings") );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_OBJECTS ), TRUE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_BSP ), TRUE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_PATH_SOURCE_EDIT ), TRUE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_PATH_DEST_EDIT ), TRUE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_PATH_SOURCE_ELLIPSE ), TRUE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_PATH_DEST_ELLIPSE ), TRUE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_SELECTION_ONLY ), TRUE );
	EnableWindow( GetDlgItem( hwndDlg, ID_EXPORT_ENTIRE_SCENE ), TRUE );
	EnableWindow( GetDlgItem( hwndDlg, ID_ADVANCED_SETTINGS ), TRUE );
	EnableWindow( GetDlgItem( hwndDlg, ID_PREVIEW ), TRUE );
	EnableWindow( GetDlgItem( hwndDlg, ID_SAVE ), TRUE );
	UpdateWindow( hwndDlg );

	return bResult;
}

void DoAdvancedSceneObjectSettings( HWND hwndDlg )
{
	char szFromFrame[32], szToFrame[32], szLOD[32];
	int nTextureExport = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_TEXTURES ), CB_GETCURSEL, 0, (LPARAM) 0 );
	bool bExportNormals = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_NORMALS ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bExportTangents = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_TANGENTS ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bExportColors = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_COLORS ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bExportSecondaryTextureCoordinates = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_TEXTURE2 ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bAnimation = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_FRAMES ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bExportInWorldSpace = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_IN_WORLD_SPACE ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bStripify = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_STRIPIFY ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	bool bStaticLOD = SendMessage( GetDlgItem( hwndDlg, ID_EXPORT_LODS ), BM_GETCHECK, 0, (LPARAM) 0 ) == BST_CHECKED ? true : false;
	GetWindowText( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_FROM_EDIT ), szFromFrame, 1024 );
	GetWindowText( GetDlgItem( hwndDlg, ID_EXPORT_ANIMATION_TO_EDIT ), szToFrame, 1024 );
	GetWindowText( GetDlgItem( hwndDlg, ID_EXPORT_LODS_EDIT ), szLOD, 1024 );

	// Fill the export settings
	g_exportSettings->computeTangents = bExportTangents;
	g_exportSettings->computeNormals = bExportNormals;
	g_exportSettings->computeColors = bExportColors;
	g_exportSettings->computeSecondaryTextureCoordinates = bExportSecondaryTextureCoordinates;
	g_exportSettings->exportInWorldSpace = bExportInWorldSpace;
	g_exportSettings->stripifyGeometry = bStripify;
	g_exportSettings->computeNumLODs = ( bStaticLOD ? atoi( szLOD ) : 0 );
	g_exportSettings->includeAnimation = bAnimation;
	g_exportSettings->animStartFrame = atoi( szFromFrame );
	g_exportSettings->animEndFrame = atoi( szToFrame );
	g_exportSettings->includeTextures = ( nTextureExport > 0 );
	g_exportSettings->textureNameOnly = ( nTextureExport > 2 );
	g_exportSettings->embeddedTextures = ( nTextureExport == 1 );
}

/////////////////////////////////////////////////////////////
// Local Functions
/////////////////////////////////////////////////////////////

//
// DefaultDialogProc
//
INT_PTR CALLBACK DefaultDialogProc( HWND hwndDlg,  // handle to dialog box
								    UINT uMsg,     // message
								    WPARAM wParam, // first message parameter
								    LPARAM lParam  // second message parameter
)
{
	switch ( uMsg )
	{
		case WM_COMMAND:
		{
            switch ( LOWORD( wParam ) )
            {
				case ID_CANCEL:
				{
					EndDialog( hwndDlg, FALSE );
					return TRUE;
				}
				case ID_OK:
				{
					EndDialog( hwndDlg, TRUE );
					return TRUE;
				}
			}
		}
	};

	return FALSE;
}