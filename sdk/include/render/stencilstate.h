/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		stencilstate.h
	Author:		Eric Bryant

	Sets the stencil buffer parameters within the renderer
*/

#ifndef _STENCILSTATE_H
#define _STENCILSTATE_H

namespace Katana
{

///
/// StencilOp
/// Stencil buffer operations
///
enum StencilOp
{
	STENCIL_OP_KEEP		= 1,
	STENCIL_OP_ZERO		= 2,
	STENCIL_OP_REPLACE	= 3,
	STENCIL_OP_INCRSAT	= 4,
	STENCIL_OP_DECRSAT	= 5,
	STENCIL_OP_INVERT	= 6,
	STENCIL_OP_INCR		= 7,
	STENCIL_OP_DECR		= 8,
};

///
/// StencilCompare
/// Stencil comparision function for stencil tests
///
enum StencilCompare
{
	STENCIL_COMPARE_NEVER = 1,
	STENCIL_COMPARE_LESS = 2,
	STENCIL_COMPARE_EQUAL = 3,
	STENCIL_COMPARE_LESSEQUAL = 4,
	STENCIL_COMPARE_GREATER = 5,
	STENCIL_COMPARE_NOTEQUAL = 6,
	STENCIL_COMPARE_GREATEREQUAL = 7,
	STENCIL_COMPARE_ALWAYS = 8,
};


///
/// StencilState
/// Use this RenderState to set the renderer's stencil buffer
///
class StencilState : public RenderState
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( StencilState );

public:
	/// Constructor (disables the stencil buffer)
	StencilState();

	/// Constructor which enables it with this reference value
	StencilState( int ref );

	/// Constructor which enables it with this reference value and pass function
	StencilState( int ref, StencilOp passOp );

	/// Constructor which enables it with this reference value and stencil function
	StencilState( int ref, StencilCompare compareFn );

	/// Constructor which enables it with this reference value, comparison function and
	/// stencils operations for pass/fail
	StencilState( int ref, StencilCompare compareFn, StencilOp passOp, StencilOp failOp );

	/// Constructor which enables it 

	/// Sets the stencil enabled flag
	void setStencilEnabled( bool enabled )			{ m_stencilEnabled = enabled; }

	/// Gets whether the stencil is enabled
	bool getStencilEnabled() const					{ return m_stencilEnabled; }

	/// Sets the stencil comparison function
	void setCompareFunction( StencilCompare func )	{ m_stencilFunc = func; }

	/// Gets the stencil comparison function
	StencilCompare getCompareFunction() const		{ return m_stencilFunc; }

	/// Sets the stencil pass operation
	void setPassOp( StencilOp op )					{ m_stencilPassOp = op; }

	/// Gets the stencil pass operation
	StencilOp getPassOp() const						{ return m_stencilPassOp; }

	/// Sets the stencil fail operation
	void setFailOp( StencilOp op )					{ m_stencilFailOp = op; }

	/// Gets the stencil fail operation
	StencilOp getFailOp() const						{ return m_stencilFailOp; }

	/// Sets the z fail operation
	void setZFailOp( StencilOp op )					{ m_zFailOp = op; }

	/// Gets the z fail operation
	StencilOp getZFailOp() const					{ return m_zFailOp; }

	/// Sets the stencil reference value
	void setStencilRef( int ref )					{ m_ref = ref; }

	/// Gets the stencil reference value
	int getStencilRef() const						{ return m_ref; }

	/// Sets the stencil mask
	void setStencilMask( unsigned long mask )		{ m_stencilMask = mask; }

	/// Gets the stencil mask
	unsigned long getStencilMask() const			{ return m_stencilMask; }

	/// Sets the stencil write mask
	void setStencilWriteMask( unsigned long mask )	{ m_writeMask = mask; }

	/// Gets the stencil write mask
	unsigned long getStencilWriteMask() const		{ return m_writeMask; }


private:
	
	/// Flags whether the stencil buffer is enabled
	bool			m_stencilEnabled;

	/// Stencil comparison function. Default is COMPARE_ALWAYS
	StencilCompare	m_stencilFunc;

	/// Stencil operation to perform if the stencil test passes
	/// and the depth test fails. Default is STENCIL_OP_KEEP
	StencilOp		m_zFailOp;

	/// Stencil operation to perform if the stencil test fails.
	/// The default is STENCIL_OP_KEEP.
	StencilOp		m_stencilFailOp;

	/// Stencil operation to perform if the stencil test passes.
	/// The default is STENCIL_OP_KEEP.
	StencilOp		m_stencilPassOp;

	/// The reference value for the stencil test. The default is 0.
	int				m_ref;

	/// Mask applied to the reference value and each stencil buffer entry
	/// to determine the significant bits for the stencil test.
	/// Default is 0xFFFFFFFF.
	unsigned long	m_stencilMask;

	/// Write mask applied to values written into the stencil buffer. 
	/// The default mask is 0xFFFFFFFF.
	unsigned long	m_writeMask;


};

KIMPLEMENT_STREAM( StencilState );

//
// Inline
//

//
// StencilState::constructors
//
inline StencilState::StencilState()
	: m_stencilEnabled( false )
	, m_stencilFunc( STENCIL_COMPARE_ALWAYS )
	, m_zFailOp( STENCIL_OP_KEEP )
	, m_stencilFailOp( STENCIL_OP_KEEP )
	, m_stencilPassOp( STENCIL_OP_KEEP )
	, m_ref( 0 )
	, m_stencilMask( 0xFFFFFFFF )
	, m_writeMask( 0xFFFFFFFF )
{}

inline StencilState::StencilState( int ref )
	: m_stencilEnabled( true )
	, m_stencilFunc( STENCIL_COMPARE_ALWAYS )
	, m_zFailOp( STENCIL_OP_KEEP )
	, m_stencilFailOp( STENCIL_OP_KEEP )
	, m_stencilPassOp( STENCIL_OP_KEEP )
	, m_ref( ref )
	, m_stencilMask( 0xFFFFFFFF )
	, m_writeMask( 0xFFFFFFFF )
{}

inline StencilState::StencilState( int ref, StencilOp passOp )
	: m_stencilEnabled( true )
	, m_stencilFunc( STENCIL_COMPARE_ALWAYS )
	, m_zFailOp( STENCIL_OP_KEEP )
	, m_stencilFailOp( STENCIL_OP_KEEP )
	, m_stencilPassOp( passOp )
	, m_ref( ref )
	, m_stencilMask( 0xFFFFFFFF )
	, m_writeMask( 0xFFFFFFFF )
{}

inline StencilState::StencilState( int ref, StencilCompare compareFn )
	: m_stencilEnabled( true )
	, m_stencilFunc( compareFn )
	, m_zFailOp( STENCIL_OP_KEEP )
	, m_stencilFailOp( STENCIL_OP_KEEP )
	, m_stencilPassOp( STENCIL_OP_KEEP )
	, m_ref( ref )
	, m_stencilMask( 0xFFFFFFFF )
	, m_writeMask( 0xFFFFFFFF )
{}

inline StencilState::StencilState( int ref, StencilCompare compareFn, StencilOp passOp, StencilOp failOp )
	: m_stencilEnabled( true )
	, m_stencilFunc( compareFn )
	, m_zFailOp( passOp )
	, m_stencilFailOp( failOp )
	, m_stencilPassOp( STENCIL_OP_KEEP )
	, m_ref( ref )
	, m_stencilMask( 0xFFFFFFFF )
	, m_writeMask( 0xFFFFFFFF )
{}

} // Katana

#endif // _STENCILSTATE_H