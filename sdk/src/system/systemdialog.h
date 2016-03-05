/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		systemdialog.h
	Author:		Eric Bryant

	System Dialog
*/

#ifndef _SYSTEMDIALOG_H
#define _SYSTEMDIALOG_H

namespace Katana
{

// Forward declarations
class SystemInfo;
class kexport;
struct ExportSettings;

///
/// SystemDialog
/// Displays a dialog 
///
class SystemDialog
{
public:
	/// Constructor
	SystemDialog( const char * szCaption, unsigned long style, 
				  int x, int y, int w, int h,
				  const char * szFont = 0, 
				  unsigned short fontSize = 8 );

	/// Destructor
    virtual ~SystemDialog();

	/// Functions to add the various dialog constructs
    void addComponent(const char * szType, const char * szCaption, unsigned long style, 
		unsigned long exStyle, int x, int y, int w, int h, unsigned short id);
    void addButton(const char * szCaption, unsigned long style, unsigned long exStyle,
		int x, int y, int w, int h, unsigned short id);
    void addEditBox(const char * szCaption, unsigned long style, unsigned long exStyle, 
		int x, int y, int w, int h, unsigned short id);
    void addStatic(const char * szCaption, unsigned long style, unsigned long exStyle, 
		int x, int y, int w, int h, unsigned short id);
    void addListBox(const char * szCaption, unsigned long style, unsigned long exStyle, 
		int x, int y, int w, int h, unsigned short id);
    void addScrollBar(const char * szCaption, unsigned long style, unsigned long exStyle, 
		int x, int y, int w, int h, unsigned short id);
    void addComboBox(const char * szCaption, unsigned long style, unsigned long exStyle, 
		int x, int y, int w, int h, unsigned short id);

	/// Displays this dialog. Returns true if the dialog was closed properly (via OK button),
	/// or false if it was cancelled.
	virtual bool showDialog( void * pfDialogProcedure = 0 );

protected:

	/// Helper Functions
    void AddStandardComponent(unsigned short type, const char * caption, unsigned long style,
							  unsigned long exStyle, int x, int y, int w, int h, unsigned short id);
    void AlignData(int size);
    void AppendString(const char * string);
    void AppendData(void* data, int dataLength);
    void EnsureSpace(int length);

protected:
    int				m_totalBufferLength;
    int				m_usedBufferLength;
	DialogHandle *	m_dialogTemplate;
	WindowHandle *	m_dialogWindow;
};

///
/// KatanaStartupDialog
/// Dialog which first displayed during katana startup.
/// It allows the user to set game resolution and startup script
///
class KatanaStartupDialog : public SystemDialog
{
public:
	/// Constructor
	KatanaStartupDialog( SystemInfo & sysInfo );

	/// Overridden show dialog
	virtual bool showDialog( void * pfDialogProcedure = 0 );
};

///
/// KatanaExportDialog
/// Dialog which displays export settings and uses the kexport
/// class to enumerate through an exported scene graph (from MAX,
/// Maya, etc.) and saves it to a file.
///
class KatanaExportDialog : public SystemDialog
{
public:
	/// Constructor which takes a kexport derived class which provides
	/// the methods to enumerate through the exported scene and serializes
	/// them to media.
	KatanaExportDialog( kexport & exportImpl, ExportSettings & exportSettings );

	/// Overridden show dialog
	virtual bool showDialog( void * pfDialogProcedure = 0 );
};

} // Katana


#endif // _SYSTEMDIALOG_H
