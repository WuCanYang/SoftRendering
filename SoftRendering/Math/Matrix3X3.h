#pragma once
#include "Vector3.h"
#include "Matrix4X4.h"

class Matrix3X3
{
public:
	float m11, m12, m13;
	float m21, m22, m23;
	float m31, m32, m33;

	Matrix3X3()
	{
		m11 = m12 = m13 = 0.0f;
		m21 = m22 = m23 = 0.0f;
		m31 = m32 = m33 = 0.0f;
	}

	Matrix3X3(const Matrix4X4& m)
	{
		m11 = m.m11;  m12 = m.m12;  m13 = m.m13;
		m21 = m.m21;  m22 = m.m22;  m23 = m.m23;
		m31 = m.m31;  m32 = m.m32;  m33 = m.m33;
	}

	inline Matrix3X3 operator*(const Matrix3X3& a)
	{
		Matrix3X3 m;
		m.m11 = m11 * a.m11 + m12 * a.m21 + m13 * a.m31;
		m.m12 = m11 * a.m12 + m12 * a.m22 + m13 * a.m32;
		m.m13 = m11 * a.m13 + m12 * a.m23 + m13 * a.m33;

		m.m21 = m21 * a.m11 + m22 * a.m21 + m23 * a.m31;
		m.m22 = m21 * a.m12 + m22 * a.m22 + m23 * a.m32;
		m.m23 = m21 * a.m13 + m22 * a.m23 + m23 * a.m33;

		m.m31 = m31 * a.m11 + m32 * a.m21 + m33 * a.m31;
		m.m32 = m31 * a.m12 + m32 * a.m22 + m33 * a.m32;
		m.m33 = m31 * a.m13 + m32 * a.m23 + m33 * a.m33;

		return m;
	}

	inline Vector3 operator*(const Vector3& a)
	{
		float x = m11 * a.x() + m12 * a.y() + m13 * a.z();
		float y = m21 * a.x() + m22 * a.y() + m23 * a.z();
		float z = m31 * a.x() + m32 * a.y() + m33 * a.z();
		return Vector3(x, y, z);
	}

	inline Matrix3X3 operator/(const float& a)
	{
		if (a == 0.0f) return Matrix3X3();

		Matrix3X3 m;
		m.m11 = m11 / a;
		m.m12 = m12 / a;
		m.m13 = m13 / a;

		m.m21 = m21 / a;
		m.m22 = m22 / a;
		m.m23 = m23 / a;

		m.m31 = m31 / a;
		m.m32 = m32 / a;
		m.m33 = m33 / a;
		
		return m;
	}

	Matrix3X3 inverse()
	{
		float Determinant = m11 * m22 * m33 + m21 * m32 * m13 + m12 * m23 * m31
			- m13 * m22 * m31 - m12 * m21 * m33 - m11 * m23 * m32;

		Matrix3X3 m;
		m.m11 = (m22 * m33 - m23 * m32);
		m.m12 = -(m21 * m33 - m23 * m31);
		m.m13 = (m21 * m32 - m22 * m31);

		m.m21 = -(m12 * m33 - m13 * m32);
		m.m22 = (m11 * m33 - m13 * m31);
		m.m23 = -(m11 * m32 - m12 * m31);

		m.m31 = (m12 * m23 - m13 * m22);
		m.m32 = -(m11 * m23 - m13 * m21);
		m.m33 = (m11 * m22 - m12 * m21);

		return m / Determinant;
	}

	Matrix3X3 transpose()
	{
		Matrix3X3 m;
		m.m11 = m11;
		m.m12 = m21;
		m.m13 = m31;

		m.m21 = m12;
		m.m22 = m22;
		m.m23 = m32;

		m.m31 = m13;
		m.m32 = m23;
		m.m33 = m33;
		return m;
	}
};
