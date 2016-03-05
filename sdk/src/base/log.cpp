/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		log.cpp
	Author:		Eric Bryant

	Log File.
*/

#include "version.h"
#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "engine/console.h"
#include "base/kstring.h"

//
// Maximum buffer size for a line
//
const int MAX_LINE_SIZE = 2048;

//
// External variables
//
extern shared_ptr<Console> katana_console;

//
// Local Functions
//
bool WriteToOutput( const char * szOutput );

//
// Constructor
//
Log::Log() :
	m_verbose(false)
{
}

Log::Log(const char * szLogPath, bool bVerboseLogging) :
	m_verbose(bVerboseLogging)
{
	Open(szLogPath);
}

//
// Destructor
//
Log::~Log()
{
	Close();
}

//
// Open
//
bool Log::Open(const char * szLogPath)
{
	// Creates a new Log File
//	m_spLogFile = new KFile(LogPath, KFile::TEXT_FILE, KFile::BUFFER_IMMEDIATE_FLUSH);

	// Is the file valid?
	if ( !m_spLogFile )
		return false;

	// Creates a header for the file
	CreateHeader();

	return true;
}

//
// Close
//
bool Log::Close()
{
	// Assertations
//	assert(m_spLogFile);

	if ( !m_spLogFile )
		return false;

	// Creates a footer for the file
	CreateFooter();

//	return m_spLogFile->CloseFile();
	return true;
}

//
// GetLogPath
//
const char * Log::GetLogPath() const
{
//	return (const char *)m_kstrLogPath;
	return 0;
}

// 
// SetVerbose
//
void Log::SetVerbose(bool bOn)
{
	m_verbose = bOn;

	// If the Log File is Already Open, Log this Action
//	if ( m_spLogFile && m_spLogFile->IsOpened() )
//		Format("Verbose Log: %s\n", m_bVerbose ? "On":"Off");
}

//
// GetVerbose
//
bool Log::GetVerbose() const
{
	return m_verbose;
}

//
// Format
//
bool Log::Format(const char * szLine, ...)
{
	// Assertations
//	assert(m_spLogFile);

	if ( !m_spLogFile )
		return false;
	
	// Compose the variable arguments into a single string
	va_list l_va;
	char szBuffer[MAX_LINE_SIZE];

	va_start(l_va, szLine);
	vsprintf(szBuffer, szLine, l_va);
	va_end(l_va);

	// Store the string in the log file (with CR at the end)
	return Indent(szBuffer);
}

//
// Entry
//
bool Log::Entry(const char * szLine, bool bNewLine, bool bToConsole)
{
	// Assertations
//	assert(m_spLogFile);

	if ( !m_spLogFile )
		return false;

	// Writes the string to the file
//	if ( NULL != szLine )
//		m_spLogFile->WriteString(szLine);

	//  a carriage return if necessary
//	if ( true == bNewLine )
//		m_spLogFile->WriteString("\n");

	// Mirror the Output to the Console
	if ( true == bToConsole )
	{
		// Output the Line to the Console/STDERR
		if ( szLine != NULL )
			WriteToOutput( szLine );

		// Newline if necessary
		if ( true == bNewLine )
			WriteToOutput( "\r\n" );
	}

	return true;
}

//
// Indent
//
bool Log::Indent(const char * szLine, int nIndent, bool bNewLine, bool bToConsole)
{
	// Assertations
//	assert(m_spLogFile);

	if ( !m_spLogFile )
		return false;

	// Default Indentation is "-- "
	string strIndented("-- ");

	// Add additional indents
	for(int i = 0; i < nIndent; i++)
		strIndented.append("  ");

	// Finally append our new line
	strIndented.append(szLine);

	// Store the Entry
	return Entry(strIndented.c_str(), bNewLine, bToConsole);
}

//
// Header
//
bool Log::Header(const char * szLine)
{
	// Assertations
//	assert(m_spLogFile);

	if ( !m_spLogFile )
		return false;

	// Default Indentation is "-- "
	string strHeader("-- ");
	
	// Append our line
	strHeader.append(szLine);

	// Close off the header with " --";
	strHeader.append(" --");

	return Entry(strHeader.c_str(), true);
}

//
// Debug
//
bool Log::Debug(const char * szLine, ...)
{
	// Assertations
//	assert(m_spLogFile);

	if ( !m_spLogFile )
		return false;

	// Compose the variable arguments into a single string
	va_list l_va;
	char szBuffer[MAX_LINE_SIZE];

	va_start(l_va, szLine);
	vsprintf(szBuffer, szLine, l_va);
	va_end(l_va);

	// Default Indentation is ">> "
	string strError(">> ");
	
	// Append our preprocessed buffer
	strError.append(szBuffer);

	// Output the buffer to the MSVC Output Console
//	KOutputDebugString(szBuffer);
//	KOutputDebugString("\n");

	return Entry(strError.c_str(), true);
}

//
// Debug2
//
bool Log::Debug2(const char * szLine, ...)
{
	// Assertations
//	assert(m_spLogFile);

	if ( !m_spLogFile )
		return false;

	// If Verbose Logging Is Disabled, Ignore this Directive
	if ( false == m_verbose )
		return false;

	// Compose the variable arguments into a single string
	va_list l_va;
	char szBuffer[MAX_LINE_SIZE];

	va_start(l_va, szLine);
	vsprintf(szBuffer, szLine, l_va);
	va_end(l_va);

	// Output the buffer to the MSVC Output Console
//	KOutputDebugString(szBuffer);
//	KOutputDebugString("\n");

	return Entry(szBuffer, true);
}

