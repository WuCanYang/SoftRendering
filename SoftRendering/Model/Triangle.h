#pragma once
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4X4.h"

class Vertex
{
public:
	Vector3 WorldPos;			//���������µ�λ��
	Vector3 WorldNormal;		//���������µķ���
	Vector4 Position;			//�ӿڱ任�������
	Vector4 FragPosLightSpace;  //��Դ�ռ��µ�����
	Vector2 TextureCoordinate;
};

class Triangle
{
public:
	Vertex va, vb, vc;

	Matrix4X4 view;
	Matrix4X4 projection;

	Triangle() {}
	Triangle(const Vertex& a, const Vertex& b, const Vertex& c) : va(a), vb(b), vc(c) {}
};