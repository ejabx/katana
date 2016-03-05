/*
	Katana Engine
	Copyright © 2001 Eric Bryant, Inc.

	File:		matrix.h
	Author:		Eric Bryant

	Represents a matrix
*/

#ifndef _MATRIX_H
#define _MATRIX_H


namespace Katana
{

///
/// Matrix4
/// Represents a 4x4 Homogeneous Matrix
///
class Matrix4
{
public:
	/// Constructor
	Matrix4();
	Matrix4 (
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33);

	/// Copy Constructor
    Matrix4 (const Matrix4& mat);

	/// Sets the matrix as the identity matrix
	void setIdentity();

	/// Zeros out the matrix
	void setZero();

	/// Transposes the matrix
	Matrix4 & transpose();

	/// Inverse the matrix
	Matrix4 & inverse();

	/// Constructs a rotation matrix
	void makeRotation( float angle, const Point3 & axis );

	/// Common Arithmetic
	Matrix4 & operator*=(const Matrix4 & m);
	Matrix4	  operator*(const Matrix4 & m) const;

	/// Set operator which sets all the row in the matrix at one time
	void set(float m00, float m01, float m02, float m03,
			 float m10, float m11, float m12, float m13,
			 float m20, float m21, float m22, float m23,
			 float m30, float m31, float m32, float m33);

public:
	/// Matrix elements. We've organized them into 16 byte aligned
	/// quantities for readibility. Use the set/get functions to 
	/// retrieve them as raw indices.
	union
	{
        float m[4][4];		// As an multidimension array
		float s[16];		// As a single dimension array
        struct				// As matrix variables
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
		struct 				// As a coordinate frame
		{
			Point4 right;
			Point4 up;
			Point4 at;
			Point4 pos;
		};
	};
};

//
// set
//
inline void Matrix4::set(float _m00, float _m01, float _m02, float _m03,
						 float _m10, float _m11, float _m12, float _m13,
						 float _m20, float _m21, float _m22, float _m23,
						 float _m30, float _m31, float _m32, float _m33)
{
	m00 = _m00; m01 = _m01; m02 = _m02; m03 = _m03;
	m10 = _m10; m11 = _m11; m12 = _m12; m13 = _m13;
	m20 = _m20; m21 = _m21; m22 = _m22; m23 = _m23;
	m30 = _m30; m31 = _m31; m32 = _m32; m33 = _m33;
}

}; // Katana

#endif // _MATRIX_H