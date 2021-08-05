#pragma once
#include <math.h>
#include "Vector3.h"

class Vector4
{
public:
	float _x, _y, _z, _w;

	Vector4() :_x(0), _y(0), _z(0), _w(0) {}
	Vector4(float a) :_x(a), _y(a), _z(a), _w(a) {}
	Vector4(float a, float b, float c, float d) : _x(a), _y(b), _z(c), _w(d) {}
	Vector4(const Vector4& a) : _x(a._x), _y(a._y), _z(a._z), _w(a._w) {}
	Vector4(const Vector3& a) : _x(a._x), _y(a._y), _z(a._z), _w(1.0f) {}

	inline float x() const { return _x; }
	inline float y() const { return _y; }
	inline float z() const { return _z; }
	inline float w() const { return _w; }

	inline void Zero() { _x = 0; _y = 0; _z = 0; _w = 0; }

	inline bool operator==(const Vector4& a) const
	{
		return _x == a._x && _y == a._y && _z == a._z && _w == a._w;
	}

	inline bool operator!=(const Vector4& a) const
	{
		return _x != a._x || _y != a._y || _z != a._z || _w != a._w;
	}

	inline Vector4 operator*(const float& a) const
	{
		return Vector4(_x * a, _y * a, _z * a, _w * a);
	}

	inline Vector4 operator/(const float& a) const
	{
		return Vector4(_x / a, _y / a, _z / a, _w / a);
	}

	inline Vector4 operator+(const Vector4& a) const
	{
		return Vector4(_x + a._x, _y + a._y, _z + a._z, _w + a._w);
	}

	inline Vector4 operator-(const Vector4& a) const
	{
		return Vector4(_x - a._x, _y - a._y, _z - a._z, _w - a._w);
	}

	inline Vector4& operator+=(const Vector4& a)
	{
		_x += a._x;
		_y += a._y;
		_z += a._z;
		_w += a._w;
		return *this;
	}

	inline Vector4& operator/=(const float& a)
	{
		_x /= a;
		_y /= a;
		_z /= a;
		_w /= a;
		return *this;
	}

	inline double length() const
	{
		return sqrt(pow(_x, 2) + pow(_y, 2) + pow(_z, 2) + pow(_w, 2));
	}
};

inline Vector4 operator*(const float& a, const Vector4& b)
{
	return Vector4(b._x * a, b._y * a, b._z * a, b._w * a);
}