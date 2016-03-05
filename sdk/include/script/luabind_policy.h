/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		luabind_policy.h
	Author:		Eric Bryant

	Additional luabind policies
*/

#ifndef _LUABIND_POLICY_H
#define _LUABIND_POLICY_H

#include <luabind/config.hpp>
#include <luabind/detail/policy.hpp>
#include <luabind/detail/implicit_cast.hpp>

///
/// shared_ptr_policy
///
/// Allows up/down-casts of boost::shared_ptr's. This is done by first 
/// casting the held pointer up to a base class, then dynamic_cast down
/// this pointer to the element_type of the shared_ptr and finally construct 
/// a new shared_ptr with the new raw pointer. To stop the shared_ptr from
/// deleting the held object, we supply a custom deleter functor that doesn't do anything.
///

namespace luabind { namespace detail 
{
	struct do_nothing_deleter
	{ 
		void operator()(const void*) const {} 
	};

	struct shared_ptr_converter
	{
		template<class T>
			boost::shared_ptr<T> apply(lua_State* L, by_value<boost::shared_ptr<T> >, int index)
		{
			object_rep* obj = static_cast<object_rep*>(lua_touserdata(L, index));
			assert((obj != 0) && "internal error, please report"); // internal error
			const class_rep* crep = obj->crep();

			T* ptr = static_cast<T*>(crep->convert_to(LUABIND_TYPEID(T), obj, 0));
			return boost::shared_ptr<T>(ptr, do_nothing_deleter());
		}

		template<class T>
			boost::shared_ptr<T> apply(lua_State* L, by_const_reference<boost::shared_ptr<T> >, int index)
		{ return apply(L, by_value<boost::shared_ptr<T> >(), index); }

		template<class T>
			static int match(lua_State* L, by_value<boost::shared_ptr<T> >, int index)
		{
			object_rep* obj = static_cast<object_rep*>(lua_touserdata(L, index));
			assert((obj != 0) && "internal error, please report"); // internal error
			const class_rep* crep = obj->crep();

			int d;
			return implicit_cast(obj->crep(), LUABIND_TYPEID(T), d);
		}

		template<class T>
			static int match(lua_State* L, by_const_reference<boost::shared_ptr<T> >, int index)
		{ return match(L, by_value<boost::shared_ptr<T> >(), index); }

		void converter_postcall(...) {}
		void converter_precall(...) {}
	};

	template<int N>
	struct shared_ptr_policy : conversion_policy<N>
	{
		static void precall(lua_State*, const index_map&) {}
		static void postcall(lua_State*, const index_map&) {}

		template<class T, class Direction>
		struct generate_converter
		{
			typedef shared_ptr_converter type;
		};
	};
}}

namespace luabind
{
	template<int N>
		detail::policy_cons<detail::shared_ptr_policy<N>, detail::null_type>
		shared_ptr_policy(boost::arg<N>) { return detail::policy_cons<detail::shared_ptr_policy<N>, detail::null_type>(); }
}

#endif // _LUABIND_POLICY_H