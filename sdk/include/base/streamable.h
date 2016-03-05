/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		streamable.h
	Author:		Eric Bryant

	Interface for classes which can store and retreive itself
	from katana streams.
*/

#ifndef _STREAMABLE_H
#define _STREAMABLE_H

namespace Katana
{

//
// Forward Declarations
//
class kistream;
class kostream;

///
/// Streamable
/// Base class for all streamable objects.
///
class Streamable : public RTTI
{
	KDECLARE_RTTI;

public:
	/// Callback for when the object must load itself from a stream
	virtual bool OnLoadStream( kistream & istr )=0;

	/// Callback for when the object must save itself to a stream
	virtual bool OnSaveStream( kostream & ostr ) const=0;

	/// This virtual constructor is used by the streaming library during loading
	/// of streams. Given an object name, it will create the object instance
	static Streamable * Construct( const char * objectType );

private:

	/// Functions type for the creation methods
	typedef Streamable * (CreationMethod)();

	/// This hash table maps is used for virtual construction. It maps the RTTI name
	/// with the corresponding creation method. Note for this to work, the Streamable
	/// class must have a default constructor
	static std::map< kstring, CreationMethod * > * m_vcstrTable;

public:

	/// Adds a Streamable's object's creation method to the internal static table.
	/// Client do not explicitly call this method, it's called during the 
	/// KIMPLEMENT_STREAM() Macro.
	static bool Register( const char * objectType, CreationMethod * method );
};




/// Place this macro in the class definition to support Streaming
#define KDECLARE_STREAM(classname) \
protected:														\
	virtual bool OnLoadStream( kistream & istr );				\
	virtual bool OnSaveStream( kostream & ostr ) const;			\
	static const bool bRetValue;								\
public:															\
	static Streamable * __vcstr()								\
	{															\
		return new classname;									\
	}															\

};

/// Place this macro in the class source to support Streaming
#define KIMPLEMENT_STREAM(classname) \
		static bool classname##bRetValue = Streamable::Register(#classname, &classname::__vcstr);	\

#endif // _STREAMABLE_H