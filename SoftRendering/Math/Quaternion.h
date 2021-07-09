#pragma once
#include <math.h>
#include "Vector3.h"
#include "Matrix4X4.h"
#include "Model/Constant.h"

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

	inline float dot(const Quaternion& a) const
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

	static Quaternion slerp(const Quaternion& q0, const Quaternion& q1, float t)
	{
		if (t <= 0.0f) return q0;
		if (t >= 1.0f) return q1;

		float cosOmega = q0.dot(q1);

		float q1w = q1.w;
		float q1x = q1.x;
		float q1y = q1.y;
		float q1z = q1.z;
		if (cosOmega < 0.0f)
		{
			q1w = -q1w;
			q1x = -q1x;
			q1y = -q1y;
			q1z = -q1z;
			cosOmega = -cosOmega;
		}

		float k0, k1;
		if (cosOmega > 0.9999f)
		{
			k0 = 1.0f - t;
			k1 = t;
		}
		else
		{
			float sinOmega = sqrtf(1.0f - cosOmega * cosOmega);
			float Omega = atan2(sinOmega, cosOmega);
			float invSinOmega = 1.0f / sinOmega;

			k0 = sinf((1.0f - t) * Omega) * invSinOmega;
			k1 = sinf(t * Omega) * invSinOmega;
		}

		Quaternion result;
		result.w = k0 * q0.w + k1 * q1w;
		result.x = k0 * q0.x + k1 * q1x;
		result.y = k0 * q0.y + k1 * q1y;
		result.z = k0 * q0.z + k1 * q1z;
		return result;
	}

	Vector3 toEulerAngle()
	{
		float a = atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y));
		float b = asin(2 * (w * y - z * x));
		float c = atan2(2 * (w * z + x * y), 1 - 2 * (y * y + z * z));
		return Vector3(a / PI * 180.0f, b / PI * 180.0f, c / PI * 180.0f);
	}

};