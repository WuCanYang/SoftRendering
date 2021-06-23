#pragma once
#include <string>
#include <vector>
#include "../../Model/Model.h"
#include "../../Model/Light.h"
#include "../../Camera/Camera.h"

class RenderManager;

class SceneManager  //Ó¦ÓÃ³ÌÐò½×¶Î
{
	RenderManager* renderManager;

	std::vector<Model*> models;
	Light* light;
	Camera* camera;

private:

	void loadModel(std::string path);
	void loadLight();
	void loadCamera();

public:
	SceneManager();
	~SceneManager();

	void setRenderManager(RenderManager* rm);


	void frame();
};