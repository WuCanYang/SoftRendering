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

	for (size_t i = 0; i < ClipVertices.size(); ++i) //透视除法   -1 ~ 1
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

void RenderManager::Render_ShadowMapMode(ShadowMapShader& shader, Model* model)	//多了存储光源空间MVP下的FragPos
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
	std::vector<Vector4> FragPosLightSpace;

	shader.VertexShader(modelMatrix, view, projection, Vertices, Normals, WorldVertices, WorldNormals, ClipVertices, FragPosLightSpace);

	for (size_t i = 0; i < ClipVertices.size(); ++i) //透视除法   -1 ~ 1
	{
		ClipVertices[i]._x /= ClipVertices[i]._w;
		ClipVertices[i]._y /= ClipVertices[i]._w;
		ClipVertices[i]._z /= ClipVertices[i]._w;
	}

	for (int i = 0; i < ClipVertices.size(); ++i) //视口变换
	{
		ClipVertices[i]._x = (ClipVertices[i]._x + 1) * 0.5f * (SCREEN_WIDTH - 1);
		ClipVertices[i]._y = (ClipVertices[i]._y + 1) * 0.5f * (SCREEN_HEIGHT - 1);
		//ClipVertices[i]._z = (ClipVertices[i]._z + 1) * 0.5f;
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

		triangle.va.FragPosLightSpace = FragPosLightSpace[VertexIndex.index[0]];
		triangle.vb.FragPosLightSpace = FragPosLightSpace[VertexIndex.index[1]];
		triangle.vc.FragPosLightSpace = FragPosLightSpace[VertexIndex.index[2]];

		triangle.view = view;
		triangle.projection = projection;

		displayManager->Rasterization_ShadowMapMode(triangle, shader);
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
			//ClipVertices[i]._z = (ClipVertices[i]._z + 1) * 0.5f;
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
		Render_ShadowMapMode(shader, model);
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
	Matrix4X4 lightSpaceProjection = light->GetOrthoMatrix(-5.0f, 5.0f, -5.0f, 5.0f, 1.0f, 7.5f);
	

	displayManager->ClearShadowMap();
	RenderingShadowMap(lightSpaceView, lightSpaceProjection);

	displayManager->ClearBuffer(); 
	RenderingScene_ShadowMap(lightSpaceView, lightSpaceProjection);
	RenderingLight();
	testFunc();
	displayManager->SwapBuffer();
}

void RenderManager::WireframeMode()
{
	loadSceneResources();
	displayManager->ClearBuffer();

	Matrix4X4 view = camera->GetViewMatrix();
	Matrix4X4 projection = camera->GetPerspectiveMatrix();

	for (Model* model : *models)
	{
		std::vector<Vector3>& Vertices = model->Vertices;
		std::vector<Index3I>& VerticesIndices = model->VerticesIndices;

		Matrix4X4 modelMatrix = model->GetModelMatrix();

		std::vector<Vector4> ClipVertices;
		for (int i = 0; i < Vertices.size(); ++i)  // MVP
		{
			Vector4 ClipVert = projection * view * modelMatrix * Vector4(Vertices[i]);
			ClipVertices.push_back(ClipVert);
		}

		for (size_t i = 0; i < ClipVertices.size(); ++i) //透视除法   -1 ~ 1
		{
			ClipVertices[i]._x /= ClipVertices[i]._w;
			ClipVertices[i]._y /= ClipVertices[i]._w;
			ClipVertices[i]._z /= ClipVertices[i]._w;
		}

		for (int i = 0; i < ClipVertices.size(); ++i) //视口变换
		{
			ClipVertices[i]._x = (ClipVertices[i]._x + 1) * 0.5f * (SCREEN_WIDTH - 1);
			ClipVertices[i]._y = (ClipVertices[i]._y + 1) * 0.5f * (SCREEN_HEIGHT - 1);
			ClipVertices[i]._z = (ClipVertices[i]._z + 1) * 0.5f;
		}

		for (int i = 0; i < VerticesIndices.size(); ++i) //设置三角形
		{
			Index3I& VertexIndex = VerticesIndices[i];

			Triangle triangle;
			triangle.va.Position = ClipVertices[VertexIndex.index[0]];
			triangle.vb.Position = ClipVertices[VertexIndex.index[1]];
			triangle.vc.Position = ClipVertices[VertexIndex.index[2]];

			displayManager->Draw_WireFrame(triangle);
		}
	}

	displayManager->SwapBuffer();
}

