/*
	Katana Engine
	Copyright © 2001-2003 Eric Bryant, Inc.

	File:		console.h
	Author:		Eric Bryant

	Console window
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "application.h"
#include "console.h"
#include "system/systemconsole.h"

//
// Constructor
//
Console::Console() :
	m_pConsole ( NULL ),
	m_bIsConsoleVisible( false )
{
	m_pConsole = new SystemConsole();
}

//
// Destructor
//
Console::~Console()
{
	delete m_pConsole;
}

//
// openConsole
//
bool Console::openConsole()
{
	if ( m_pConsole )
		return ( m_bIsConsoleVisible = m_pConsole->showConsole() );

	return false;
}

//
// closeConsole
//
bool Console::closeConsole()
{
	if ( m_pConsole ) 
		return ( m_bIsConsoleVisible = !m_pConsole->hideConsole() );

	return false;
}

//
// operator<<
//
Console & Console::operator<<( const char * szMessage )
{
	if ( m_pConsole ) m_pConsole->addString( szMessage );
	return *this;
}

Console & Console::operator<<( const string & kstr )
{
	if ( m_pConsole ) m_pConsole->addString( kstr.c_str() );
	return *this;
}