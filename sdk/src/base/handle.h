/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		handle.h
	Author:		Eric Bryant

	Union to handle all different types of Win32 Handles
*/

#ifndef _HANDLE_H
#define _HANDLE_H

namespace Katana
{

///
/// Handle
/// Union to handle all different types of Win32 Handles
class Handle
{
public:
	/// Constructor
	Handle() { 
		_l = 0;		// Should be largest variable
		_isNull = true;
	};

	/// Copy Constructors
	Handle(long l)				{ _l = l; _isNull = false; }
	Handle(void * pv)			{ _pv = pv; _isNull = false; }
	Handle(HWND hWnd)			{ _hWnd = hWnd; _isNull = false; }

	/// Operator equal overloads
	void operator=(long l)				{ _l = l; _isNull = false; }
	void operator=(void * pv)			{ _pv = pv; _isNull = false; }
	void operator=(HWND hWnd)			{ _hWnd = hWnd; _isNull = false; }

	/// For Conditional Statements
	operator bool()			{ return _l ? true : false; }

	/// Explicit Casts
	operator long()			{ return _l; }
	operator void*()		{ return _pv; }
	operator HWND()			{ return _hWnd; }

	/// Check if the value is non-zero
	bool isValid()			{ return !_isNull && _l != 0; }

private:
	union
	{
		long		_l;
		HWND		_hWnd;
		void *		_pv;
	};

	bool		_isNull;
};

};

#endif // _HANDLE_H
