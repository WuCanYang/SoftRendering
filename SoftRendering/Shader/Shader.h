#pragma once
#include "../Math/Matrix4X4.h"
#include "../Math/Vector3.h"
#include <vector>

class Shader
{
public:
	void VertexShader(Matrix4X4& model, Matrix4X4& view, Matrix4X4& projection, 
		std::vector<Vector3>& Vertices, std::vector<Vector3>& Normals, 
		std::vector<Vector3>& WorldVertices, std::vector<Vector3>& WorldNormals, std::vector<Vector4>& ClipVertices);  //后面三个用作顶点着色器输出
	//void FragmentShader();
};