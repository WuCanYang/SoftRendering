#pragma once
#include "Math/Matrix4X4.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include <vector>

class Shader
{
public:

	class Light* light;
	class Camera* camera;
	class Texture* texture;


	virtual void VertexShader(Matrix4X4& model, Matrix4X4& view, Matrix4X4& projection, 
		std::vector<Vector3>& Vertices, std::vector<Vector3>& Normals, 
		std::vector<Vector3>& WorldVertices, std::vector<Vector3>& WorldNormals, std::vector<Vector4>& ClipVertices);  //后面三个用作顶点着色器输出

	virtual Vector3 FragmentShader(Vector3& FragPos, Vector3& Normal, Vector2& TexCoord);
};


class ShadowMapShader : public Shader
{
	float CalculateShadow(Vector4& FragPosLightSpace);

public:

	struct DepthBuffer* ShadowMap;
	Matrix4X4 lightSpaceView;
	Matrix4X4 lightSpaceProjection;

	Vector3 FragmentShader(Vector3& FragPos, Vector3& Normal, Vector2& TexCoord) override;
};