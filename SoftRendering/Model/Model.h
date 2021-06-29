#pragma once

#include "Triangle.h"
#include "Texture.h"
#include "Math/Matrix4X4.h"
#include <vector>

struct Index3I
{
	int index[3];
};

class Model
{
private:

	Matrix4X4 ScaleMatrix();
	Matrix4X4 RotateMatrix();
	Matrix4X4 TranslateMatrix();

public:
	Vector3 Position;
	Vector3 Rotation;
	Vector3 Scale;

	Texture* modelTexture;

	//顶点数据
	std::vector<Vector3> Vertices;
	std::vector<Vector2> TexCoords;
	std::vector<Vector3> Normals;
	
	//面数据
	std::vector<Index3I> VerticesIndices; // 三个顶点位置的索引
	std::vector<Index3I> TexCoordsIndices;
	std::vector<Index3I> NormalsIndices;


	Model();
	~Model();

	void update(); //对模型做一些移动 旋转等

	Matrix4X4 GetModelMatrix();
};