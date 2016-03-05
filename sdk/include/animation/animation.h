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

#ifndef _ANIMATION_H
#define _ANIMATION_H

namespace Katana
{

// Forward Declarations
class AnimationTrack;
struct Keyframe;

///
/// Animation
///
class Animation
	: public Streamable
{
	KDECLARE_RTTI;
	KDECLARE_SCRIPT;
	KDECLARE_STREAM(Animation);

public:
	/// Default constructor
	Animation();

	/// Constructor which takes a track as a parameter
	Animation( shared_ptr<AnimationTrack> defaultTrack );

	/// Constructor which takes an existing animation. The animation tracks
	/// will be shared by both animations
	Animation( shared_ptr<Animation> animation );

	/// Adds an additional track
	void addTrack( shared_ptr<AnimationTrack> animationTrack );

	/// Clears all animation tracks
	void clearTracks();

	/// Gets a particular animation track
	shared_ptr<AnimationTrack> getTrack( unsigned int trackIndex );

	/// Scales the animation time to execute slower or faster
	void setScaleTime( float scale );

	/// Returns the scaled time factor
	float getScaleTime() const;

	/// Sets whether this animation will loop when it's at the end
	void setLooping( bool loop );

	/// Gets whether this animation will loop when it's at the end
	bool getLooping() const;

	/// Gets the total animation length. This is calculated by the maximum keyframe time
	/// in all the animation tracks.
	float getAnimationLength() const;

	/// Sets whether this animation is enabled
	void setEnabled( bool enabled );

	/// Gets whether this animation is enabled
	bool getEnabled() const;

	/// Sets the current animation time
	void setAnimationTime( float fAnimTime );

	/// Gets the current animation time
	float getAnimationTime() const;

	/// Advances the animation time (or rewinds if the deltaTime is negative).
	/// This will adjust the animation time and apply the animation to the
	/// inputs (position and rotation). The weighting is used to blend different animations.
	/// This function will return FALSE if there are no animation tracks
	/// or we are at the end of the animation.
	bool advance( float deltaTime, Point3 & position, Quaternion & rotation, float fWeight = 1.f );

private:

	/// We map animat
	/// Collection of animation tracks
	vector< shared_ptr<AnimationTrack> > m_animationTracks;

	/// When stepping the animation in time, we scale it by this amount. The default is 1.
	float m_scaleDeltaTime;

	/// When stepping the animation, this is the overall time the animation has been playing
	float m_currentAnimationTime;

	/// This is the maximum keyframed time for all the animation tracks. This will determine
	/// whether the animation loops or not.
	float m_animationLength;

	/// Flags whether this animation will loop when it has reached the final keyframe
	/// The default is false.
	bool m_canLoop;

	/// Enable or disable this animation
	bool m_enabled;

};

KIMPLEMENT_SCRIPT( Animation );
KIMPLEMENT_STREAM( Animation );

//
// Inline
//

//
// Animation::constructors
inline Animation::Animation()
	: m_scaleDeltaTime( 1 )
	, m_currentAnimationTime( 0 )
	, m_animationLength( 0 )
	, m_canLoop( false )
	, m_enabled( true )
{}

inline Animation::Animation( shared_ptr<AnimationTrack> defaultTrack )
	: m_scaleDeltaTime( 1 )
	, m_currentAnimationTime( 0 )
	, m_animationLength( 0 )
	, m_canLoop( false )
	, m_enabled( true )
{ 
	addTrack( defaultTrack );
}

//
// Animation::clearTracks
//
inline void Animation::clearTracks()
{ m_animationTracks.clear(); }

//
// Animation::getTrack
//
inline shared_ptr<AnimationTrack> Animation::getTrack( unsigned int trackIndex )
{
	if ( trackIndex < m_animationTracks.size() )
		return m_animationTracks[trackIndex];
	else
		return shared_ptr<AnimationTrack>();
}

//
// Animation::setScaleTime
//
inline void Animation::setScaleTime( float scale )
{
	m_scaleDeltaTime = scale;
}

//
// Animation::getScaleTime
//
inline float Animation::getScaleTime() const
{
	return m_scaleDeltaTime;
}

//
// Animation::setLooping
//
inline void Animation::setLooping( bool loop )
{
	m_canLoop = loop;
}

//
// Animation::getLooping
//
inline bool Animation::getLooping() const
{
	return m_canLoop;
}

//
// Animation::getAnimationLength
//
inline float Animation::getAnimationLength() const
{
	return m_animationLength;
}

//
// Animation::setEnable
//
inline void Animation::setEnabled( bool enabled )
{
	m_enabled = enabled;
}

//
// Animation::getEnable
//
inline bool Animation::getEnabled() const
{
	return m_enabled;
}

//
// Animation::setAnimationTime
//
inline void Animation::setAnimationTime( float fAnimTime )
{
	m_currentAnimationTime = fAnimTime;
}

//
// Animation::getAnimationTime
//
inline float Animation::getAnimationTime() const
{
	return m_currentAnimationTime;
}

} // Katana

#endif // _ANIMATION_H