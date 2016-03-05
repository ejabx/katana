/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		version.h
	Author:		Eric Bryant

	Define the version information of the Katana SDK.
*/

#ifndef _VERSION_H_
#define _VERSION_H_

namespace Katana
{
	// -------------------
	// Version Information
	// -------------------
	//
	// The BUILD_VERSION corresponds to the current release of the engine
	// and is used for quality assurance.
	//
	// The FILE_VERSION is the current version of the Katana File Format,
	// and is independent of the BUILD_VERSION because it'll only change
	// when the file's signature has changed (for instance, a class serializes
	// additional information to disk, thus, the file format is invalidated).

	// Build Version of Katana Engine, the format is:
	//		(Major Version).(Minor Version).(QA Version).(Development Version)
	//
	const char BUILD_VERSION[] = "2.0.0.14";

	// File Verion of Katana Engine, the format is:
	//		(Major Version).(Minor Version).(QA Version).(Development Version)

	const char FILE_VERSION[] = "2.0.0.17";

}; // Katana

#endif //_VERSION_H_