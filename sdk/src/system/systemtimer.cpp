/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		systemtimer.cpp
	Author:		Eric Bryant

	High Resolution Timer
*/

#include <windows.h>
#include "katana_core_includes.h"
#include "systemtimer.h"

//
// Constructor
//
SystemTimer::SystemTimer(bool bStartWatch) :
    m_timerPeriod      ( 0.0f ),
    m_startTick        ( 0 ),
    m_prevElapsedTicks ( 0 ),
    m_isRunning        ( false )   // set in Start()
{
    // Get the timer frequency (ticks per second)
    LARGE_INTEGER qwTimerFreq;
    QueryPerformanceFrequency( &qwTimerFreq );

    // Store as period to avoid division in GetElapsed()
    m_timerPeriod = 1.0f / (float)( qwTimerFreq.QuadPart );

    if( bStartWatch )
	{
        Start();
	}
}

//
// Start
//
void SystemTimer::Start()
{
    m_startTick = GetTicks();
    m_isRunning = true;
}

//
// StartZero
//
VOID SystemTimer::StartZero()
{
    m_prevElapsedTicks = 0;
    m_startTick = GetTicks();
    m_isRunning = true;
}

//
// Stop
//
void SystemTimer::Stop()
{
    if( m_isRunning )
    {
        // Store the elapsed time
        m_prevElapsedTicks += GetTicks() - m_startTick;
        m_startTick = 0;

        // Stop running
        m_isRunning = false;
    }
}

//
// Reset
//
void SystemTimer::Reset()
{
    m_prevElapsedTicks = 0;
    if( m_isRunning )
	{
        m_startTick = GetTicks();
	}
}


//
// isRunning
//
bool SystemTimer::isRunning() const
{
    return( m_isRunning );
}

//
// GetElapsedSeconds
//
float SystemTimer::GetElapsedSeconds() const
{
    // Start with any previous time
    LONGLONG nTotalTicks( m_prevElapsedTicks );

    // If the watch is running, add the time since the last start
    if( m_isRunning )
	{
        nTotalTicks += GetTicks() - m_startTick;
	}

    // Convert to floating pt
    float fSeconds = (float)( nTotalTicks ) * m_timerPeriod;
    return( fSeconds );
}


//
// GetElapsedMilliseconds
//
float SystemTimer::GetElapsedMilliseconds() const
{
    return( GetElapsedSeconds() * 1000.0f );
}


//
// GetTicks
//
__int64 SystemTimer::GetTicks() const
{
    // Grab the current tick count
    LARGE_INTEGER qwCurrTicks;
    QueryPerformanceCounter( &qwCurrTicks );
    return( qwCurrTicks.QuadPart );
}