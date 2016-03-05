/*
	Katana Engine
	Copyright © 2004 Eric Bryant, Inc.

	File:		animationtrack.h
	Author:		Eric Bryant

	An animation track is a collection of keyframes. The track interpolates
	the keyframes at a given time to determine the resultant position and
	rotation that constitute the animation.
*/

#ifndef _ANIMATIONTRACK_H
#define _ANIMATIONTRACK_H

namespace Katana
{

//
// AnimationTrack
//
class AnimationTrack
	: public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;
	KDECLARE_STREAM( AnimationTrack )

	/// Default constructor
	AnimationTrack();

	/// Adds a keyframe to the collection
	void addKeyframe( const Keyframe & keyframe );

	/// Clears all the keyframes
	void clearKeyframes();

	/// Returns a keyframe at a particular index
	Keyframe * getKeyframe( unsigned int keyIndex );

	/// Gets two keyframes which fall between the given time index. Returns the parametric value
	/// indicating how far along the time is between the two keyframes, e.g., 0 if exactly at the
	/// first keyframe, 0.5 if half way, etc.
	float getKeyframesAtTime( float fTime, Keyframe & keyFrame1, Keyframe & keyFrame2 );

	/// Gets a keyframe which is interpolates at a given time index
	Keyframe getInterpolatedKeyframe( float fTime );

	/// Gets the maximum keyframe time for this animation track
	float getMaximumKeyframeTime() const;


private:

	/// The collection of keyframes
	vector<Keyframe> m_keyframes;

};

KIMPLEMENT_SCRIPT( AnimationTrack );
KIMPLEMENT_STREAM( AnimationTrack );

//
// Inline
//

//
// AnimationTrack::constructor
//
inline AnimationTrack::AnimationTrack()
{}

//
// AnimationTrack::addKeyframe
//
inline void AnimationTrack::addKeyframe( const Keyframe & keyframe )
{ m_keyframes.push_back( keyframe ); }

inline void AnimationTrack::clearKeyframes()
{ m_keyframes.clear(); }

//
// AnimationTrack::getKeyframe
//
inline Keyframe * AnimationTrack::getKeyframe( unsigned int keyIndex )
{
	if ( keyIndex < m_keyframes.size() )
		return &m_keyframes[keyIndex];
	else
		return NULL;
}

}; // Katana

#endif // _ANIMATIONTRACK_H