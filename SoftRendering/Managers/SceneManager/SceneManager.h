#pragma once
#include <string>
#include <vector>
#include "Model/Model.h"
#include "Model/Light.h"
#include "Camera/Camera.h"

class RenderManager;

class SceneManager  //应用程序阶段
{
	RenderManager* renderManager;

	std::vector<Model*> models;
	Light* light;
	Camera* camera;

private:

	void loadModel(std::string path);
	void loadLight();
	void loadCamera();

	void loadPlane();  //阴影模式下用于阴影的呈现

public:
	SceneManager();
	~SceneManager();

	void setRenderManager(RenderManager* rm);

	std::vector<Model*>* GetSceneModels();
	Light* GetLight();
	Camera* GetCamera();

	void frame();
};