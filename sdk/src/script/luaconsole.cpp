/*
	Katana Engine
	Copyright © 2003 Eric Bryant, Inc.

	File:		luaconsole.cpp
	Author:		Eric Bryant

	Console developer for lua using the standard i/o
*/

#include <signal.h>

#include "katana_core_includes.h"
#include "luahelper.h"
#include "luaconsole.h"
#include "base/refcount.h"
#include "base/log.h"

//-----------------------------------------------------------
// Constants
//-----------------------------------------------------------

const char * PROMPT		= "> ";
const char * PROMPT2	= ">> ";

//-----------------------------------------------------------
// Local Functions
//-----------------------------------------------------------

int luaplus_loadstring (void);
int luaplus_incomplete (int status);
void luaplus_lstop (lua_State *l, lua_Debug *ar);
void luaplus_laction (int i);
int luaplus_lcall (int narg, int clear);
int luaplus_readline (lua_State *l, const char *prompt);
void luaplus_saveline( lua_State *l, const char * line );
int luaplus_report (int status);
const char * luaplus_getprompt (int firstline);

//-----------------------------------------------------------
// LuaConsole
//-----------------------------------------------------------

//
// Static Variables
//
std::string LuaConsole::LAST_SCRIPT_COMMAND;

//
// ProcessCommands
//
bool LuaConsole::ProcessCommand()
{
	int status;

	status = luaplus_loadstring();
	if (status == 0) status = luaplus_lcall(0, 0);
	luaplus_report(status);
	if (status == 0 && lua_gettop( GLOBAL_LUA_STATE ) > 0) // Any result to print?
	{
		lua_getglobal( GLOBAL_LUA_STATE, "print" );
		lua_insert( GLOBAL_LUA_STATE, 1 );
		if ( lua_pcall( GLOBAL_LUA_STATE, lua_gettop( GLOBAL_LUA_STATE )-1, 0, 0 ) != 0 )
			KERROR( lua_pushfstring( GLOBAL_LUA_STATE, "error calling `print' (%s)", lua_tostring( GLOBAL_LUA_STATE,-1 ) ) );
	}

	lua_settop( GLOBAL_LUA_STATE, 0 );  // clear stack

	return true;
}

//-----------------------------------------------------------
// Implementation of Local Functions
//-----------------------------------------------------------

int luaplus_loadstring (void)
{
	int status;
	lua_settop( GLOBAL_LUA_STATE, 0 );

	if ( luaplus_readline( GLOBAL_LUA_STATE, luaplus_getprompt(1) ) == 0 )  // no input?
		return -1;

	if ( lua_tostring( GLOBAL_LUA_STATE, -1)[0] == '=' ) {  // line starts with `=' ?
		lua_pushfstring( GLOBAL_LUA_STATE, "return %s", lua_tostring( GLOBAL_LUA_STATE, -1)+1); // `=' -> `return'
		lua_remove( GLOBAL_LUA_STATE, -2);  // remove original line
	}

	if ( strnicmp( lua_tostring( GLOBAL_LUA_STATE, -1 ), "exit", 4 ) == 0 ) // Check for exit
	{
		luaplus_saveline( GLOBAL_LUA_STATE, "exit" );
		return -1;
	}


	for (;;) {  // repeat until gets a complete line
		status = luaL_loadbuffer( GLOBAL_LUA_STATE, lua_tostring( GLOBAL_LUA_STATE, 1 ), lua_strlen( GLOBAL_LUA_STATE, 1 ), "=stdin" );
		if ( !luaplus_incomplete( status ) ) break;  // cannot try to add lines?
		if ( luaplus_readline( GLOBAL_LUA_STATE, luaplus_getprompt( 0 ) ) == 0 )  // no more input?
			return -1;
		lua_concat( GLOBAL_LUA_STATE, lua_gettop( GLOBAL_LUA_STATE ) );  // join lines
	}

	// Save the command line for a "history" system
	luaplus_saveline( GLOBAL_LUA_STATE, lua_tostring( GLOBAL_LUA_STATE, 1 ) );
	lua_remove( GLOBAL_LUA_STATE, 1 );  // remove line

	return status;
}

int luaplus_incomplete (int status)
{
	if (status == LUA_ERRSYNTAX &&
		strstr(lua_tostring( GLOBAL_LUA_STATE, -1), "near `<eof>'") != NULL) 
	{
			lua_pop( GLOBAL_LUA_STATE, 1);
			return 1;
	}
	else
	{
		return 0;
	}
}

void luaplus_lstop (lua_State *l, lua_Debug *ar)
{
	(void)ar;  // unused arg.

	lua_sethook( GLOBAL_LUA_STATE, NULL, 0, 0 );
	luaL_error( GLOBAL_LUA_STATE, "interrupted!" );
}

void luaplus_laction (int i)
{
	signal(i, SIG_DFL); // if another SIGINT happens before luaplus_lstop,
						// terminate process (default action)

	lua_sethook( GLOBAL_LUA_STATE, luaplus_lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

int luaplus_lcall (int narg, int clear)
{
	int status;
	int base = lua_gettop( GLOBAL_LUA_STATE ) - narg;  // function index
	lua_pushliteral( GLOBAL_LUA_STATE , "_TRACEBACK" );
	lua_rawget( GLOBAL_LUA_STATE, LUA_GLOBALSINDEX );  // get traceback function
	lua_insert( GLOBAL_LUA_STATE, base );  // put it under chunk and args
	signal( SIGINT, luaplus_laction );
	status = lua_pcall( GLOBAL_LUA_STATE, narg, (clear ? 0 : LUA_MULTRET), base );
	signal(SIGINT, SIG_DFL);
	lua_remove( GLOBAL_LUA_STATE , base);  // remove traceback function

	return status;
}

int luaplus_readline (lua_State *l, const char *prompt)
{
	static char buffer[1024];
	if ( prompt ) 
	{
		fputs(prompt, stdout);
		fflush(stdout);
	}
	if ( fgets( buffer, sizeof( buffer ), stdin ) == NULL )
	{
		return 0;  // read fails
	}
	else 
	{
		lua_pushstring( GLOBAL_LUA_STATE, buffer );
		return 1;
	}
}

void luaplus_saveline( lua_State *l, const char * line )
{
	LuaConsole::LAST_SCRIPT_COMMAND = line;
}

int luaplus_report (int status)
{
	const char *msg;
	if (status) 
	{
		msg = lua_tostring( GLOBAL_LUA_STATE , -1);
		if (msg == NULL) msg = "(error with no message)";
		KERROR( msg );
		lua_pop( GLOBAL_LUA_STATE , 1);
	}
	return status;
}

const char * luaplus_getprompt (int firstline)
{
	const char *p = NULL;
	lua_pushstring( GLOBAL_LUA_STATE , firstline ? "_PROMPT" : "_PROMPT2");
	lua_rawget( GLOBAL_LUA_STATE , LUA_GLOBALSINDEX);
	p = lua_tostring( GLOBAL_LUA_STATE , -1);
	if (p == NULL) p = (firstline ? PROMPT : PROMPT2);
	lua_pop( GLOBAL_LUA_STATE , 1);  /* remove global */

	return p;
}