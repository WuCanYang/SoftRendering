#include "SceneManager.h"
#include "Managers/RenderManager/RenderManager.h"
#include "Model/Model.h"
#include "Model/Light.h"
#include "Model/OBJ_Loader.h"
#include <Model/Constant.h>

SceneManager::SceneManager(): renderManager(nullptr), camera(nullptr), light(nullptr)
{
	std::cout << "Scene loading" << std::endl;

	loadModel("../Models/spot.obj");
	loadLight();
	loadCamera();

	if (SHADOW && !GENERAL && !WIREFRAME)
	{
		loadPlane();
	}
}

SceneManager::~SceneManager()
{
	for (Model* model : models)
	{
		delete model;
	}
	delete light;
	delete camera;
}

void SceneManager::loadPlane()
{
	Model* m = new Model;
	//m->Position = Vector3(0.0f, -1.3f, 0.0f);
	m->Position = Vector3(0.0f, -0.65f, 0.0f);
	m->Scale = Vector3(0.5f);

	/*float planeVertices[] = {
		// positions            // normals         // texcoords
		 5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		-5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,

		 5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		 5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
		-5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f
	};*/

	float planeVertices[] = {
		// positions            // normals         // texcoords
		 5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		-5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		 0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,

		-5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		 0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,

		 0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,   0.0f, 5.0f,
		 5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f,

		 5.0f, 0.0f,  5.0f,  0.0f, 1.0f, 0.0f,  5.0f,  0.0f,
		 0.0f, 0.0f,  0.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		 5.0f, 0.0f, -5.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f,
	};

	for (int i = 0; i < 12; ++i)
	{
		int index = i * 8;
		m->Vertices.push_back(Vector3(planeVertices[index], planeVertices[index + 1], planeVertices[index + 2]));
		m->Normals.push_back(Vector3(planeVertices[index + 3], planeVertices[index + 4], planeVertices[index + 5]));
		m->TexCoords.push_back(Vector2(planeVertices[index + 6], planeVertices[index + 7]));

		if ((i + 1) % 3 == 0)
		{
			m->VerticesIndices.push_back(Index3I(i - 2, i - 1, i));
			m->TexCoordsIndices.push_back(Index3I(i - 2, i - 1, i));
			m->NormalsIndices.push_back(Index3I(i - 2, i - 1, i));
		}
	}
	models.push_back(m);

	/*m->Vertices.push_back(Vector3(1.0f, 0.0f, -1.0f));
	m->Vertices.push_back(Vector3(-1.0f, 0.0f, -1.0f));
	m->Vertices.push_back(Vector3(1.0f, 0.0f, 1.0f));
	m->Vertices.push_back(Vector3(-1.0f, 0.0f, 1.0f));

	m->TexCoords.push_back(Vector2(-1.0f, -1.0f));
	m->TexCoords.push_back(Vector2(1.0f, -1.0f));
	m->TexCoords.push_back(Vector2(1.0f, 1.0f));
	m->TexCoords.push_back(Vector2(-1.0f, 1.0f));

	m->Normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	m->Normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	m->Normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	m->Normals.push_back(Vector3(0.0f, 1.0f, 0.0f));

	m->VerticesIndices.push_back(Index3I(0, 1, 2));
	m->VerticesIndices.push_back(Index3I(1, 2, 3));

	m->TexCoordsIndices.push_back(Index3I(0, 1, 2));
	m->TexCoordsIndices.push_back(Index3I(1, 2, 3));

	m->NormalsIndices.push_back(Index3I(0, 1, 2));
	m->NormalsIndices.push_back(Index3I(1, 2, 3));*/
}

void SceneManager::loadModel(std::string name)
{
	Model* m = Loader::LoadFile(name);
	if (!m) return;
	models.push_back(m);
}

void SceneManager::loadLight()
{
	light = new Light;
	//light->Position = Vector3(1.0f, 3.0f, -1.0f);
	light->Position = Vector3(-1.0f, 3.0f, -2.0f);
	if (!models.empty())
	{
		light->Direction = light->Position - Vector3();//models[0]->Position;
	}

	Model* m = new Model;		//为光源添加顶点数据等，用于光源可视化
	m->Position = light->Position;
	m->Scale = Vector3(0.1f);
	light->mesh = m;

	float vertices[] = {		//cube数据  P/N/T
		// back face
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
	};

	for (int i = 0; i < 36; ++i)
	{
		int index = i * 8;
		m->Vertices.push_back(Vector3(vertices[index], vertices[index+1], vertices[index + 2]));
		m->Normals.push_back(Vector3(vertices[index + 3], vertices[index + 4], vertices[index + 5]));
		m->TexCoords.push_back(Vector2(vertices[index + 6], vertices[index + 7]));

		if ((i + 1) % 3 == 0)
		{
			m->VerticesIndices.push_back(Index3I(i - 2, i - 1, i));
			m->TexCoordsIndices.push_back(Index3I(i - 2, i - 1, i));
			m->NormalsIndices.push_back(Index3I(i - 2, i - 1, i));
		}
	}
}

void SceneManager::loadCamera()
{
	camera = new Camera;
	camera->Position = Vector3(0.0f, 5.0f, 3.0f);
	if (!models.empty())
	{
		camera->Target = Vector3(0.0f); //models[0]->Position;
	}
}

void SceneManager::setRenderManager(RenderManager* rm)
{
	renderManager = rm;
}

void SceneManager::frame()
{
	for (Model* model : models)
	{
		model->update();
	}
	light->update();
}

std::vector<Model*>* SceneManager::GetSceneModels()
{
	return &models;
}

Light* SceneManager::GetLight()
{
	return light;
}

Camera* SceneManager::GetCamera()
{
	return camera;
}