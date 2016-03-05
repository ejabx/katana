/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		katana.h
	Author:		Eric Bryant

	SDK Includes
*/


#ifndef _KATANA_H_
#define _KATANA_H_

	/// Define this parameter if you DO NOT want the Katana Game Engine
	/// to autolink the library if you include this header.
	#ifndef KATANA_NO_AUTOLINK
		#ifdef _DEBUG
			#pragma comment(lib, "katana_d.lib")
		#else
			#pragma comment(lib, "katana.lib")
		#endif
	#endif

	// Include files for Windows support 
	#ifdef WIN32
		#include <windows.h>
	#endif

	// Include files for standard template library
	#include <string>
	#include <map>
	#include <stack>
	#include <bitset>
	#include <list>
	#include <vector>
	using namespace std;

	// Include files for Lua
	extern "C" {
		#include <lua.h>
		#include <lauxlib.h>
		#include <lualib.h>
	}

	// Include files for boost
	#include <boost/weak_ptr.hpp>
	#include <boost/shared_ptr.hpp>
	#include <boost/enable_shared_from_this.hpp>
	using namespace boost;

	// Include files for the Katana SDK

	// Core Libraries
	#include "base/kbase.h"
	#include "base/rtti.h"
	#include "base/refcount.h"
	#include "base/handle.h"
	#include "base/kstring.h"
	#include "base/karray.h"
	#include "base/color.h"
	#include "base/log.h"
	#include "base/listener.h"
	#include "base/message.h"
	#include "base/messagerouter.h"
	#include "base/streamable.h"
	#include "base/kstream.h"
	#include "base/kistream.h"
	#include "base/kostream.h"
	#include "base/kexport.h"

	// Math Libraries
	#include "math/kmath.h"
	#include "math/point.h"
	#include "math/plane.h"
	#include "math/matrix.h"
	#include "math/quaternion.h"
	#include "math/bound.h"
	#include "math/box.h"

	// System Specific Libraries
	#include "system/systemtimer.h"
	#include "system/systemfile.h"
	#include "system/systemdialog.h"

	// Scripting Libraries
	#include "script/scriptengine.h"
	#include "script/luahelper.h"

	// Render Libraries
	#include "render/rendertypes.h"
	#include "render/geometry.h"
	#include "render/vertexbuffer.h"
	#include "render/indexbuffer.h"
	#include "render/texture.h"
	#include "render/material.h"
	#include "render/light.h"
	#include "render/renderstate.h"
	#include "render/wireframestate.h"
	#include "render/multitexturestate.h"
	#include "render/cullstate.h"
	#include "render/blendstate.h"
	#include "render/shaderstate.h"
	#include "render/materialstate.h"
	#include "render/zbufferstate.h"

	// Physics System
	#include "physics/physicssystem.h"
	#include "physics/collidable.h"
	#include "physics/rigidbody.h"

	// Scene Libraries
	#include "scene/visible.h"
	#include "scene/camera.h"
	#include "scene/vismesh.h"
	#include "scene/visnode.h"
	#include "scene/scenecontext.h"
	#include "scene/scenegraph.h"
	#include "scene/bspscene.h"
	#include "scene/zone.h"

	// Controller Libraries
	#include "scene/controller.h"
	#include "scene/mousecontroller.h"
	#include "scene/keyboardcontroller.h"

	// Animation Libraries
	#include "animation/keyframe.h"
	#include "animation/animation.h"
	#include "animation/animationtrack.h"
	
	// Input System
	#include "input/inputsystem.h"
	#include "input/inputmessages.h"

	// Game Engine Libraries
	#include "engine/application.h"
	#include "engine/debugoutput.h"
	#include "engine/gameengine.h"
	#include "engine/gamesettings.h"
	#include "engine/shell.h"

#endif //_KATANA_H_