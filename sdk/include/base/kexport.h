/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		kexport.h
	Author:		Eric Bryant

	Interface for exporting files
*/

#ifndef _KEXPORT_H
#define _KEXPORT_H

namespace Katana
{

/// Specifies what kind of entites to export
enum ExportType { EXPORT_GEOMETRY, EXPORT_BIPED, EXPORT_BSP, EXPORT_ANIMATION };

///
/// ExportSettings
/// This structure is passed to the enumerateScene function
/// and indicates to the client the export settings.
///
struct ExportSettings
{
public:
	/// Default constructor
	ExportSettings();

	/// Constructor which takes a settings file
	ExportSettings( const char * szSettingsFile );

	/// Loads the default settings
	bool loadSettings( const char * szSettingsFile );

	/// Save the default settings
	bool saveSettings( const char * szSettingsFile );

public:
	/// General Settings
	string		exportFileSourcePath;			/// Source File for Export
	string		exportFileDestinationPath;		/// Destation File for Export
	ExportType	exportType;						/// Type of Export
	bool		selectionOnly;					/// Only export selected object
	bool		previewOnly;					/// Preview the export only

	/// Geometry Settings
	bool		includeTextures;				/// Include textures in exported file
	bool		embeddedTextures;				/// Embed the textures within the exported file. If false only the texture path is save
	bool		textureNameOnly;				/// Only include the texture names, not the path
	bool		computeNormals;					/// Include the normals for the meshes
	bool		computeTangents;				/// Compute the tangent vectors for the meshes (S only)
	bool		computeColors;					/// Include the colors for the meshes
	bool		computeSecondaryTextureCoordinates;	/// Include secondary texture coordinates (if available)
	bool		includeAnimation;				/// Include animation (if available)
	bool		exportAnimationOnly;			/// Only export the animation
	int			animStartFrame;					/// Start frame of the exported animation
	int			animEndFrame;					/// End frame of the exported animation
	bool		stripifyGeometry;				/// Compute triangle strips from the meshes (instead of triangle lists)
	int			computeNumLODs;					/// Generate LODs for the meshes (0 = no LOD)
	bool		exportInWorldSpace;				/// Export the nodes in world space instead of object space

	/// Error Description (if applicable)
	string		lastExportError;				/// If the export resulted in an error, it is reported here
};

///
/// kexport
///
class kexport
{
public:
	/// This method is called by the client to request that the class
	/// walks through its scene graph, and export objects into the
	/// exported scene via kostream::insertObject(). These objects
	/// are serialized during endStream() or during the destructor.
	virtual bool enumerateScene( ExportSettings & settings )=0;
};

} // Katana

#endif // _KEXPORT_H