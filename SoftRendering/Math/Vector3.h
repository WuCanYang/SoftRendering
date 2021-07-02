#pragma once
#include <math.h>

class Vector3
{
public:
	float _x, _y, _z;

	Vector3() :_x(0), _y(0), _z(0) {}
	Vector3(float a) :_x(a), _y(a), _z(a) {}
	Vector3(float a, float b, float c) : _x(a), _y(b), _z(c) {}
	Vector3(const Vector3& a) : _x(a._x), _y(a._y), _z(a._z) {}

	inline float x() const { return _x; }
	inline float y() const { return _y; }
	inline float z() const { return _z; }

	inline void Zero() { _x = 0; _y = 0; _z = 0; }

	inline Vector3 operator+(const Vector3& a)
	{
		return Vector3(_x + a._x, _y + a._y, _z + a._z);
	}

	inline Vector3 operator-(const Vector3& a)
	{
		return Vector3(_x - a._x, _y - a._y, _z - a._z);
	}

	inline Vector3 operator*(const float& a)
	{
		return Vector3(_x * a, _y * a, _z * a);
	}

	inline Vector3 operator/(const float& a)
	{
		if (a == 0.0f)
		{
			return Vector3(0.0f);
		}
		return Vector3(_x / a, _y / a, _z / a);
	}

	inline bool operator==(const Vector3& a) const
	{
		return _x == a._x && _y == a._y && _z == a._z;
	}

	inline bool operator!=(const Vector3& a) const
	{
		return _x != a._x || _y != a._y || _z != a._z;
	}

	inline float dot(const Vector3& a)
	{
		return _x * a._x + _y * a._y + _z * a._z;
	}

	inline Vector3 cross(const Vector3& a)
	{
		float val1 = _y * a._z - a._y * _z;
		float val2 = _z * a._x - a._z * _x;
		float val3 = _x * a._y - a._x * _y;
		return Vector3(val1, val2, val3);
	}

	inline void Normalize()
	{
		float val = sqrtf(_x * _x + _y * _y + _z * _z);
		if (val == 0.0f) return;
		_x /= val;
		_y /= val;
		_z /= val;
	}

	inline float length()
	{
		return sqrtf(powf(_x, 2) + powf(_y, 2) + powf(_z, 2));
	}
};