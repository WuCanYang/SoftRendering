#pragma once
#include <math.h>
#include "Vector3.h"
#include "Matrix4X4.h"

class Quaternion
{
public:
	float w, x, y, z;

	Quaternion() :w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
	Quaternion(float _w, float _x, float _y, float _z) : w(_w), x(_x), y(_y), z(_z) {}

	void identity()
	{
		w = 1.0f;
		x = y = z = 0.0f;
	}

	inline float dot(const Quaternion& a)
	{
		return w * a.w + x * a.x + y * a.y + z * a.z;
	}

	inline Quaternion operator*(const Quaternion& a)
	{
		Quaternion result;

		result.w = w * a.w - x * a.x - y * a.y - z * a.z;
		result.x = w * a.x + x * a.w + z * a.y - y * a.z;
		result.y = w * a.y + y * a.w + x * a.z - z * a.x;
		result.z = w * a.z + z * a.w + y * a.x - x * a.y;
		return result;
	}

	inline Quaternion conjugate()
	{
		return Quaternion(w, -x, -y, -z);
	}

	inline Quaternion inverse()
	{
		return Quaternion(w, -x, -y, -z);
	}

	void RotateX(float theta)
	{
		float halfTheta = theta * 0.5f;

		w = cosf(halfTheta);
		x = sinf(halfTheta);
		y = 0.0f;
		z = 0.0f;
	}

	void RotateY(float theta)
	{
		float halfTheta = theta * 0.5f;

		w = cosf(halfTheta);
		x = 0.0f;
		y = sinf(halfTheta);
		z = 0.0f;
	}

	void RotateZ(float theta)
	{
		float halfTheta = theta * 0.5f;

		w = cosf(halfTheta);
		x = 0.0f;
		y = 0.0f;
		z = sinf(halfTheta);
	}

	void RotateAxis(Vector3& axis, float theta)
	{
		axis.Normalize();
		float halfTheta = theta * 0.5f;

		w = cosf(halfTheta);
		x = axis.x() * sinf(halfTheta);
		y = axis.y() * sinf(halfTheta);
		z = axis.z() * sinf(halfTheta);
	}

	void normalize()
	{
		float val = sqrtf(w * w + x * x + y * y + z * z);
		if (val == 0.0f) return;
		w /= val;
		x /= val;
		y /= val;
		z /= val;
	}

	Matrix4X4 toMatrix()
	{
		Matrix4X4 m;
		m.m11 = 1 - 2 * y * y - 2 * z * z;
		m.m12 = 2 * x * y + 2 * w * z;
		m.m13 = 2 * x * z - 2 * w * y;

		m.m21 = 2 * x * y - 2 * w * z;
		m.m22 = 1 - 2 * x * x - 2 * z * z;
		m.m23 = 2 * y * z + 2 * w * x;

		m.m31 = 2 * x * z + 2 * w * y;
		m.m32 = 2 * y * z - 2 * w * x;
		m.m33 = 1 - 2 * x * x - 2 * y * y;

		m.m44 = 1.0f;
		return m;
	}

};