//
// Error
//
bool Log::Error(const char * szLine, ...)
{
	// Assertations
//	assert(m_spLogFile);

	if ( !m_spLogFile )
		return false;

	// Compose the variable arguments into a single string
	va_list l_va;
	char szBuffer[MAX_LINE_SIZE];

	va_start(l_va, szLine);
	vsprintf(szBuffer, szLine, l_va);
	va_end(l_va);

	// Default Indentation is "** ERROR: "
	string strError("** ERROR: ");
	
	// Append our preprocessed buffer
	strError.append(szBuffer);

	return Entry(strError.c_str(), true);
}

//
// Warning
//
bool Log::Warning(const char * szLine, ...)
{
	// Assertations
//	assert(m_spLogFile);

	if ( !m_spLogFile )
		return false;

	// Compose the variable arguments into a single string
	va_list l_va;
	char szBuffer[MAX_LINE_SIZE];

	va_start(l_va, szLine);
	vsprintf(szBuffer, szLine, l_va);
	va_end(l_va);

	// Default Indentation is "** ERROR: "
	string strWarning("** WARNING: ");
	
	// Append our preprocessed buffer
	strWarning.append(szBuffer);

	return Entry(strWarning.c_str(), true);
}

//
// Date
//
bool Log::Date()
{
	// Assertations
//	assert(m_spLogFile);

	if ( !m_spLogFile )
		return false;

	// Time Class (the default is the current time)
//	KTime localTime;

	// Looks familiar, eh? (Java)
//	return Entry(localTime.ToString(), false);	
	return true;
}

//
// NewLine
//
bool Log::NewLine()
{
	return Entry();	// The default is a new line
}

//
// Operator overloads
//
Log & Log::operator<<(float f)
{
	Format("%d", f);
	return *this;
}

Log & Log::operator<<(const char * sz)
{
	Entry(sz, false);
	return *this;
}

Log & Log::operator<<(string & s)
{
	Entry(s.c_str(), false);
	return *this;
}

//
// CreateHeader
//
void Log::CreateHeader()
{
//	KTime localTime;
	string strPlatform;
	string strBuildNumber;
	string strDebugRelease;
	kstring strLocalTime;

#ifdef _DEBUG
	strDebugRelease = "Debug";
#else
	strDebugRelease = "Release";
#endif

#ifdef _WIN32
	strPlatform = "Win32";
#endif
#ifdef _XBOX
	strPlatform = "XBox";
#endif

	// Appends the Month/Day, for instance October 21 will have a build number of "1021"
//	strBuildNumber.Append(localTime.GetMonth());
//	strBuildNumber.Append(localTime.GetDay());

	// Truncate the last \n\0 from the LocalTime (a side effect of ::asctime());
//	strLocalTime.Append(localTime.ToString());
	strLocalTime.strip("\n\0");

	// Engine Version
	Format("Katana Engine v%s (build %s) - %s %s - %s",
			Katana::BUILD_VERSION,			
			strBuildNumber.c_str(),
			strPlatform.c_str(),
			strDebugRelease.c_str(),
			strLocalTime.c_str());

	// File Version
	Format("File Version: v%s", Katana::FILE_VERSION);

	// Verbose Setting
	Format("Verbose Log: %s", m_verbose ? "On":"Off");
}

//
// CreateFooter
//
void Log::CreateFooter()
{
	NewLine();
	Entry("Log Closed - ", false);
	Date();
}

//
// KLOG
//
bool Katana::KLOG(const char * szLine, ...)
{
	// Compose the variable arguments into a single string
	va_list l_va;
	char szBuffer[MAX_LINE_SIZE];

	va_start(l_va, szLine);
	vsprintf(szBuffer, szLine, l_va);
	va_end(l_va);

	return WriteToOutput( "-- " ) && WriteToOutput( szBuffer ) && WriteToOutput( "\r\n" );
}

//
// KLOG2
//
bool Katana::KLOG2(const char * szLine, ...)
{
	// Compose the variable arguments into a single string
	va_list l_va;
	char szBuffer[MAX_LINE_SIZE];

	va_start(l_va, szLine);
	vsprintf(szBuffer, szLine, l_va);
	va_end(l_va);

	return WriteToOutput( "--   " ) && WriteToOutput( szBuffer ) && WriteToOutput( "\r\n" );
}

//
// KError
//
bool Katana::KERROR(const char * szLine, ...)
{
	// Compose the variable arguments into a single string
	va_list l_va;
	char szBuffer[MAX_LINE_SIZE];

	va_start(l_va, szLine);
	vsprintf(szBuffer, szLine, l_va);
	va_end(l_va);

	return WriteToOutput( "** ERROR: " ) && WriteToOutput( szBuffer ) && WriteToOutput( "\r\n" );
}

//
// KPrint
//
bool Katana::KPRINT(const char * szLine, ...)
{
	// Compose the variable arguments into a single string
	va_list l_va;
	char szBuffer[MAX_LINE_SIZE];

	va_start(l_va, szLine);
	vsprintf(szBuffer, szLine, l_va);
	va_end(l_va);

	return WriteToOutput( szBuffer ) && WriteToOutput( "\r\n" );
}

//
// WriteToOutput
//
bool WriteToOutput( const char * szOutput )
{
	// Write the output to the console
	*katana_console.get() << szOutput;

	return true;
}