/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		debugoutput.cpp
	Author:		Eric Bryant

	Debugging facility of the katana engine. Use this class to
	register tracing of objects, or debug rendering objects
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "render/rendertypes.h"
#include "render/geometry.h"
#include "render/light.h"
#include "render/render.h"
#include "scene/visible.h"
#include "scene/camera.h"
#include "scene/scenecontext.h"
#include "scene/scenegraph.h"
#include "textdisplay.h"
#include "debugoutput.h"

//
// Constructor
//
DebugOutput::DebugOutput() 
	: m_drawNormals( false )
	, m_drawTangents( false )
	, m_drawLights( false )
	, m_drawBounds( false )
	, m_drawSceneStatistics( false )
	, m_enableCulling( true )
	, m_drawShadowVolumes( false )
{
}

//
// addTrace
//
void DebugOutput::addTrace( Visible * object )
{
	m_traceObjects.push_back( object );
}

//
// OnDebugOutput
//
void DebugOutput::OnDebugOutput( SceneContext * context )
{
	// Display the trace information for each traced object
	for( vector<Visible *>::iterator iter_t = m_traceObjects.begin(); iter_t != m_traceObjects.end(); iter_t++ )
		DisplayTraceInfo( (*iter_t), context );

	// Display the scene statistics
	if ( m_drawSceneStatistics )
	{
		const SceneStatistics & sceneStatistics = context->currentScene->getStatistics();
		kstring fps; fps.format( "fps: %2.2f", sceneStatistics.framesPerSecond );
//		kstring tps; tps.format( "tps: %2.2f", sceneStatistics.trianglesPerSecond );
		kstring objects; objects.format( "objects: %d", sceneStatistics.objectsRenderedLastFrame );
		context->textDisplay->drawText( 5, 5, fps.c_str() );
		context->textDisplay->drawText( 5, 23, objects.c_str() );
//		context->textDisplay->drawText( 5, 31, tps.c_str() );
	}
}

//
// DisplayTraceInfo
//
void DebugOutput::DisplayTraceInfo( Visible * object, SceneContext * context )
{
	if ( object->isDirty() )
	{
		// Game time and object identifier
		KLOG ("TRACE (%2.2fs): <%s>", context->gameTime * 0.001f, object->GetRTTI()->GetName() );

		// Translation
		KLOG2("translation: (%0.2f, %0.2f, %0.2f)", 
				object->getTranslation().x, object->getTranslation().y, object->getTranslation().z );

		// Rotation
		Matrix4 m; object->getRotation().toMatrix( m );
		KLOG2("rotation:\n\tRIGHT=(%0.2f, %0.2f, %0.2f)"
			   "\n\tUP=(%0.2f, %0.2f, %0.2f)"
			   "\n\tAT=(%0.2f, %0.2f, %0.2f)\n",
			   m.right.x, m.right.y, m.right.z, m.up.x, m.up.y, m.up.z,
			   m.at.x, m.at.y, m.at.z );
	}
}