#include "RenderManager.h"
#include "Managers/SceneManager/SceneManager.h"
#include "Managers/DisplayManager/DisplayManager.h"
#include "Model/Constant.h"
#include <iostream>

void RenderManager::setSceneManager(SceneManager* sm)
{
	sceneManager = sm;
}

void RenderManager::setDisplayManager(DisplayManager* dm)
{
	displayManager = dm;
}

void RenderManager::loadSceneResources()
{
	models = sceneManager->GetSceneModels();
	light = sceneManager->GetLight();
	camera = sceneManager->GetCamera();
}

void RenderManager::Render(Shader& shader, Model* model)
{
	Matrix4X4 view = camera->GetViewMatrix();
	Matrix4X4 projection = camera->GetPerspectiveMatrix();

	shader.texture = model->modelTexture;

	std::vector<Vector3>& Vertices = model->Vertices;
	std::vector<Vector2>& TexCoords = model->TexCoords;
	std::vector<Vector3>& Normals = model->Normals;

	std::vector<Index3I>& VerticesIndices = model->VerticesIndices;
	std::vector<Index3I>& TexCoordsIndices = model->TexCoordsIndices;
	std::vector<Index3I>& NormalsIndices = model->NormalsIndices;

	Matrix4X4 modelMatrix = model->GetModelMatrix();

	std::vector<Vector3> WorldVertices;  // 顶点着色器输出, 前两个用于光照的计算
	std::vector<Vector3> WorldNormals;
	std::vector<Vector4> ClipVertices;

	shader.VertexShader(modelMatrix, view, projection, Vertices, Normals, WorldVertices, WorldNormals, ClipVertices);

	for (int i = 0; i < ClipVertices.size(); ++i) //透视除法   -1 ~ 1
	{
		ClipVertices[i]._x /= ClipVertices[i]._w;
		ClipVertices[i]._y /= ClipVertices[i]._w;
		ClipVertices[i]._z /= ClipVertices[i]._w;
	}

	for (int i = 0; i < ClipVertices.size(); ++i) //视口变换
	{
		ClipVertices[i]._x = (ClipVertices[i]._x + 1) * 0.5f * (SCREEN_WIDTH - 1);
		ClipVertices[i]._y = (ClipVertices[i]._y + 1) * 0.5f * (SCREEN_HEIGHT - 1);
	}

	for (int i = 0; i < VerticesIndices.size(); ++i) //设置三角形
	{
		Index3I& VertexIndex = VerticesIndices[i];
		Index3I& TexCoordIndex = TexCoordsIndices[i];
		Index3I& NormalIndex = NormalsIndices[i];

		Triangle triangle;
		triangle.va.Position = ClipVertices[VertexIndex.index[0]];
		triangle.vb.Position = ClipVertices[VertexIndex.index[1]];
		triangle.vc.Position = ClipVertices[VertexIndex.index[2]];

		triangle.va.WorldPos = WorldVertices[VertexIndex.index[0]];
		triangle.vb.WorldPos = WorldVertices[VertexIndex.index[1]];
		triangle.vc.WorldPos = WorldVertices[VertexIndex.index[2]];

		triangle.va.WorldNormal = WorldNormals[NormalIndex.index[0]];
		triangle.vb.WorldNormal = WorldNormals[NormalIndex.index[1]];
		triangle.vc.WorldNormal = WorldNormals[NormalIndex.index[2]];

		triangle.va.TextureCoordinate = TexCoords[TexCoordIndex.index[0]];
		triangle.vb.TextureCoordinate = TexCoords[TexCoordIndex.index[1]];
		triangle.vc.TextureCoordinate = TexCoords[TexCoordIndex.index[2]];

		displayManager->Rasterization(triangle, shader);
	}
}

void RenderManager::GeneralMode()
{
	loadSceneResources();
	Shader shader;
	shader.light = light;
	shader.camera = camera;

	displayManager->ClearBuffer();
	for (Model* model : *models)
	{
		Render(shader, model);
	}
	displayManager->SwapBuffer();
}

void RenderManager::RenderingShadowMap(Matrix4X4& lightSpaceView, Matrix4X4& lightSpaceProjection)
{
	for (Model* model : *models)
	{
		std::vector<Vector3>& Vertices = model->Vertices;
		std::vector<Index3I>& VerticesIndices = model->VerticesIndices;

		Matrix4X4 modelMatrix = model->GetModelMatrix();
		std::vector<Vector4> ClipVertices;
		for (int i = 0; i < Vertices.size(); ++i)  // MVP
		{
			Vector4 ClipVert = lightSpaceProjection * lightSpaceView * modelMatrix * Vector4(Vertices[i]);
			ClipVertices.push_back(ClipVert);
		}

		for (int i = 0; i < ClipVertices.size(); ++i) //透视除法   -1 ~ 1
		{
			ClipVertices[i]._x /= ClipVertices[i]._w;
			ClipVertices[i]._y /= ClipVertices[i]._w;
			ClipVertices[i]._z /= ClipVertices[i]._w;
		}

		for (int i = 0; i < ClipVertices.size(); ++i) //视口变换
		{
			ClipVertices[i]._x = (ClipVertices[i]._x + 1) * 0.5f * (SHADOW_WIDTH - 1);
			ClipVertices[i]._y = (ClipVertices[i]._y + 1) * 0.5f * (SHADOW_HEIGHT - 1);
		}

		for (int i = 0; i < VerticesIndices.size(); ++i) //设置三角形
		{
			Index3I& VertexIndex = VerticesIndices[i];

			Triangle triangle;
			triangle.va.Position = ClipVertices[VertexIndex.index[0]];
			triangle.vb.Position = ClipVertices[VertexIndex.index[1]];
			triangle.vc.Position = ClipVertices[VertexIndex.index[2]];

			displayManager->Rasterization_ShadowMap(triangle);
		}
	}
}

void RenderManager::RenderingScene_ShadowMap(Matrix4X4& lightSpaceView, Matrix4X4& lightSpaceProjection)
{
	ShadowMapShader shader;
	shader.light = light;
	shader.camera = camera;
	shader.ShadowMap = displayManager->GetShadowMap();
	shader.lightSpaceView = lightSpaceView;
	shader.lightSpaceProjection = lightSpaceProjection;

	for (Model* model : *models)
	{
		Render(shader, model);
	}
}

void RenderManager::RenderingLight()
{
	LightShader shader;
	
	Render(shader, light->mesh);
}

void RenderManager::ShadowMode()
{
	loadSceneResources();
	Matrix4X4 lightSpaceView = light->GetViewMatrix();
	Matrix4X4 lightSpaceProjection = light->GetOrthoMatrix(-3.0f, 3.0f, -3.0f, 3.0f, 1.0f, 7.5f);

	displayManager->ClearShadowMap();
	RenderingShadowMap(lightSpaceView, lightSpaceProjection);

	displayManager->ClearBuffer();
	RenderingScene_ShadowMap(lightSpaceView, lightSpaceProjection);
	RenderingLight();
	displayManager->SwapBuffer();
}

void RenderManager::frame()
{
	if (GENERAL) GeneralMode();
	else if (SHADOW) ShadowMode();
}


