/*
	Katana Engine
	Copyright © 2004 Eric Bryant, Inc.

	File:		animationtrack.cpp
	Author:		Eric Bryant
	Credits:	Ogre, Object-oriented Graphics Rendering Engine
				Copyright 2000-2002
				http://www.ogre3d.org/

	An animation track is a collection of keyframes. The track interpolates
	the keyframes at a given time to determine the resultant position and
	rotation that constitute the animation.
*/

#include "katana_core_includes.h"
#include "katana_base_includes.h"
#include "keyframe.h"
#include "animationtrack.h"

// ------------------------------------------------------------------
// RTTI declaration
// ------------------------------------------------------------------

KIMPLEMENT_RTTI( AnimationTrack, Streamable );

// ------------------------------------------------------------------

//
// getKeyframesAtTime
// Gets two keyframes which fall between the given time index
//
float AnimationTrack::getKeyframesAtTime( float fTime, Keyframe & keyFrame1, Keyframe & keyFrame2 )
{
	// This are the indices of the first and last keyframes
	int keyframeIndex1 = -1, keyframeIndex2 = -1;

	// Get the animation length
	float fAnimationLength = getMaximumKeyframeTime();

	// Find the last keyframe before or on the current time
	vector<Keyframe>::const_iterator iter = m_keyframes.begin();
	while( ( iter != m_keyframes.end() ) && ( (*iter).m_time <= fTime ) )
	{
		keyFrame1 = *iter++;
		++keyframeIndex1;
	}

	// Make sure we found a keyframe. If not, then assume we just use the first keyframe
	if ( keyframeIndex1 == -1 )
	{
		keyFrame1 = *iter;
		++keyframeIndex1;
	}

	// Parametric time
	// t1 = time of previous keyframe
	// t2 = time of next keyframe 
	float t1, t2;

	// Find the first key after the time
	// If not, select last
	if ( iter == m_keyframes.end() )
	{
		keyFrame2 = *(--m_keyframes.end());
		t2 = fAnimationLength;
	}
	else
	{
		keyFrame2 = *iter;
		t2 = keyFrame2.m_time;
	}

	t1 = keyFrame1.m_time;

	// Check if there is only one key
	if (t1 == t2) 
		return 0.0;
	else
		return (fTime - t1) / (t2 - t1);

}

//
// getInterpolatedKeyframe
// Gets a keyframe which is interpolates at a given time index
//
Keyframe AnimationTrack::getInterpolatedKeyframe( float fTime )
{
	// Find the two keyframes bounded by our target time
	Keyframe key1, key2, resultKey;
	float t = getKeyframesAtTime( fTime, key1, key2 );

	// If t == 0, then we just use the first keyframe
	if ( t == 0 )
	{
		return key1;
	}
	// Otherwise, interpolate the keyframes by t
	else
	{
		// Linearlly interpolate the position
		resultKey.m_translation = key1.m_translation + ( key2.m_translation - key1.m_translation ) * t;

		// Linearlly interpolate the rotation
		resultKey.m_rotation = Quaternion::slerp( t, key1.m_rotation, key2.m_rotation );

		// Normalize the rotation
		resultKey.m_rotation.normalise();
	}

	// Return the resultant keyframe
	return resultKey;
}

//
// AnimationTrack::getMaximumKeyframeTime
//
float AnimationTrack::getMaximumKeyframeTime() const
{
	if ( m_keyframes.size() > 0 )
		return ( --m_keyframes.end() )->m_time;
	else
		return 0;
}

// -------------------------------------------------------


//
// OnLoadStream
//
bool AnimationTrack::OnLoadStream( kistream & istr )
{
	// Load the revelant member variables
	istr >> m_keyframes;

	return true;
}

//
// OnSaveStream
//
bool AnimationTrack::OnSaveStream( kostream & ostr ) const
{
	// Save the revelant member variables
	ostr << m_keyframes;

	return true;
}