#pragma once

#include "../Math/Vector3.h"
#include "../Math/Matrix4X4.h"

class Light
{
public:
	Vector3 Position;
	Vector3 Direction;

	Vector3 Color;

	Light() : Color(1.0f) {}

	void update();	//对光源做一些运动

	Matrix4X4 GetViewMatrix();  //计算 用于阴影计算的深度贴图 所需
	Matrix4X4 GetOrthoMatrix(float left, float right, float bottom, float top, float near, float far);
};