#pragma once
#include "../Math/Vector3.h"
#include "../Math/Matrix4X4.h"

class Camera
{
public:
	Vector3 Position;
	Vector3 Target;

	Camera() : Target(0.0f) {}
	Camera(const Vector3& p, const Vector3& t) : Position(p), Target(t) {}

	Matrix4X4 GetViewMatrix();
};