#pragma once
#include "Vector4.h"

class Matrix4X4
{
	float m11, m12, m13, m14;
	float m21, m22, m23, m24;
	float m31, m32, m33, m34;
	float m41, m42, m43, m44;

public:

	

	inline Matrix4X4 operator*(const Matrix4X4& a)
	{
		Matrix4X4 m;
		m.m11 = m11 * a.m11 + m12 * a.m21 + m13 * a.m31 + m14 * a.m41;
		m.m12 = m11 * a.m12 + m12 * a.m22 + m13 * a.m32 + m14 * a.m42;
		m.m13 = m11 * a.m13 + m12 * a.m23 + m13 * a.m33 + m14 * a.m43;
		m.m14 = m11 * a.m14 + m12 * a.m24 + m13 * a.m34 + m14 * a.m44;

		m.m21 = m21 * a.m11 + m22 * a.m21 + m23 * a.m31 + m24 * a.m41;
		m.m22 = m21 * a.m12 + m22 * a.m22 + m23 * a.m32 + m24 * a.m42;
		m.m23 = m21 * a.m13 + m22 * a.m23 + m23 * a.m33 + m24 * a.m43;
		m.m24 = m21 * a.m14 + m22 * a.m24 + m23 * a.m34 + m24 * a.m44;

		m.m31 = m31 * a.m11 + m32 * a.m21 + m33 * a.m31 + m34 * a.m41;
		m.m32 = m31 * a.m12 + m32 * a.m22 + m33 * a.m32 + m34 * a.m42;
		m.m33 = m31 * a.m13 + m32 * a.m23 + m33 * a.m33 + m34 * a.m43;
		m.m34 = m31 * a.m14 + m32 * a.m24 + m33 * a.m34 + m34 * a.m44;

		m.m41 = m41 * a.m11 + m42 * a.m21 + m43 * a.m31 + m44 * a.m41;
		m.m42 = m41 * a.m12 + m42 * a.m22 + m43 * a.m32 + m44 * a.m42;
		m.m43 = m41 * a.m13 + m42 * a.m23 + m43 * a.m33 + m44 * a.m43;
		m.m44 = m41 * a.m14 + m42 * a.m24 + m43 * a.m34 + m44 * a.m44;

		return m;
	}

	inline Vector4 operator*(const Vector4& a)
	{
		float x = m11 * a.x() + m12 * a.y() + m13 * a.z() + m14 * a.w();
		float y = m21 * a.x() + m22 * a.y() + m23 * a.z() + m24 * a.w();
		float z = m31 * a.x() + m32 * a.y() + m33 * a.z() + m34 * a.w();
		float w = m41 * a.x() + m42 * a.y() + m43 * a.z() + m44 * a.w();
		return Vector4(x, y, z, w);
	}
};