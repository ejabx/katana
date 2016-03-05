/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		systeminfo.h
	Author:		Eric Bryant

	Gathers information about the machine (CPU, rendering devices/modes, etc.)
*/

#ifndef _SYSTEMINFO_H
#define _SYSTEMINFO_H

namespace Katana
{

///
/// DisplayMode
///
struct DisplayMode
{
	DisplayMode( unsigned int w, unsigned int h, unsigned b ) :
		width ( w ), height( h ), bitDepth( b ) {}

	unsigned int width;
	unsigned int height;
	unsigned int bitDepth;
};

///
/// SystemInfo
///
class SystemInfo
{
public:
	enum ProcessorType
	{
		UNDEFINED,
		PENTIUM_I,
		PENTIUM_II,
		PENTIUM_III,
		PENTIUM_IV,
		AMD,
		OTHER,
	};

	enum { MAX_DISPLAY_MODES = 5 };

public:
	/// Constructor
	SystemInfo();

	/// Returns the CPU Clock Speed (in MHz)
	float getCPUSpeed();

	/// Returns the processor type
	ProcessorType getProcessorType();

	/// Does this machine have a math co-processor
	bool hasMathCoprocessor();

	/// Returns a list of rendering device names on this machine
	std::vector<std::string> getRenderDevices();

	/// Returns a list of display modes for the indexed device
	std::vector<DisplayMode> getDisplayModes( unsigned int deviceIndex = 0 );

	/// Query all of the above information (and outputs it to the log)
	void queryAllSystemInfo();


private:
	float						m_fCPUSpeed;							/// Speed of the current machine
	int							m_nMathCoprocessor;						/// -1 = untested, 0 = false, 1 = true
	ProcessorType				m_ProcessorType;						/// Type of the processor
	std::vector<std::string>	m_renderDevices;						/// List of render devices
	std::vector<DisplayMode>	m_displayModes[MAX_DISPLAY_MODES];		/// Display modes for all devices
};

}; // Katana

#endif _SYSTEMINFO_H