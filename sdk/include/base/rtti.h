/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		rtti.h
	Author:		Eric Bryant

	Supports RTTI Type information.
*/

#ifndef _RTTI_H
#define _RTTI_H



namespace Katana
{

///
/// RTTI_TYPE
/// Private class which supports Run-Time Type Information. Instead
/// of using strings, it's uses static pointer for RTTI comparisons.
/// The idea is derived from Dave Edberly's 3d Game Engine Design.
///
class RTTI_TYPE
{
public:
	/// Constructor
	RTTI_TYPE()											{}
	RTTI_TYPE(char* pName, const RTTI_TYPE* pBaseRTTI) { m_pName = pName; m_pBaseRTTI = pBaseRTTI; }

	/// Destructor
	virtual ~RTTI_TYPE () {}

	/// Get the RTTI Type Name
    const char* GetName() const					{ return m_pName; }	

	/// Get the Base class's RTTI
    const RTTI_TYPE* GetBaseRTTI() const		{ return m_pBaseRTTI; }

	/// Returns TRUE if a class is exactly a particular RTTI.
	bool	IsExactlyClass( const RTTI_TYPE* pkQueryRTTI ) const;

	/// Returns TRUE if a class is derived from a particular RTTI.
	bool	IsDerivedFromClass( const RTTI_TYPE* pkQueryRTTI ) const;

protected:
	/// Pointer to RTTI Name
    char* m_pName;

	/// Pointer to Base Class's RTTI
    const RTTI_TYPE* m_pBaseRTTI;
};



///
/// RTTI
/// Interface for RTTI classes. Any class which wishes to support RTTI
/// Information must derive from this class and use the KDECLARE_RTTI
/// macro in the class definition, and the KIMPLEMENT_RTTI macro in 
/// the source file.
///
class RTTI
{
public:
	virtual const RTTI_TYPE* GetRTTI() const = 0;
};



/// Place this macro in the class definition to support RTTI
#define KDECLARE_RTTI \
public:																			\
	static const RTTI_TYPE ms_kRTTI;											\
	virtual const RTTI_TYPE* GetRTTI() const { return &ms_kRTTI; }				\

/// Place this macro in the source of the base RTTI class
#define KIMPLEMENT_ROOT_RTTI( rootclassname ) \
	const RTTI_TYPE  rootclassname::ms_kRTTI( #rootclassname, 0 );

/// Allows you to specify a base RTTI name alias that is different from the class name
#define KIMPLEMENT_ROOT_RTTI_ALIAS( rootclassname, alias ) \
	const RTTI_TYPE  rootclassname::ms_kRTTI( #alias, 0 );

/// Place this macro in the source of the class to support RTTI
#define KIMPLEMENT_RTTI( classname,baseclassname ) \
	const RTTI_TYPE  classname::ms_kRTTI( #classname, &baseclassname::ms_kRTTI );

/// Allows you to specify a RTTI name alias that is different from the class name
#define KIMPLEMENT_RTTI_ALIAS( classname, baseclassname, alias ) \
	const RTTI_TYPE  classname::ms_kRTTI( #alias, &baseclassname::ms_kRTTI );

/// IsExactlyClass
inline bool RTTI_TYPE::IsExactlyClass( const RTTI_TYPE* pkQueryRTTI ) const
{
	return ( this == pkQueryRTTI );
}

// IsDerivedFromClass
inline bool RTTI_TYPE::IsDerivedFromClass( const RTTI_TYPE* pkQueryRTTI ) const
{
	// Keep diving down our hierarchy looking for a match
	const RTTI_TYPE* pkRTTI = this;
	while ( pkRTTI )
	{
		if ( pkRTTI == pkQueryRTTI )
			return true;
		pkRTTI = pkRTTI->GetBaseRTTI();
	}
	return false;
}

/// Checks if any object is exactly of class T
template <class T> inline bool KIsExactlyFromClass( const RTTI* pObject )
	{ return (pObject && pObject->GetRTTI()) ? pObject->GetRTTI()->IsExactlyClass(&T::ms_kRTTI) : false; }

/// Checks if any object is derived from class T
template <class T> inline bool KIsDerivedFromClass( const RTTI* pObject )
	{ return (pObject && pObject->GetRTTI()) ? pObject->GetRTTI()->IsDerivedFromClass(&T::ms_kRTTI) : false; }

/// Performs a dynamic cast to T* with RTTI verification.  Returns NULL if cast is invalid.
template <class T> inline T* KDynamicCast( RTTI* pObject )
	{ return KIsDerivedFromClass<T>(pObject) ? static_cast<T*>(pObject) : 0; }

/// Performs a dynamic cast to const T* with RTTI verification.  Returns NULL if cast is invalid.
template <class T> inline const T* KDynamicCast( const RTTI* pObject )
	{ return KIsDerivedFromClass<T>(pObject) ? static_cast<const T*>(pObject) : 0; }

}; // Katana

#endif // _RTTI_H