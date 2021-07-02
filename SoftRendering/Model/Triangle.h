#pragma once
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4X4.h"

class Vertex
{
public:
	Vector3 WorldPos;			//世界坐标下的位置
	Vector3 WorldNormal;		//世界坐标下的法线
	Vector4 Position;			//视口变换后的坐标
	Vector4 FragPosLightSpace;  //光源空间下的坐标
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