#include "SceneManager.h"
#include "../RenderManager/RenderManager.h"
#include "../../Model/Model.h"
#include "../../Model/Light.h"

SceneManager::SceneManager()
{
	loadModel("");
	loadLight();
}

SceneManager::~SceneManager()
{
	for (Model* model : models)
	{
		delete model;
	}
	delete light;
}

void SceneManager::loadModel(std::string path)
{

}

void SceneManager::loadLight()
{

}

void SceneManager::setRenderManager(RenderManager* rm)
{
	renderManager = rm;
}

void SceneManager::frame()
{
}
