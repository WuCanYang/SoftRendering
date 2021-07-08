#pragma once
#include "Vector4.h"
#include <iostream>
class Matrix4X4
{
public:
	float m11, m12, m13, m14;
	float m21, m22, m23, m24;
	float m31, m32, m33, m34;
	float m41, m42, m43, m44;

	Matrix4X4()
	{
		m11 = m12 = m13 = m14 = 0.0f;
		m21 = m22 = m23 = m24 = 0.0f;
		m31 = m32 = m33 = m34 = 0.0f;
		m41 = m42 = m43 = m44 = 0.0f;
	}

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

	inline Matrix4X4 operator/(const float& a)
	{
		if (a == 0.0f) return Matrix4X4();

		Matrix4X4 m;
		m.m11 = m11 / a;
		m.m12 = m12 / a;
		m.m13 = m13 / a;
		m.m14 = m14 / a;

		m.m21 = m21 / a;
		m.m22 = m22 / a;
		m.m23 = m23 / a;
		m.m24 = m24 / a;

		m.m31 = m31 / a;
		m.m32 = m32 / a;
		m.m33 = m33 / a;
		m.m34 = m34 / a;

		m.m41 = m41 / a;
		m.m42 = m42 / a;
		m.m43 = m43 / a;
		m.m44 = m44 / a;

		return m;
	}

	/*Matrix4X4 inverse()
	{
		float SubFactor00 = m33 * m44 - m43 * m34;
		float SubFactor01 = m32 * m44 - m42 * m34;
		float SubFactor02 = m32 * m43 - m42 * m33;
		float SubFactor03 = m31 * m44 - m41 * m34;
		float SubFactor04 = m31 * m43 - m41 * m33;
		float SubFactor05 = m31 * m42 - m41 * m32;
		float SubFactor06 = m23 * m44 - m43 * m24;
		float SubFactor07 = m22 * m44 - m42 * m24;
		float SubFactor08 = m22 * m43 - m42 * m23;
		float SubFactor09 = m21 * m44 - m41 * m24;
		float SubFactor10 = m21 * m43 - m41 * m23;
		float SubFactor11 = m21 * m42 - m41 * m22;
		float SubFactor12 = m23 * m34 - m33 * m24;
		float SubFactor13 = m22 * m34 - m32 * m24;
		float SubFactor14 = m22 * m33 - m32 * m23;
		float SubFactor15 = m21 * m34 - m31 * m24;
		float SubFactor16 = m21 * m33 - m31 * m23;
		float SubFactor17 = m21 * m32 - m31 * m22;

		Matrix4X4 Inverse;

		Inverse.m11 = +(m22 * SubFactor00 - m23 * SubFactor01 + m24 * SubFactor02);
		Inverse.m12 = -(m21 * SubFactor00 - m23 * SubFactor03 + m24 * SubFactor04);
		Inverse.m13 = +(m21 * SubFactor01 - m22 * SubFactor03 + m24 * SubFactor05);
		Inverse.m14 = -(m21 * SubFactor02 - m22 * SubFactor04 + m23 * SubFactor05);

		Inverse.m21 = -(m12 * SubFactor00 - m13 * SubFactor01 + m14 * SubFactor02);
		Inverse.m22 = +(m11 * SubFactor00 - m13 * SubFactor03 + m14 * SubFactor04);
		Inverse.m23 = -(m11 * SubFactor01 - m12 * SubFactor03 + m14 * SubFactor05);
		Inverse.m24 = +(m11 * SubFactor02 - m12 * SubFactor04 + m13 * SubFactor05);

		Inverse.m31 = +(m12 * SubFactor06 - m13 * SubFactor07 + m14 * SubFactor08);
		Inverse.m32 = -(m11 * SubFactor06 - m13 * SubFactor09 + m14 * SubFactor10);
		Inverse.m33 = +(m11 * SubFactor07 - m12 * SubFactor09 + m14 * SubFactor11);
		Inverse.m34 = -(m11 * SubFactor08 - m12 * SubFactor10 + m13 * SubFactor11);

		Inverse.m41 = -(m12 * SubFactor12 - m13 * SubFactor13 + m14 * SubFactor14);
		Inverse.m42 = +(m11 * SubFactor12 - m13 * SubFactor15 + m14 * SubFactor16);
		Inverse.m43 = -(m11 * SubFactor13 - m12 * SubFactor15 + m14 * SubFactor17);
		Inverse.m44 = +(m11 * SubFactor14 - m12 * SubFactor16 + m13 * SubFactor17);

		float Determinant =
			+m11 * Inverse.m11
			+ m12 * Inverse.m12
			+ m13 * Inverse.m13
			+ m14 * Inverse.m14;

		Inverse = Inverse / Determinant;

		return Inverse;
	}*/

