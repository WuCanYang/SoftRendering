#pragma once
#include <math.h>

class Vector2
{
public:
	float _x, _y;

	Vector2() :_x(0), _y(0) {}
	Vector2(float a):_x(a), _y(a){}
	Vector2(float a, float b) : _x(a), _y(b) {}
	Vector2(const Vector2& a) :_x(a._x), _y(a._y) {}

	inline float x() const { return _x;}
	inline float y() const { return _y;}

	inline void Zero() { _x = 0; _y = 0; }

	inline Vector2 operator+(const Vector2& a)
	{
		return Vector2(_x + a._x, _y + a._y);
	}

	inline Vector2 operator-(const Vector2& a)
	{
		return Vector2(_x - a._x, _y - a._y);
	}

	inline Vector2& operator+=(const Vector2& a)
	{
		_x += a._x;
		_y += a._y;
		return *this;
	}

	inline Vector2& operator/=(const float& a)
	{
		_x /= a;
		_y /= a;
		return *this;
	}

	inline bool operator==(const Vector2& a)
	{
		return _x == a._x && _y == a._y;
	}

	inline bool operator!=(const Vector2& a)
	{
		return _x != a._x || _y != a._y;
	}

	inline Vector2 operator*(const float& a)
	{
		return Vector2(_x * a, _y * a);
	}

	inline Vector2 operator/(const float& a)
	{
		return Vector2(_x / a, _y / a);
	}

	inline float dot(const Vector2& a)
	{
		return _x * a._x + _y * a._y;
	}

	inline void Normalize()
	{
		float val = sqrtf(_x * _x + _y * _y);
		if (val == 0.0f) return;
		_x /= val;
		_y /= val;
	}

	inline float length()
	{
		return sqrtf(powf(_x, 2) + powf(_y, 2));
	}
};

inline Vector2 operator*(const float& a, const Vector2& b)
{
	return Vector2(b._x * a, b._y * a);
}