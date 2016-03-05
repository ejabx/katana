/*
	Katana Engine
	Copyright © 2003 Eric Bryant, Inc.

	File:		luaconsole.h
	Author:		Eric Bryant
	Credits:	The lua console code was taken from LuaPlus:
				http://wwhiz.com/LuaPlus/

	Console developer for lua using the standard i/o
*/

#ifndef LUACONSOLE_H
#define LUACONSOLE_H

//
// Forward Declarations
//
struct lua_State;

namespace Katana
{

///
/// LuaConsole
///
class LuaConsole
{
public:
	/// Uses lua standard i/o to process commands until the exit command is given
	static bool ProcessCommand();

public:
	/// The last command executed by the console
	static std::string LAST_SCRIPT_COMMAND;
};

} // Katana

#endif // LUACONSOLE_H