void RenderManager::frame()
{
	if (GENERAL) GeneralMode();
	else if (SHADOW) ShadowMode();
	else if (WIREFRAME) WireframeMode();
}




void RenderManager::testFunc()
{
	Model* plane = (*models)[1];
	std::vector<Vector4> vec;
	std::vector<Vector4> ClipVertices;
	Matrix4X4 m = plane->GetModelMatrix();
	Matrix4X4 view = camera->GetViewMatrix();
	Matrix4X4 projection = camera->GetPerspectiveMatrix();

	std::cout << m.m11 << "   " << m.m12 << "   " << m.m13 << "   " << m.m14 << std::endl;
	std::cout << m.m21 << "   " << m.m22 << "   " << m.m23 << "   " << m.m24 << std::endl;
	std::cout << m.m31 << "   " << m.m32 << "   " << m.m33 << "   " << m.m34 << std::endl;
	std::cout << m.m41 << "   " << m.m42 << "   " << m.m43 << "   " << m.m44 << std::endl;
	std::cout << "----------------------" << std::endl;
	std::cout << view.m11 << "   " << view.m12 << "   " << view.m13 << "   " << view.m14 << std::endl;
	std::cout << view.m21 << "   " << view.m22 << "   " << view.m23 << "   " << view.m24 << std::endl;
	std::cout << view.m31 << "   " << view.m32 << "   " << view.m33 << "   " << view.m34 << std::endl;
	std::cout << view.m41 << "   " << view.m42 << "   " << view.m43 << "   " << view.m44 << std::endl;
	std::cout << "----------------------" << std::endl;
	std::cout << projection.m11 << "   " << projection.m12 << "   " << projection.m13 << "   " << projection.m14 << std::endl;
	std::cout << projection.m21 << "   " << projection.m22 << "   " << projection.m23 << "   " << projection.m24 << std::endl;
	std::cout << projection.m31 << "   " << projection.m32 << "   " << projection.m33 << "   " << projection.m34 << std::endl;
	std::cout << projection.m41 << "   " << projection.m42 << "   " << projection.m43 << "   " << projection.m44 << std::endl;
	

	Matrix4X4 tmp = projection * view * m;
	std::cout << tmp.m11 << "   " << tmp.m12 << "   " << tmp.m13 << "   " << tmp.m14 << std::endl;
	std::cout << tmp.m21 << "   " << tmp.m22 << "   " << tmp.m23 << "   " << tmp.m24 << std::endl;
	std::cout << tmp.m31 << "   " << tmp.m32 << "   " << tmp.m33 << "   " << tmp.m34 << std::endl;
	std::cout << tmp.m41 << "   " << tmp.m42 << "   " << tmp.m43 << "   " << tmp.m44 << std::endl;

	for (int i = 0; i < plane->Vertices.size(); ++i)
	{
		Vector4 worldPos = m * Vector4(plane->Vertices[i]);
		vec.push_back(worldPos);

		Vector4 clipvert = projection * view * worldPos;
		ClipVertices.push_back(clipvert);
	}

	Vector4 testPoint = vec[0] * 0.3333333f + vec[1] * 0.3333333f + vec[2] * 0.3333334f; //测试点世界坐标;
	testPoint = Vector4(-2.5f, -0.65f, 0.0f, 1.0f);
	std::cout << "testPoint worldPos :  " << testPoint.x() << "   " << testPoint.y() << "   " << testPoint.z() << std::endl;

	Vector4 testClip = projection * view * testPoint;
	testClip._x /= testClip._w;
	testClip._y /= testClip._w;
	testClip._z /= testClip._w;
	int testx = (testClip._x + 1.0f) * 0.5f * (SCREEN_WIDTH - 1);
	int testy = (testClip._y + 1.0f) * 0.5f * (SCREEN_HEIGHT - 1);	//测试点屏幕x y ， 用于计算屏幕空间重心坐标
	float testDepth = (testClip._z + 1) * 0.5f;

	std::cout << testx << "   " << testy<<"   "<<testDepth << std::endl;

	for (int i = 0; i < ClipVertices.size(); ++i)
	{
		ClipVertices[i]._x /= ClipVertices[i]._w;
		ClipVertices[i]._y /= ClipVertices[i]._w;
		//ClipVertices[i]._z /= ClipVertices[i]._w;
	}

	for (int i = 0; i < ClipVertices.size(); ++i) //视口变换
	{
		ClipVertices[i]._x = (ClipVertices[i]._x + 1.0f) * 0.5f * (SCREEN_WIDTH - 1);
		ClipVertices[i]._y = (ClipVertices[i]._y + 1.0f) * 0.5f * (SCREEN_HEIGHT - 1);
		//ClipVertices[i]._z = (ClipVertices[i]._z + 1.0f) * 0.5f;
	}

	Triangle triangle;
	triangle.va.Position = ClipVertices[0];		std::cout << ClipVertices[0].x() << "   " << ClipVertices[0].y() << "   " << ClipVertices[0].z() << std::endl;
	triangle.vb.Position = ClipVertices[1];		std::cout << ClipVertices[1].x() << "   " << ClipVertices[1].y() << "   " << ClipVertices[1].z() << std::endl;
	triangle.vc.Position = ClipVertices[2];		std::cout << ClipVertices[2].x() << "   " << ClipVertices[2].y() << "   " << ClipVertices[2].z() << std::endl;

	triangle.va.WorldPos = Vector3(vec[0].x(), vec[0].y(), vec[0].z());		std::cout << vec[0].x() << "   " << vec[0].y() << "   " << vec[0].z() << std::endl;
	triangle.vb.WorldPos = Vector3(vec[1].x(), vec[1].y(), vec[1].z());		std::cout << vec[1].x() << "   " << vec[1].y() << "   " << vec[1].z() << std::endl;
	triangle.vc.WorldPos = Vector3(vec[2].x(), vec[2].y(), vec[2].z());		std::cout << vec[2].x() << "   " << vec[2].y() << "   " << vec[2].z() << std::endl;

	//displayManager->drawLine(testx, testy, triangle.va.Position.x(), triangle.va.Position.y());
	//displayManager->drawLine(triangle.va.Position.x(), triangle.va.Position.y(), triangle.vc.Position.x(), triangle.vc.Position.y());

	Vector3 p(testx, testy, 1.0f);
	Vector3 Coords = displayManager->computeBarycentricCoords(p, triangle); std::cout << Coords.x() << "   " << Coords.y() << "   " << Coords.z() << std::endl;
	Vector3 zVal(1.0f / triangle.va.Position.z(), 1.0f / triangle.vb.Position.z(), 1.0f / triangle.vc.Position.z());
	float depth = 1.0f / (Coords.dot(zVal));
	std::cout << depth << std::endl;

	Vector3 interpolateFragPos = (triangle.va.WorldPos * Coords.x() * zVal.x() + triangle.vb.WorldPos * Coords.y() * zVal.y()
		+ triangle.vc.WorldPos * Coords.z() * zVal.z()) * depth;

	std::cout << "interpolateFragPos:  " << interpolateFragPos.x() << "   " << interpolateFragPos.y() << "    " << interpolateFragPos.z() << std::endl;
}