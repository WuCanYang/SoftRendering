#pragma once

#include "../Math/Vector3.h"
#include "../Math/Matrix4X4.h"

class Light
{
public:
	Vector3 Position;
	Vector3 Direction;


	Matrix4X4 GetViewMatrix();  //计算 用于阴影计算的深度贴图 所需
};