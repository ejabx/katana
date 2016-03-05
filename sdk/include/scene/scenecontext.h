/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		scenecontext.h
	Author:		Eric Bryant

	
*/

#ifndef _SCENECONTEXT_H
#define _SCENECONTEXT_H

namespace Katana
{

//
// Forward Declarations
//
class Camera;
class Render;
class Matrix4;
class DebugOutput;
class Light;
class VertexShader;
class PixelShader;
class VisNode;
class Visible;
class TextDisplay;
class SceneGraph;

///
/// SceneContext
/// Encapsulates the state of the scene graph.
///
struct SceneContext
{
	float						deltaTime;
	float						gameTime;
	int							renderPass;
	int							frameCount;
	DebugOutput *				debugOutput;
	TextDisplay *				textDisplay;
	SceneGraph *				currentScene;
	Render *					currentRenderer;
	const Camera *				currentCamera;
	const Matrix4 *				currentViewMatrix;
	Visible *					currentVisibleObject;
	VisNode *					currentParent;
	vector< shared_ptr<Light> > currentLights;
};

///
/// SceneStats
/// Statistics which are kept track on a per-frame and per-game instance basis
///
struct SceneStatistics
{
	int		objectsRenderedLastFrame;			/// Total objects rendered within the last frame
	int		trianglesRenderedLastFrame;			/// Total triangles rendered within the last frame
	int		totalObjectsRendered;				/// Total objects rendered within the lifetime of the game
	int		totalTrianglesRendered;				/// Total triangles rendered within the lifetime of the game
	float	framesPerSecond;					/// Number of frames the game is rendering per second
	int		trianglesPerSecond;					/// Number of triangles the game is rendering per second
};

} // Katana

#endif // _SCENECONTEXT_H