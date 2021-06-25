#include "Shader.h"
#include "../Math/Matrix3X3.h"

void Shader::VertexShader(Matrix4X4& model, Matrix4X4& view, Matrix4X4& projection, 
	std::vector<Vector3>& Vertices, std::vector<Vector3>& Normals, 
	std::vector<Vector3>& WorldVertices, std::vector<Vector3>& WorldNormals, std::vector<Vector4>& ClipVertices)
{
	for (int i = 0; i < Vertices.size(); ++i)
	{
		Vector4 worldPos = model * Vector4(Vertices[i]);

		Matrix3X3 normalMatrix = Matrix3X3(model).inverse().transpose();
		Vector3 worldNormal = normalMatrix * Normals[i];

		Vector4 ClipVert = projection * view * worldPos;

		WorldVertices.push_back(Vector3(worldPos.x(), worldPos.y(), worldPos.z()));
		WorldNormals.push_back(worldNormal);
		ClipVertices.push_back(ClipVert);
	}
}
