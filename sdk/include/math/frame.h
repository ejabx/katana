/*
	Katana Engine
	Copyright © 2004 Eric Bryant, Inc.

	File:		frame.h
	Author:		Eric Bryant

	Represents a frame of reference in 3D space (translation and rotation),
	as well as hierarchy information.
*/

#ifndef _FRAME_H
#define _FRAME_H

namespace Katana
{

///
/// Frame
///
class Frame
{
public:
	/// Constructor
	Frame()															{ m_isDirty = true; m_parent = NULL; }

	/// Sets the position
	void setPosition( const Point3 & pos )							{ m_position = pos; m_isDirty = true; }

	/// Sets the rotation
	void setRotation(const Quaternion & rot)						{ m_rotation = rot; m_isDirty = true; }

	/// Sets the transformation
	void setTransform(const Point3 & trans, const Quaternion & rot)	{ m_translation = trans; m_rotation = rot; m_isDirty = true; }

	/// Returns the local position
	const Point3& getPosition() const								{ return m_position; m_isDirty = true; }

	/// Returns the local rotation
	const Quaternion & getRotation() const							{ return m_rotation; m_isDirty = true; }

	/// Returns the world matrix
	const Matrix4 & getWorldMatrix() const							{ return m_worldViewMatrix; m_isDirty = true; }

	/// Sets the parent relationship
	void setParent( const Frame & frame )							{ m_parent = &frame; m_isDirty = true; }

	/// Gets the parent
	const Frame * getParent()										{ return m_parent; }

protected:
	Frame *				m_parent;			/// Our parent frame
	Point4				m_position;			/// Local Position (is concatenated into the world matrix)
	Quaternion			m_rotation;			/// Local Rotation (is concatenated into the world matrix)
	Matrix4				m_worldMatrix;		/// Matrix which is calculated using local transform and parent relationships.	
	bool				m_isDirty;			/// Flag used to determine whether the world matrix must be recalculated (usually due to translation or rotation of the object).
};

}; // Katana

#endif // _FRAME_H