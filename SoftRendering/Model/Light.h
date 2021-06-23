#pragma once

#include "../Math/Vector3.h"
#include "../Math/Matrix4X4.h"

class Light
{
public:
	Vector3 Position;
	Vector3 Direction;


	Matrix4X4 GetViewMatrix();  //���� ������Ӱ����������ͼ ����
};