	/*Matrix4X4 inverse()
	{
		float SubFactor00 = m33 * m44 - m34 * m43;
		float SubFactor01 = m23 * m44 - m24 * m43;
		float SubFactor02 = m23 * m34 - m24 * m33;
		float SubFactor03 = m13 * m44 - m14 * m43;
		float SubFactor04 = m13 * m34 - m14 * m33;
		float SubFactor05 = m13 * m24 - m14 * m23;
		float SubFactor06 = m32 * m44 - m34 * m42;
		float SubFactor07 = m22 * m44 - m24 * m42;
		float SubFactor08 = m22 * m34 - m24 * m32;
		float SubFactor09 = m12 * m44 - m14 * m42;
		float SubFactor10 = m12 * m34 - m14 * m32;
		float SubFactor11 = m12 * m24 - m14 * m22;
		float SubFactor12 = m32 * m43 - m33 * m42;
		float SubFactor13 = m22 * m43 - m23 * m42;
		float SubFactor14 = m22 * m33 - m23 * m32;
		float SubFactor15 = m12 * m43 - m13 * m42;
		float SubFactor16 = m12 * m33 - m13 * m32;
		float SubFactor17 = m12 * m23 - m13 * m22;

		Matrix4X4 Inverse;

		Inverse.m11 = +(m22 * SubFactor00 - m32 * SubFactor01 + m42 * SubFactor02);
		Inverse.m21 = -(m12 * SubFactor00 - m32 * SubFactor03 + m42 * SubFactor04);
		Inverse.m31 = +(m12 * SubFactor01 - m22 * SubFactor03 + m42 * SubFactor05);
		Inverse.m41 = -(m12 * SubFactor02 - m22 * SubFactor04 + m32 * SubFactor05);

		Inverse.m12 = -(m21 * SubFactor00 - m31 * SubFactor01 + m41 * SubFactor02);
		Inverse.m22 = +(m11 * SubFactor00 - m31 * SubFactor03 + m41 * SubFactor04);
		Inverse.m32 = -(m11 * SubFactor01 - m21 * SubFactor03 + m41 * SubFactor05);
		Inverse.m42 = +(m11 * SubFactor02 - m21 * SubFactor04 + m31 * SubFactor05);

		Inverse.m13 = +(m21 * SubFactor06 - m31 * SubFactor07 + m41 * SubFactor08);
		Inverse.m23 = -(m11 * SubFactor06 - m31 * SubFactor09 + m41 * SubFactor10);
		Inverse.m33 = +(m11 * SubFactor07 - m21 * SubFactor09 + m41 * SubFactor11);
		Inverse.m43 = -(m11 * SubFactor08 - m21 * SubFactor10 + m31 * SubFactor11);

		Inverse.m14 = -(m21 * SubFactor12 - m31 * SubFactor13 + m41 * SubFactor14);
		Inverse.m24 = +(m11 * SubFactor12 - m31 * SubFactor15 + m41 * SubFactor16);
		Inverse.m34 = -(m11 * SubFactor13 - m21 * SubFactor15 + m41 * SubFactor17);
		Inverse.m44 = +(m11 * SubFactor14 - m21 * SubFactor16 + m31 * SubFactor17);

		float Determinant =
			+m11 * Inverse.m11
			+ m21 * Inverse.m21
			+ m31 * Inverse.m31
			+ m41 * Inverse.m41;


		Inverse = Inverse / Determinant;

		return Inverse;
	}*/

};