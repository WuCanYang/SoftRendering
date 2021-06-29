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
	m->Position = Vector3(0.0f, -0.5f, 0.0f);
	m->Scale = Vector3(2.0f, 2.0f, 2.0f);

	m->Vertices.push_back(Vector3(-1.0f, 0.0f, -1.0f));
	m->Vertices.push_back(Vector3(1.0f, 0.0f, -1.0f));
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
	m->VerticesIndices.push_back(Index3I(0, 2, 3));

	m->TexCoordsIndices.push_back(Index3I(0, 1, 2));
	m->TexCoordsIndices.push_back(Index3I(0, 2, 3));

	m->NormalsIndices.push_back(Index3I(0, 1, 2));
	m->NormalsIndices.push_back(Index3I(0, 2, 3));

	models.push_back(m);
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
	light->Position = Vector3(3.0f, 3.0f, 0.0f);
	if (!models.empty())
	{
		light->Direction = light->Position - models[0]->Position;
	}
}

void SceneManager::loadCamera()
{
	camera = new Camera;
	camera->Position = Vector3(0.0f, 3.0f, 3.0f);
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