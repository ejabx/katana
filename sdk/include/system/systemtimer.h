/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		systemtimer.h
	Author:		Eric Bryant

	High Resolution Timer
*/

#ifndef _SYSTEMTIMER_H
#define _SYSTEMTIMER_H

namespace Katana
{

///
/// SystemTimer
/// High Resolution Timer
class SystemTimer
{
public:
	/// Constructor (set bStartWatch false to start watch in stopped state)
    SystemTimer( bool bStartWatch = true );

	/// Starts the Timer (keeping previous time)
    void  Start();

	/// Starts the Timer (clearing the previous time)
    void  StartZero();

	/// Pauses the Timer
    void  Stop();

	/// Resets the Time
    void  Reset();

	/// Is the Timer current running?
    bool  isRunning() const;

	/// Elasped seconds since timer was reset
    float GetElapsedSeconds() const;

	/// Elapses milliseconds since timer was reset
    float GetElapsedMilliseconds() const;

private:

#ifdef _WIN32
	/// Get high-resolution timer
    __int64 GetTicks() const;
#endif

private:
    float	m_timerPeriod;        /// Seconds per tick (1/Hz)
    bool	m_isRunning;          /// True if watch is running

#ifdef _WIN32
    __int64	m_startTick;          /// Time watch last started/reset
    __int64	m_prevElapsedTicks;   /// Time watch was previously running
#endif
};

} // Katana

#endif // _SYSTEMTIMER_H