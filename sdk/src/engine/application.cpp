/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		application.cpp
	Author:		Eric Bryant

	Platform-independent application framework which handles "window"
	creation and system events (mouse, keyboard, etc.)	
*/

#include <windows.h>
#include "katana_config.h"
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "render/rendertypes.h"
#include "render/render.h"
#include "system/systemtimer.h"
#include "gameengine.h"
#include "gamesettings.h"
#include "application.h"

//
// Extern Variables
//
extern shared_ptr<GameSettings> katana_settings;

//
// Only defined for Win32 Applications
//
#if defined APPLICATION_CREATE_DEFAULT_WINDOW && defined _WIN32

	//
	// Local Variables
	//
	const char * KATANA_WINDOW_CLASS = "KatanaApp";

	//
	// Local Functions
	//


	//
	// Win32App
	// This specialize version of Application supports Win32 applications
	//
	class Win32App : public Application
	{
	public:
		Win32App();
	public:
		// Application Overloads
		virtual bool Startup();
		virtual bool Terminate();
		virtual bool Run();
		virtual bool SetPosition(int x, int y, int cx, int cy);
		virtual bool SetFullscreen(bool bEnable);
		virtual bool ShowCursor(bool bEnable);
		virtual bool GetWindowInfo(WindowHandle & hHandle);
		virtual bool GetWindowInfo(WindowHandle & hHandle, int & x, int & y, int & cx, int & cy, bool & full);
		static LRESULT CALLBACK	WinProc (HWND hWnd, unsigned int uiMsg, WPARAM wParam, LPARAM lParam);

	private:
		// Called every game tick
		void GameLoop();

	private:
		HINSTANCE				m_hInst;
		HWND					m_hWnd;
		static const char *		m_lpszWindowClassName;
	};

	//
	// Constructor
	//
	Win32App::Win32App() :
		m_hInst(0),
		m_hWnd(0)
	{
	}

	//
	// Startup
	//
	bool Win32App::Startup()
	{
		// Registers the Windows Class
		WNDCLASS wc = { CS_VREDRAW | CS_HREDRAW | CS_OWNDC, 
						(WNDPROC)WinProc,
						0, 
						0, 
						m_hInst,
						0, 
						0, 
						(HBRUSH)GetStockObject(BLACK_BRUSH),
						0, 
						KATANA_WINDOW_CLASS };

		if ( !RegisterClass(&wc) )
		{
			// If this function errored because the window is already 
			// registered, that's OK
			DWORD dwLastError = GetLastError();
			if ( dwLastError != ERROR_CLASS_ALREADY_EXISTS )
				return FALSE;
		}

		unsigned int uiFlags = WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE;

		// Create render window
		m_hWnd = CreateWindow
		(	
			KATANA_WINDOW_CLASS,
			NULL,
			uiFlags,
			0,												// initial x position
			0,												// initial y position
			katana_settings->resolutionWidth,
			katana_settings->resolutionHeight,
			NULL,											// parent window handle
			NULL,											// window menu handle
			m_hInst,										// program instance handle
			NULL											// creation parameters
		);

		if ( katana_settings->fullscreen ) SetFullscreen( true );

		return true;
	}

	//
	// Terminate
	//
	bool Win32App::Terminate()
	{
		SendMessage( m_hWnd, WM_CLOSE, 0, 0 );
		return true;
	}

	//
	// Run
	//
	bool Win32App::Run()
	{
		::MSG msg;
		memset(&msg, 0, sizeof(msg));

		while ( msg.message != WM_QUIT )
		{
			if ( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )
			{
				if ( msg.message == WM_QUIT )
					return true;
			
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				GameLoop();
			}
		}

		// This is performed after the GameLoop because it may invalidate
		// the underlying render device
		SetFullscreen( false );

		return true;
	}

	//
	// SetPosition
	//
	bool Win32App::SetPosition(int x, int y, int cx, int cy)
	{
		if ( TRUE == MoveWindow(m_hWnd, x, y, cx, cy, TRUE) )
		{
			return true;
		}

		return false;
	}

	//
	// GetWindowInfo
	//
	bool Win32App::GetWindowInfo(WindowHandle & hHandle)
	{
		hHandle = m_hWnd;
		return true;
	}

	bool Win32App::GetWindowInfo(WindowHandle & hHandle, int & x, int & y, int & cx, int & cy, bool & full)
	{
		hHandle = m_hWnd;
		x = 0;
		y = 0;
		cx = 640;
		cy = 480;
		full = false;

		return true;
	}

	//
	// SetFullscreen
	//
	bool Win32App::SetFullscreen(bool bEnable)
	{
		if ( bEnable )
		{
			DEVMODE dmScreenSettings;											// Developer Mode
			memset(&dmScreenSettings, 0, sizeof(DEVMODE));						// Zero Structure

			dmScreenSettings.dmSize			= sizeof(DEVMODE);					// Size Of The Devmode Structure
			dmScreenSettings.dmPelsWidth	= katana_settings->resolutionWidth;	// Screen Width
			dmScreenSettings.dmPelsHeight	= katana_settings->resolutionHeight;// Screen Height
			dmScreenSettings.dmFields		= DM_PELSWIDTH | DM_PELSHEIGHT;		// Pixel Mode

			if ( ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != 
				DISP_CHANGE_SUCCESSFUL )
			{
				return false;
			}
		}
		else
		{
			ChangeDisplaySettings( NULL, 0 );
		}

		return true;
	}

	//
	// ShowCursor
	//
	bool Win32App::ShowCursor(bool bEnable)
	{
		return ( ::ShowCursor(bEnable) ? true : false );
	}

	//
	// GameLoop
	//
	void Win32App::GameLoop()
	{
		m_spGameEngine->m_currentRenderer = m_spRenderer;

		m_spRenderer->BeginFrame();
			m_spGameEngine->OnTick();
		m_spRenderer->EndFrame();
	}

	//
	// Create
	// 
	Application * Application::Create()
	{
		return new Win32App;
	}

	// 
	// WinProc
	//
	LRESULT CALLBACK Win32App::WinProc (HWND hWnd, unsigned int uiMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uiMsg)
		{
			case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}
			case WM_ACTIVATE:
			case WM_CREATE:
			case WM_MOVE:
			case WM_SIZE:
			case WM_TIMER:
			case WM_KEYUP:
			case WM_KEYDOWN:
			case WM_MOUSEMOVE:
				return 0;

			case WM_PAINT: // WM_PAINT has to go to the default winproc
			default:
				return DefWindowProc( hWnd, uiMsg, wParam, lParam );
		};
	}

#endif // KATANA_WINDOWED_APPLICATION