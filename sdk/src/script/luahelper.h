/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		luahelper.h
	Author:		Eric Bryant

	Helper macros for lua
*/

#ifndef _LUAHELPER_H
#define _LUAHELPER_H

///
/// REGISTER_SCRIPTING_GUARD
/// Place this macro before defining a class registration function
/// in order to cast the lua state to a valid pointer and to make
/// sure the class isn't registered more than once.
///
#define REGISTER_SCRIPTING_GUARD()						\
	using namespace luabind;							\
	static bool bRegisterAlready = false;				\
	if ( bRegisterAlready ) return false;				\
	bRegisterAlready = true;							\

///
/// HOLDER TYPE SUPPORT FOR SHARED_PTR (from Luabind docs):
/// "You also have to supply two functions for your smart pointer. One that returns the type 
/// of const version of the smart pointer type (boost::shared_ptr<const A> in this case).
/// And one function that extracts the raw pointer from the smart pointer. The first function is 
/// needed because luabind has to allow the non-const -> conversion when passing values from lua to C++.
/// The second function is needed when lua calls member functions on held types, the this pointer must be
/// a raw pointer, it is also needed to allow the smart_pointer -> raw_pointer conversion from lua to C++.
///
namespace luabind
{
	template<class T>
	T * get_pointer(boost::shared_ptr<T>& p) { return p.get(); }

	template<class A>
	boost::shared_ptr<const A>* get_const_holder(boost::shared_ptr<A>*)
	{
		return 0;
	}
}

namespace Katana
{
	/// This is the global lua_State created by the Scripting Engine
	extern lua_State * GLOBAL_LUA_STATE;
}

#endif // _LUAHELPER_H