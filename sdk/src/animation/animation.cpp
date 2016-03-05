/*
	Katana Engine
	Copyright © 2004 Eric Bryant, Inc.

	File:		animation.h
	Author:		Eric Bryant

	An animation consists of a collection of animation tracks, which
	are blended together to contribute to an overall animation.
	Or, in the case of skeletal animation, each entry in the
	track represents a bone transformation.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "animation.h"
#include "animationtrack.h"
#include "keyframe.h"

// ------------------------------------------------------------------
// RTTI declaration
// ------------------------------------------------------------------

KIMPLEMENT_RTTI( Animation, Streamable );

// ------------------------------------------------------------------

//
// Copy Constructor
//
Animation::Animation( shared_ptr<Animation> animation )
{
	// Copy the tracks from the animation into our animation
	for( vector< shared_ptr<AnimationTrack>  >::iterator iter = animation->m_animationTracks.begin();
		 iter != animation->m_animationTracks.begin();
		 iter ++ )
	{
		m_animationTracks.push_back( *iter );
	}
}

//
// addTrack
// Adds an additional track
//
void Animation::addTrack( shared_ptr<AnimationTrack> animationTrack )
{ 
	// Add the track
	m_animationTracks.push_back( animationTrack ); 

	// Update the animation length
	if ( m_animationLength < animationTrack->getMaximumKeyframeTime() )
		m_animationLength = animationTrack->getMaximumKeyframeTime();
}

//
// advance
// Advances the animation time (or rewinds if the deltaTime is negative).
// This will adjust the animation time and return the resultant keyframe
// from executing the animation tracks
//
bool Animation::advance( float deltaTime, Point3 & position, Quaternion & rotation, float fWeight )
{
	// If we don't have any animation tracks, exit
	if ( m_animationTracks.size() == 0 ) return false;

	// If this animation is disabled, exit
	if ( !m_enabled ) return false;

	// Update our total animation time
	m_currentAnimationTime += deltaTime;

	// Iterate over the animation tracks and compute the animation length
	m_animationLength = 0;
	for( vector< shared_ptr<AnimationTrack> >::iterator iter = m_animationTracks.begin();
		iter != m_animationTracks.end();
		iter++ )
	{
		m_animationLength += (*iter)->getMaximumKeyframeTime();
	}

	// If the total time is back the total animation time, and we can loop, begin the 
	// animation at time 0
	if ( m_currentAnimationTime > m_animationLength ) 
	{
		// Reset the current time if we can loop
		if ( m_canLoop ) {
			m_currentAnimationTime = 0;
		
		// Otherwise, clamp the animation time and disable it for the next tick
		} else {
			m_currentAnimationTime = m_animationLength;
			m_enabled = false;
		}
	}	

	// Iterate over the animation tracks to have it apply its keyframes
	// to the input position and rotation.
	for( vector< shared_ptr<AnimationTrack> >::iterator iter = m_animationTracks.begin();
		 iter != m_animationTracks.end();
		 iter++ )
	{
		// Retrieve the interpolated keyframe
		Keyframe interpKey = ( *iter )->getInterpolatedKeyframe( m_currentAnimationTime );

		// Apply the keyframe (weighted) towards the position and rotation
		// NOTE: Quaternion are identity quaternion by default ([X,Y,Z,W] = [0,0,0,1])
		//		 So this is a slerp between the identity rotation and the final, interpolated rotation
		// NOTE2: We explicitly check if the translation isn't zero, which means there is no change in translation
		if ( interpKey.m_translation.getLength() ) position = interpKey.m_translation * fWeight;
		rotation = rotation.slerp( fWeight, Quaternion(), interpKey.m_rotation );
	}

	return true;
}

// -------------------------------------------------------


//
// OnLoadStream
//
bool Animation::OnLoadStream( kistream & istr )
{
	// Load the revelant member variables
	istr >> m_enabled;
	istr >> m_canLoop;
	istr >> m_animationLength;
	loadAsRefs( istr, m_animationTracks );

	return true;
}

//
// OnSaveStream
//
bool Animation::OnSaveStream( kostream & ostr ) const
{
	// Save the revelant member variables
	ostr << m_enabled;
	ostr << m_canLoop;
	ostr << m_animationLength;
	saveAsRefs( ostr, m_animationTracks );

	return true;
}