#pragma once
#include "Vector3.h"
#include <math.h>

class Vector3d
{
public:
	double _x, _y, _z;

	Vector3d() :_x(0), _y(0), _z(0) {}
	Vector3d(double a) :_x(a), _y(a), _z(a) {}
	Vector3d(double a, double b, double c) : _x(a), _y(b), _z(c) {}
	Vector3d(const Vector3d& a) : _x(a._x), _y(a._y), _z(a._z) {}
	Vector3d(const Vector3& a) : _x(a._x), _y(a._y), _z(a._z) {}

	inline double x() const { return _x; }
	inline double y() const { return _y; }
	inline double z() const { return _z; }

	inline void Zero() { _x = 0; _y = 0; _z = 0; }

	inline Vector3d operator+(const Vector3d& a) const
	{
		return Vector3d(_x + a._x, _y + a._y, _z + a._z);
	}

	inline Vector3d operator-(const Vector3d& a) const
	{
		return Vector3d(_x - a._x, _y - a._y, _z - a._z);
	}

	inline Vector3d operator*(const double& a)
	{
		return Vector3d(_x * a, _y * a, _z * a);
	}

	inline Vector3d operator/(const double& a)
	{
		if (a == 0.0)
		{
			return Vector3d(0.0);
		}
		return Vector3d(_x / a, _y / a, _z / a);
	}

	inline Vector3d& operator-=(const Vector3d& a)
	{
		_x -= a._x;
		_y -= a._y;
		_z -= a._z;
		return *this;
	}

	inline Vector3d& operator+=(const Vector3d& a)
	{
		_x += a._x;
		_y += a._y;
		_z += a._z;
		return *this;
	}

	inline Vector3d& operator*=(const double& a)
	{
		_x *= a;
		_y *= a;
		_z *= a;
		return *this;
	}

	inline bool operator==(const Vector3d& a) const
	{
		return _x == a._x && _y == a._y && _z == a._z;
	}

	inline bool operator!=(const Vector3d& a) const
	{
		return _x != a._x || _y != a._y || _z != a._z;
	}

	inline double operator[](const int idx) const
	{
		if (idx == 0) return _x;
		else if (idx == 1) return _y;
		else return _z;
	}

	inline double dot(const Vector3d& a) const
	{
		return _x * a._x + _y * a._y + _z * a._z;
	}

	inline Vector3d cross(const Vector3d& a) const
	{
		double val1 = _y * a._z - a._y * _z;
		double val2 = _z * a._x - a._z * _x;
		double val3 = _x * a._y - a._x * _y;
		return Vector3d(val1, val2, val3);
	}

	inline void Normalize()
	{
		double val = sqrt(_x * _x + _y * _y + _z * _z);
		if (val == 0.0) return;
		_x /= val;
		_y /= val;
		_z /= val;
	}

	inline double length() const
	{
		return sqrt(pow(_x, 2) + pow(_y, 2) + pow(_z, 2));
	}
};

inline Vector3d operator*(const double& a, const Vector3d& b)
{
	return Vector3d(b._x * a, b._y * a, b._z * a);
}