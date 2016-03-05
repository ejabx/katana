/*
	Katana Engine
	Copyright © 2001-2003 Eric Bryant, Inc.

	File:		console.h
	Author:		Eric Bryant

	Console window
*/

#ifndef _CONSOLE_H
#define _CONSOLE_H

namespace Katana
{

// Forward declarations
class kstring;
class SystemConsole;

///
/// Console
///
class Console
{
	KDECLARE_SCRIPT;

public:
	/// Constructor
	Console();

	/// Destructor
	~Console();

	/// Opens the console. If it hasn't been created, it will create a new
	/// console window. Otherwise, it will be unhidden.
	bool openConsole();

	/// Closes the console. It will hide the console. The window isn't destroyed
	/// until the destructor
	bool closeConsole();

	/// Queries whether the console is visible
	bool isVisible() const											{ return m_bIsConsoleVisible; }

	/// Stream operator which route the output to the console output window
	Console & operator<<( const char * szMessage );
	Console & operator<<( const string & kstr );

private:
	/// Pointer to the console window. It will be created on openConsole, and
	/// destroyed during the destructor.
	SystemConsole *	m_pConsole;

	/// Flag to indicate whether the console is visible
	bool					m_bIsConsoleVisible;
};

KIMPLEMENT_SCRIPT( Console );

}; // Katana

#endif // _CONSOLE_H