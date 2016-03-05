/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		log.h
	Author:		Eric Bryant

	Log File.
*/

#ifndef _LOG_H
#define _LOG_H

namespace Katana
{

// Forward Declarations
class SystemFile;

/// Log
/// Encapsulates log file
class Log
{
private:
	/// The System File
	shared_ptr<SystemFile> m_spLogFile;

	/// If verbose is TRUE, we include debug commands.
	bool m_verbose;

public:
	/// Constructs a log file
	Log();
	Log(const char * szFileName, bool bVerboseLogging = false);

	/// Destructor
	~Log();

	/// Opens a log file
	bool Open(const char * szLogPath);

	/// Closes a log file
	bool Close();

	/// Returns the file name
	const char * GetLogPath() const;

	/// Sets the verbose settings. If it's TRUE, we include debug commands
	/// otherwise, they are ignored (which is the default).
	void SetVerbose(bool bOn);

	/// Retrieves the verbose mode
	bool GetVerbose() const;

	/// This stream operator takes a function pointer as a parameter.
	/// This is to simulate iostream::end functionality, functions
	/// are allowed to do operations on the log (see Log::endl)
	Log & operator<<(Log & (*f)(Log &)) { (*f)(*this); return *this; }

	/// Log Accessors
	bool Format(const char * szLine, ...);
	bool Debug(const char * szLine, ...);
	bool Debug2(const char * szLine, ...);	// Doesn't have a Default Indentation
	bool Error(const char * szLine, ...);
	bool Warning(const char * szLine, ...);
	bool Entry(const char * szLine = 0, bool bNewLine = true, bool bToConsole = true);
	bool Indent(const char * szLine, int nIndent = 0, bool bNewLine = true, bool bToConsole = true);
	bool Header(const char * szLine);
	bool Date();
	bool NewLine();

	/// Streaming Overloads
	Log & operator<<(float f);
	Log & operator<<(const char * sz);
	Log & operator<<(string & s);

private:
	/// Creates the initial header (for Open())
	void CreateHeader();

	/// Creates the closing foot (for Close())
	void CreateFooter();
};

/// defines endl
inline Log & endl(Log & log) { log.NewLine(); return log; }

/// function for logging
bool KLOG(const char * szLine, ...);
bool KLOG2(const char * szLine, ...); /// indents the log
bool KERROR(const char * szLine, ...);
bool KPRINT( const char * szLine, ...);

}; // Katana

#endif // _LOG_H