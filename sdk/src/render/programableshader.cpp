/*
	Katana Engine
	Copyright © 2001-2004 Eric Bryant, Inc.

	File:		programableshader.cpp
	Author:		Eric Bryant

	Class which represents vertex and fragment shaders.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "system/systemfile.h"
#include "scene/scenecontext.h"
#include "scene/visible.h"
#include "scene/camera.h"
#include "rendertypes.h"
#include "render.h"
#include "renderstate.h"
#include "multitexturestate.h"
#include "shaderstate.h"
#include "material.h"
#include "light.h"
#include "shader.h"
#include "programableshader.h"

// ------------------------------------------------------------------
// RTTI Definition
// ------------------------------------------------------------------

KIMPLEMENT_RTTI( ProgramableShader, Shader );

// ------------------------------------------------------------------
// ProgramableShader
// ------------------------------------------------------------------

//
// Constructors
//
ProgramableShader::ProgramableShader()
{
}

ProgramableShader::ProgramableShader( const char * szShaderFile )
{
	// Create the shader state which encapsulates the shader
	// code as well as the constant
	m_shaderState.reset( new ShaderState( szShaderFile ) );

	// By default, add room for the default matrices to the vertex shader constants
	vector<ShaderConstant> & vconstants = m_shaderState->getShaderConstants0();
	vconstants.resize( MAX_DEFAULT_VERTEX_CONSTANTS ); // Reserve space for the constants
	memset( &vconstants[0], 0, sizeof(ShaderConstant) * MAX_DEFAULT_VERTEX_CONSTANTS );

	// By default, add room for the material within the pixel shader constants
	vector<ShaderConstant> & pconstants = m_shaderState->getShaderConstants1();
	pconstants.resize( MAX_DEFAULT_PIXEL_CONSTANTS ); // Reserve space for the constants
	memset( &pconstants[0], 0, sizeof(ShaderConstant) * MAX_DEFAULT_PIXEL_CONSTANTS );
}

//
// OnPreRender
// Called for every object which needs to render with this shader
// It preprocesses the shader by setting the appropiate shader constants
//
bool ProgramableShader::OnPreRender( SceneContext * context )
{
	// Check that we have a valid shader state
	if ( !m_shaderState ) return false;

	// Get a pointer to the currently rendered object
	Visible * pCurrentVisibleObject = context->currentVisibleObject;

	// Calculate the commonly used matrices
	const Matrix4 modelViewMatrix = pCurrentVisibleObject->getWorldMatrix();
	const Matrix4 modelViewMatrixIT = Matrix4( modelViewMatrix ).inverse().transpose();
	const Matrix4 modelViewProjectionMatrix = modelViewMatrix * context->currentCamera->getProjection();

	// Grab the shader constants
	vector<ShaderConstant> & vshaderConstants = m_shaderState->getShaderConstants0();
	vector<ShaderConstant> & pshaderConstants = m_shaderState->getShaderConstants1();

	// Store the matricies within the shader constants
	vshaderConstants[VERTEX_CONST_WORLDVIEWPROJ] = modelViewProjectionMatrix;
	vshaderConstants[VERTEX_CONST_WORLDVIEW] = modelViewMatrix;
	vshaderConstants[VERTEX_CONST_WORLDVIEW_IT] = modelViewMatrixIT;

	// Store the eye position in object space
	vshaderConstants[VERTEX_CONST_EYE_POSITION] = context->currentCamera->getWorldMatrix().pos;

	// This structure mimics a Matrix4 which is what the shader takes as an argument
	static struct MaterialConstant {
		ColorA ambient;
		ColorA diffuse;
		ColorA specular;
		ColorA emissive; // ( Alpha = Opacity )
	} materialConstant;

	// Grab the diffuse texture from the visible object's material and set this as the target texture map
	shared_ptr<Material> material = pCurrentVisibleObject->getMaterial();
	if ( material )
	{
		// Generate the texture states. The state will be responsible for setting the appropate texture slots
		MultitextureState multiTextureState( material );

		// Setup the textures within the renderer
		context->currentRenderer->SetState( &multiTextureState );

		// Fill the material structure with the material parameters
		materialConstant.ambient = material->ambient;
		materialConstant.diffuse = material->diffuse;
		materialConstant.specular = material->specular;
		materialConstant.emissive = material->emissive;

		// Store the local material constant into the shader constant
		memcpy( &pshaderConstants[PIXEL_CONST_MATERIAL].matrix, &materialConstant.ambient, sizeof(MaterialConstant) );

		// This ensure we copy the right amount of memory into the hardware shader constant
		pshaderConstants[PIXEL_CONST_MATERIAL].constSize = sizeof(MaterialConstant);
	}
	else
	{
		// If no texture maps are available, then set the texture state without any
		// textures. This will unbind all texture stages.
		context->currentRenderer->SetState( &MultitextureState() );
	}

	// Grab the lights
	vector< shared_ptr<Light> > & currentLights = context->currentLights;

	// If there are current lights enabled, then setup the lighting parameters within the constants
	if ( currentLights.size() )
	{
		for( vector< shared_ptr<Light> >::iterator iter = currentLights.begin();
			 iter != currentLights.end();
			 iter++ )
		{
			// Grab the current light
			shared_ptr<Light> & light = (*iter);

			// This structure mimics a Matrix4 which is what the shader takes as an argument
			static struct LightConstant {
				ColorA ambient;
				ColorA diffuse;
				ColorA specular;
				ColorA emissive; // ( Alpha = Opacity )
			} lightConstant;

			// Store the light position in object space
			vshaderConstants[VERTEX_CONST_LIGHT_POSITION] = light->getPosition();

			// If this is a point light, setup the attenuation
			if ( shared_ptr<PointLight> pointLight = boost::dynamic_pointer_cast<PointLight>( light ) )
			{
				vshaderConstants[VERTEX_CONST_LIGHT_ATTENUATION] = 
					Point4( pointLight->getConstantAttenuation(), 
							pointLight->getLinearAttenuation(), 
							pointLight->getQuadraticAttenuation(),
							pointLight->getRange() );
			}

			// Modulate the material with the light parameters
			materialConstant.ambient *= light->getAmbient();
			materialConstant.diffuse *= light->getDiffuse();
			materialConstant.specular *= light->getSpecular();
		}
	}

	// Tell the renderer to render the scene using this shader
	if ( !context->currentRenderer->SetState( m_shaderState.get() ) )
		return false;

	// Call the base class to execute the pre render states
	return Shader::OnPreRender( context );
/*
	// Check whether we have an active vertex shader set
	if ( context->currentVertexShader.expired() ) return false;

	// Set references to the shaders
	weak_ptr<VertexShader> wpVertexShader = context->currentVertexShader;
	weak_ptr<PixelShader> wpPixelShader = context->currentPixelShader;

	shared_ptr<VertexShader> vertexShader = wpVertexShader.lock();
	shared_ptr<PixelShader> pixelShader = wpPixelShader.lock();

	// Check whether we have enough space for the vertex shader constants
	if ( vertexShader->constants.size() < MAX_DEFAULT_VERTEX_CONSTANTS )
	vertexShader->constants.resize( MAX_DEFAULT_VERTEX_CONSTANTS );
	if ( pixelShader && pixelShader->constants.size() < MAX_DEFAULT_PIXEL_CONSTANTS )
	pixelShader->constants.resize( MAX_DEFAULT_PIXEL_CONSTANTS );

	// Useful matrices
	const Matrix4 modelViewProjMatrix	= Matrix4( m_worldViewMatrix * context->currentCamera->getProjection() ).transpose();
	const Matrix4 modelViewMatrix		= Matrix4( m_worldViewMatrix );
	const Matrix4 modelViewMatrixIT		= Matrix4( m_worldViewMatrix ).inverse().transpose();

	// Store the common vertex shader constants
	vertexShader->constants[ VERTEX_CONST_WORLDVIEWPROJ ]		= modelViewProjMatrix;
	vertexShader->constants[ VERTEX_CONST_WORLDVIEW_IT ]		= modelViewMatrixIT;
	vertexShader->constants[ VERTEX_CONST_EYE_POSITION ]		= context->currentCamera->getPos() * modelViewMatrixIT;

	if ( context->currentLight )
	{
	// Setup the light specfic vertex shader constants
	vertexShader->constants[ VERTEX_CONST_LIGHT_OBJECT_POSITION ]		= context->currentLight->getPosition() * modelViewMatrix;
	vertexShader->constants[ VERTEX_CONST_LIGHT_POSITION ]				= context->currentLight->getPosition();
	vertexShader->constants[ VERTEX_CONST_LIGHT_COLOR ]					= context->currentLight->getDiffuse();
	vertexShader->constants[ VERTEX_CONST_LIGHT_ATTENUATION ]			= GetLightAttenuation( context->currentLight );
	vertexShader->constants[ VERTEX_CONST_PROJECTION_TEXTURE_MATRIX ]	= GetLightProjection( context->currentLight ) * modelViewProjMatrix;

	// Check if we have a pixel shader, and if so, setup the appropiate constants
	// and send them to the renderer
	if ( pixelShader )
	{
	pixelShader->constants[ PIXEL_CONST_LIGHT_COLOR ]			= context->currentLight->getDiffuse();
	pixelShader->constants[ PIXEL_CONST_LIGHT_DIRECTION ]		= GetLightDirection( context->currentLight );
	pixelShader->constants[ PIXEL_CONST_LIGHT_COS_ANGLE ]		= -GetLightConeAngle( context->currentLight );

	context->currentRenderer->SetShaderConstants( &vertexShader->constants, &pixelShader->constants );
	}
	else
	{
	// Setup the constants (without pixel shader constants)
	context->currentRenderer->SetShaderConstants( &vertexShader->constants );
	}
	}
*/
}

//
// OnPostRender
//
bool ProgramableShader::OnPostRender( SceneContext * context )
{
	// Disable the programmable shaders
	context->currentRenderer->SetState( &ShaderState() );

	// Call the base class to execute the post render states
	return Shader::OnPostRender( context );
}

// ------------------------------------------------------------------------

//
// OnLoadStream
//
bool ProgramableShader::OnLoadStream( kistream & istr )
{
	return true;
}

//
// OnSaveStream
//
bool ProgramableShader::OnSaveStream( kostream & ostr ) const
{
	return true;
}