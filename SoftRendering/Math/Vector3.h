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

	inline Vector3 operator+(const Vector3& a) const
	{
		return Vector3(_x + a._x, _y + a._y, _z + a._z);
	}

	inline Vector3 operator-(const Vector3& a) const
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

	inline Vector3& operator-=(const Vector3& a)
	{
		_x -= a._x;
		_y -= a._y;
		_z -= a._z;
		return *this;
	}

	inline Vector3& operator+=(const Vector3& a)
	{
		_x += a._x;
		_y += a._y;
		_z += a._z;
		return *this;
	}

	inline Vector3& operator*=(const float& a)
	{
		_x *= a;
		_y *= a;
		_z *= a;
		return *this;
	}

	inline Vector3& operator/=(const float& a)
	{
		_x /= a;
		_y /= a;
		_z /= a;
		return *this;
	}

	inline bool operator==(const Vector3& a) const
	{
		return _x == a._x && _y == a._y && _z == a._z;
	}

	inline bool operator!=(const Vector3& a) const
	{
		return _x != a._x || _y != a._y || _z != a._z;
	}

	inline float operator[](const int idx) const
	{
		if (idx == 0) return _x;
		else if (idx == 1) return _y;
		else return _z;
	}

	inline float dot(const Vector3& a) const
	{
		return _x * a._x + _y * a._y + _z * a._z;
	}

	inline Vector3 cross(const Vector3& a) const
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

	inline float length() const
	{
		return sqrtf(powf(_x, 2) + powf(_y, 2) + powf(_z, 2));
	}
};

inline Vector3 operator*(const float& a, const Vector3& b)
{
	return Vector3(b._x * a, b._y * a, b._z * a);
}