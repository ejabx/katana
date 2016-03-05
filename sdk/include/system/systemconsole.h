/*
	Katana Engine
	Copyright © 2003 Eric Bryant, Inc.

	File:		systemconsole.h
	Author:		Eric Bryant

	System Console
*/

#ifndef _SYSTEMCONSOLE_H
#define _SYSTEMCONSOLE_H

namespace Katana
{

///
/// SystemConsole
///
class SystemConsole
{
public:
	/// Constructor
	SystemConsole();

	/// Destructor
	~SystemConsole();

	/// Adds a string to the console
	void addString( const char * szMessage );

	/// Show the console
	bool showConsole();

	/// Hides the console
	bool hideConsole();

private:
	unsigned long	m_consoleThread;	/// The console is implemented in a separate
										/// thread which handles input.
};

} // Katana

#endif // _SYSTEMCONSOLE_H