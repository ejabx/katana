/*
	Katana Engine
	Copyright © 2004 Eric Bryant, Inc.

	File:		keyframe.h
	Author:		Eric Bryant

	Represents a transform which is executed at a specific time
*/

#ifndef _KEYFRAME_H
#define _KEYFRAME_H

namespace Katana
{

//
// KeyFrame
//
struct Keyframe
{
	KDECLARE_SCRIPT;

	/// Default constructor
	Keyframe();

	/// Constructor which creates a blank keyframe at a particular time
	Keyframe( float time );

	/// Constructor which takes a time and position (rotation is identity)
	Keyframe( float time, const Point3 & translation );

	/// Constructor which takes a rotation (position is ZERO, remember keyframes are relative to
	/// the target's transform, so it would stay in place).
	Keyframe( float time, const Quaternion & rotation );

	/// Constructor which takes a time, position and rotation
	Keyframe( float time, const Point3 & translation, const Quaternion & rotation );

public:

	float		m_time;				/// Time at which this keyframe is active
	Point3		m_translation;		/// Positional component of this keyframe
	Quaternion	m_rotation;			/// Rotational component of this keyframe

};

KIMPLEMENT_SCRIPT( Keyframe );

//
// Inline
//

//
// Keyframe::constructors
//
inline Keyframe::Keyframe()
	: m_time( 0 )
{
}

inline Keyframe::Keyframe( float time )
	: m_time( time )
{}

inline Keyframe::Keyframe( float time, const Point3 & translation )
	: m_time( time )
	, m_translation( translation )
{}

inline Keyframe::Keyframe( float time, const Quaternion & rotation )
	: m_time( time )
	, m_rotation( rotation )
{}

inline Keyframe::Keyframe( float time, const Point3 & translation, const Quaternion & rotation )
	: m_time( time )
	, m_translation( translation )
	, m_rotation( rotation )
{}

} // Katana

#endif // _KEYFRAME_H