/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		zbufferstate.h
	Author:		Eric Bryant

	Sets the renderer into z buffering mode.
*/

#ifndef _ZBUFFERSTATE_H
#define _ZBUFFERSTATE_H

namespace Katana
{

///
/// CompareOp
/// The comparison operation for z buffer fragment rejection.
/// These directly correspond to D3DCMPFUNC.
enum CompareOp
{
	COMPARE_NEVER			= 1,
	COMPARE_LESS			= 2,
	COMPARE_EQUAL			= 3,
	COMPARE_LESSEQUAL		= 4,
	COMPARE_GREATER			= 5,
	COMPARE_NOTEQUAL		= 6,
	COMPARE_GREATEREQUAL	= 7,
	COMPARE_ALWAYS			= 8,
};


///
/// ZBufferState
/// Use this RenderState to put the renderer into z buffering state
///
class ZBufferState : public RenderState
{
	KDECLARE_RTTI;
	KDECLARE_STREAM( ZBufferState );

public:

	/// Constructor.
	ZBufferState()										: m_zfunction( COMPARE_LESSEQUAL ), m_zenable( true ), m_zwrites( true )		{}

	/// Constructor which enable z buffering and sets the z function
	ZBufferState( CompareOp zfunc, bool zwrite = true ) : m_zfunction( zfunc ), m_zenable( true ), m_zwrites( zwrite )					{}

	/// Constructor which takes a dummy boolean flag to turn off z buffering
	ZBufferState( bool enable, bool zwrite = true )		: m_zfunction( COMPARE_LESSEQUAL ), m_zenable( enable ), m_zwrites( zwrite )	{}

	/// Gets the state of the z buffer
	bool getZBufferEnable() const																{ return m_zenable; }

	/// Gets the z buffer operation
	CompareOp getZBufferCompareOp() const														{ return m_zfunction; }

	/// Gets the z buffer writes flag
	bool getZBufferWrites() const																{ return m_zwrites; }

private:

	/// Z buffer comparison operation
	CompareOp	m_zfunction;

	/// Flag to determine whether to enable z buffering
	bool		m_zenable;

	/// Flag to determine whether z buffer writes are enabled
	bool		m_zwrites;
};

KIMPLEMENT_STREAM( ZBufferState );

} // Katana

#endif // _ZBUFFERSTATE_H