#pragma once

#include "Math/Vector3.h"
#include "Math/Matrix4X4.h"

class Light
{
public:
	Vector3 Position;
	Vector3 Direction;

	Vector3 Color;


	class Model* mesh; //���ڹ�Դ���ӻ���ʾ�� һ��cube


	Light() : Color(1.0f), mesh(nullptr) {}
	~Light()
	{
		if (mesh) delete mesh;
	}

	void update();	//�Թ�Դ��һЩ�˶�

	Matrix4X4 GetViewMatrix();  //���� ������Ӱ����������ͼ ����
	Matrix4X4 GetOrthoMatrix(float left, float right, float bottom, float top, float near, float far);
};