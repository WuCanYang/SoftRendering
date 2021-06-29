#include "Shader.h"
#include "Math/Matrix3X3.h"
#include "Model/Light.h"
#include "Model/Texture.h"
#include "Camera/Camera.h"
#include "Managers/DisplayManager/Buffer.h"
#include <iostream>

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

Vector3 Shader::FragmentShader(Vector3& FragPos, Vector3& Normal, Vector2& TexCoord)
{
	Vector3 Color = texture == nullptr ? Vector3(1.0f) : texture->texture2D(TexCoord.x(), TexCoord.y());
	Normal.Normalize();

	Vector3 ambient = light->Color * 0.15f;

	Vector3 lightDir = light->Position - FragPos;
	lightDir.Normalize();
	Vector3 diffuse = light->Color * std::max(0.0f, lightDir.dot(Normal)) * 1.0f;

	Vector3 viewDir = camera->Position - FragPos;
	Vector3 halfDir = lightDir + viewDir;
	halfDir.Normalize();
	Vector3 specular = light->Color * pow(std::max(0.0f, halfDir.dot(Normal)), 64.0f) * 1.0f;

	Vector3 lighting(ambient + diffuse + specular);
	return Vector3(Color.x() * lighting.x(), Color.y() * lighting.y(), Color.z() * lighting.z());
}

//--------------------------------------------------------------

float ShadowMapShader::CalculateShadow(Vector4& FragPosLightSpace)
{
	FragPosLightSpace._x /= FragPosLightSpace._w;
	FragPosLightSpace._y /= FragPosLightSpace._w;
	FragPosLightSpace._z /= FragPosLightSpace._w;

	int x = (FragPosLightSpace._x + 1) * 0.5f * (ShadowMap->width - 1);
	int y = (FragPosLightSpace._y + 1) * 0.5f * (ShadowMap->height - 1);

	int index = y * ShadowMap->width + x;
	float depth = ShadowMap->data[index];
	float curDepth = FragPosLightSpace._z;

	float shadow = curDepth /*- 0.005f*/ > depth ? 1.0f : 0.0f;
	return shadow;
}

Vector3 ShadowMapShader::FragmentShader(Vector3& FragPos, Vector3& Normal, Vector2& TexCoord)
{
	Vector3 Color = texture == nullptr ? Vector3(1.0f) : texture->texture2D(TexCoord.x(), TexCoord.y());
	Normal.Normalize();

	Vector3 ambient = light->Color * 0.15f;

	Vector3 lightDir = light->Position - FragPos;
	lightDir.Normalize();
	Vector3 diffuse = light->Color * std::max(0.0f, lightDir.dot(Normal)) * 1.0f;

	Vector3 viewDir = camera->Position - FragPos;
	Vector3 halfDir = lightDir + viewDir;
	halfDir.Normalize();
	Vector3 specular = light->Color * pow(std::max(0.0f, halfDir.dot(Normal)), 64.0f) * 1.0f;

	Vector4 FragPosLightSpace = lightSpaceProjection * lightSpaceView * Vector4(FragPos);
	float shadow = CalculateShadow(FragPosLightSpace);

	Vector3 lighting = ambient + (diffuse + specular) * (1.0f - shadow);
	return Vector3(Color.x() * lighting.x(), Color.y() * lighting.y(), Color.z() * lighting.z());
}
