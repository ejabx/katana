/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		debugoutput.h
	Author:		Eric Bryant

	Debugging facility of the katana engine. Use this class to
	register tracing of objects, or debug rendering objects
*/

#ifndef _DEBUGOUTPUT_H
#define _DEBUGOUTPUT_H

namespace Katana
{

//
// Forward Declaration
//
class Streamable;
class Visible;
class VisMesh;
class Light;
struct Geometry;
struct SceneContext;

///
/// DebugOutput
/// 
class DebugOutput
{
public:
	/// Sets whether to draw normals for visible meshes
	void setDrawNormals( bool value )							{ m_drawNormals = value; }

	/// Gets whether to draw normals for visible meshes
	bool getDrawNormals() const									{ return m_drawNormals; }

	/// Sets whether to draw tangents for visible meshes
	void setDrawTangents( bool value )							{ m_drawTangents = value; }

	/// Gets whether to draw tangents for visible meshes
	bool getDrawTangents() const								{ return m_drawTangents; }

	/// Sets whether to draw lights
	void setDrawLights( bool value )							{ m_drawLights = value; }

	/// Gets whether to draw lights
	bool getDrawLights() const									{ return m_drawLights; }

	/// Sets whether to draw bounds
	void setDrawBounds( bool value )							{ m_drawBounds = value; }

	/// Gets whether to draw bounds
	bool getDrawBounds() const									{ return m_drawBounds; }

	/// Sets whether to draw scene statistics
	void setDrawStatistics( bool value )						{ m_drawSceneStatistics = value; }

	/// Gets whether to draw scene statistics
	bool getDrawStatistics() const								{ return m_drawSceneStatistics; }

	/// Sets whether to enable frustum culling
	void setEnableFrustumCulling( bool value )					{ m_enableCulling = value; }

	/// Gets whether to enable frustum culling
	bool getEnableFrustumCulling() const						{ return m_enableCulling; }

	/// Sets whether to draw shadow volumes
	void setDrawShadowVolumes( bool value )						{ m_drawShadowVolumes = value; }

	/// Gets whether to draw shadow volumes
	bool getDrawShadowVolumes() const							{ return m_drawShadowVolumes; }

	/// Adds a trace on a visible object. When it is dirty, it will write it's new transform to the log
	void addTrace( Visible * object );

public:

	/// Constructor
	DebugOutput();

	/// Called by the game engine to display it's debug information
	void OnDebugOutput( SceneContext * context );

private:

	/// Checks if the visible object is dirty and display trace information if it is
	static void DisplayTraceInfo( Visible * object, SceneContext * context );

private:
	/// Collection of objects for tracing
	vector<Visible *> m_traceObjects;

	/// Debugging value which determines whether normals are drawn on visible meshes
	bool	m_drawNormals;

	/// Debugging value which determines whether tangent basis are drawn on visible meshes
	bool	m_drawTangents;

	/// Debugging value which determines whether lights are displayed
	bool	m_drawLights;

	/// Debugging value which determines whether bounding spheres are displayed
	bool	m_drawBounds;

	/// Debugging value which determines whether scene statistics are displayed onscreen (fps, tps, etc.)
	bool	m_drawSceneStatistics;

	/// Debugging value which determine whether to enable frustum culling
	bool	m_enableCulling;

	/// Debugging value which determines whether to draw shadow volumes
	bool	m_drawShadowVolumes;
};

}; // Katana

#endif _DEBUGOUTPUT_H