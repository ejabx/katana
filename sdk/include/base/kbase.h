/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		kbase.h
	Author:		Eric Bryant

	Base types of the katana engine.
*/

#ifndef _KBASE_H
#define _KBASE_H

namespace Katana
{

///
/// AppHandle
/// Defines the handle to a created application. Similar to Win32 HINSTANCE.
///
typedef void * AppHandle;

///
/// WindowHandle
/// Defines the handle to a created window. Similar to Win32 HWND.
///
typedef void * WindowHandle;

///
/// DialogHandle
/// Defines the hanlde to a created dialog. Similar to Win32 DLGTEMPLATE
///
typedef void * DialogHandle;

///
/// FileHandle
/// Defines the handle to a file. Similar to the Win32 FILE *
///
typedef void * FileHandle;

///
/// ScriptEnv
/// The scripting environment's variable
///
typedef void * ScriptEnv;

}; // Katana

#endif _KBASE_H