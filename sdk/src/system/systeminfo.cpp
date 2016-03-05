/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		systeminfo.cpp
	Author:		Eric Bryant

	Gathers information about the machine (CPU, rendering devices/modes, etc.)
*/

#include <windows.h>
#include "katana_core_includes.h"
#include "systeminfo.h"
#include "base/log.h"

//
// Constructor
//
SystemInfo::SystemInfo() :
	m_fCPUSpeed( 0 ),
	m_ProcessorType( UNDEFINED ),
	m_nMathCoprocessor( -1 )
{
}

//
// getCPUSpeed
//
// Code originally written by Frank Blaha.
//
float SystemInfo::getCPUSpeed()
{
	// Compute the CPU Speed
	if ( !m_fCPUSpeed )
	{
		LARGE_INTEGER   ulFreq,			// No. ticks/s ( frequency). 
										//This is exactly 1 second interval
						ulTicks,		//Curent value of ticks 
						ulValue,		//for calculation, how many tick is system
										//(depend on instaled HW) able    
						ulStartCounter, //start No. of processor counter
						ulEAX_EDX,		//we need 64 bits value and it is stored in EAX,EDX registry
						ulResult;		//calculate result of "measuring"
		    

		// Function retrieves the frequency of the high-resolution performance counter(HW not CPU)   
		// it is number of ticks per second       
		QueryPerformanceFrequency(&ulFreq); 	   


		// Current value of the performance counter        
		QueryPerformanceCounter(&ulTicks);   

		// Calculate one sec interval 
		// ONE SEC interval  = start nuber of the ticks + # of ticks/s
		// loop ( do..while statement bellow) until actual # of ticks this number is <= then 1 sec

		ulValue.QuadPart = ulTicks.QuadPart + ulFreq.QuadPart;    

		// (read time-stamp counter) instruction.    
		// This asm instruction loads the high-order 32 bits of the register into EDX, and the low-order 32 bits into EAX.      

		__asm RDTSC     

		// Load 64-bits counter from registry to LARGE_INTEGER variable( take a look to HELP)
		__asm mov ulEAX_EDX.LowPart, EAX         
		__asm mov ulEAX_EDX.HighPart, EDX       


		// Starting number of processor ticks    

		ulStartCounter.QuadPart = ulEAX_EDX.QuadPart;                 

		// Loop for 1 sec and  check ticks        
		// this is descibed bellow
		do
		{	         
			// Just read actual HW counter
			QueryPerformanceCounter(&ulTicks); 

		}while( ulTicks.QuadPart <= ulValue.QuadPart) ;         


		// Get actual number of processor ticks      
		__asm RDTSC       

		__asm mov ulEAX_EDX.LowPart, EAX        
		__asm mov ulEAX_EDX.HighPart,EDX       


		// Calculate result from current processor ticks count
		ulResult.QuadPart = ulEAX_EDX.QuadPart - ulStartCounter.QuadPart;

		// Calculated frequency (in MHz)
		m_fCPUSpeed = float(ulResult.QuadPart / 1000000);
	}

	return m_fCPUSpeed;
}

//
// getProcessorType
//
SystemInfo::ProcessorType SystemInfo::getProcessorType()
{
	// Compute the processor type
	if ( m_ProcessorType == UNDEFINED )
	{
		m_ProcessorType = PENTIUM_IV; // TODO
	}

	return m_ProcessorType;
}

//
// hasMathCoprocessor
//
bool SystemInfo::hasMathCoprocessor()
{
	// Check if this machine has a math coprocessor
	if ( m_nMathCoprocessor < 0 )
	{
		m_nMathCoprocessor = 1; // TODO
	}

	return ( m_nMathCoprocessor == 0 ? false : true );
}

//
// getRenderDevices
//
std::vector<std::string> SystemInfo::getRenderDevices()
{
	// Get the rendering device names
	if ( !m_renderDevices.size() )
	{
		// Enumerate through the display devices, storing the names
		DISPLAY_DEVICE displayDevice;
		displayDevice.cb = sizeof( DISPLAY_DEVICE );
		DWORD iDeviceNumber = 0;
		while( EnumDisplayDevices( NULL, iDeviceNumber, &displayDevice, NULL ) )
		{
			// Only add devices which are "renderable"
			if ( !( displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER ) )
				m_renderDevices.push_back( displayDevice.DeviceString );

			iDeviceNumber++;
		}
	}

	return m_renderDevices;
}

//
// getDisplayModes
//
std::vector<DisplayMode> SystemInfo::getDisplayModes( unsigned int deviceIndex )
{
	// If the device index is greater than the maximum, return a empty array
	if ( deviceIndex > MAX_DISPLAY_MODES || deviceIndex > getRenderDevices().size() )
	{
		std::vector<DisplayMode> nullDisplay;
		return nullDisplay;
	}

	// Get the render display mode for the given index
	if ( !m_displayModes[deviceIndex].size() )
	{
		// Retrieve the display name of the specified device
		DISPLAY_DEVICE displayDevice;
		displayDevice.cb = sizeof( DISPLAY_DEVICE );
		if ( !EnumDisplayDevices( NULL, deviceIndex, &displayDevice, NULL ) )
		{
			std::vector<DisplayMode> nullDisplay;
			return nullDisplay;
		}

		// Iterate through the resolutions
		DEVMODE DevMode;
		DWORD iGraphicMode = 0;
		while ( EnumDisplaySettings( displayDevice.DeviceName, iGraphicMode, &DevMode) )
		{
			// Ignore bitdepth less than 16 and display frequencies not equal to 60HZ
			if ( DevMode.dmBitsPerPel >= 16 && DevMode.dmDisplayFrequency == 60 )
			{
				m_displayModes[deviceIndex].push_back( 
					DisplayMode( DevMode.dmPelsWidth, DevMode.dmPelsHeight, DevMode.dmBitsPerPel ) );
			}
			
			iGraphicMode++;
		}
	}

	return m_displayModes[deviceIndex];
}

//
// queryAllSystemInfo
//
void SystemInfo::queryAllSystemInfo()
{
	// Collect all the system information.
	float fCPUSpeed			= getCPUSpeed();
	ProcessorType processor = getProcessorType();
	bool bMathCoprocessor	= hasMathCoprocessor();
							  getRenderDevices();
							  getDisplayModes();

	// Determine the processor string
	std::string processorStr;
		 if ( processor == PENTIUM_I )
		processorStr = "PENTIUM I";
	else if ( processor == PENTIUM_II )
		processorStr = "PENTIUM II";
	else if ( processor == PENTIUM_III )
		processorStr = "PENTIUM III";
	else if ( processor == PENTIUM_IV )
		processorStr = "PENTIUM IV";
	else if ( processor == AMD )
		processorStr = "AMD";
	else 
		processorStr = "OTHER (cyrix, etc.)";

	// Determine the math coprocessor string
	std::string mathStr;
	if ( bMathCoprocessor )
		mathStr = "YES";
	else
		mathStr = "NO";

	// Log the information
	KLOG2("Machine Statistics:\r\n"
		  "\tProcessor Type: %s\r\n"
		  "\tCPU Speed: %1.3fMHz\r\n"
		  "\tMath Coprocessor: %s\r\n",
		  processorStr.c_str(),
		  fCPUSpeed / 1000,
		  mathStr.c_str()
	);
}