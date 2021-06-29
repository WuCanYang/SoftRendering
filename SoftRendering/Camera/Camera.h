#pragma once
#include "Math/Vector3.h"
#include "Math/Matrix4X4.h"

class Camera
{
public:
	Vector3 Position;
	Vector3 Target;

	float fovy;
	float _near, _far;

	Camera() : Target(0.0f), fovy(50.0f), _near(0.1f), _far(100.0f) {}
	Camera(const Vector3& p, const Vector3& t) : Position(p), Target(t), fovy(50.0f), _near(0.1f), _far(100.0f) {}

	Matrix4X4 GetViewMatrix();
	Matrix4X4 GetPerspectiveMatrix();
};