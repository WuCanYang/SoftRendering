#include "SceneManager.h"
#include "../RenderManager/RenderManager.h"
#include "../../Model/Model.h"
#include "../../Model/Light.h"
#include "../../Model/OBJ_Loader.h"

SceneManager::SceneManager(): renderManager(nullptr), camera(nullptr), light(nullptr)
{
	loadModel("");
	loadLight();
	loadCamera();
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
		camera->Target = models[0]->Position;
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

