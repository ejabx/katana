/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		luaerr.h
	Author:		Eric Bryant

	Helper function to return lua errors
*/

#ifndef _LUAERR_H
#define _LUAERR_H

//
// Forward Declarations
// 
struct lua_State;

namespace Katana
{

///
/// LuaErr
/// When within a lua function, use this function to pass an error value
/// to lua. It automatically returns 0 (because nothing will be added to the
/// stack).
///
/// Sample Usage:
///		if ( [some check failed] )
///			return LuaErr( "This function failed because check %d was invalid", -1 );

int LuaErr( const char * format, ... );



/// Use this macro for simple errors, it will store the file name and line number
/// for the error. However, it does not support variable arguments, you must
/// use the LuaErr function directly.
#ifdef _MSC_VER
	#define LUA_ERROR( err )									\
		LuaErr( "%s (%d): %s\n", __FUNCTION__, __LINE__, err );
#else
	#define LUA_ERROR( err )									\
		LuaErr( "%s (%d): %s\n", __FILE__, __LINE__, err );
#endif 

/// Use this macro to return a successful lua function
#define LUA_SUCCESS	0;



/// Because the LuaErr used variable arguments, this is the maximum size of
/// the resultant character string. Use this variable to change the default
/// (which is 2048).
extern int			MAX_LUA_ERROR_STRING;


}; // Katana

#endif // _LUAERR_H