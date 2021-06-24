#pragma once
#include <vector>
#include "../../Model/Model.h"
#include "../../Model/Light.h"
#include "../../Camera/Camera.h"
#include "../../Shader/Shader.h"

class SceneManager;
class DisplayManager;

class RenderManager  //¼¸ºÎ½×¶Î
{
	SceneManager* sceneManager;
	DisplayManager* displayManager;

	std::vector<Model*>* models;
	Light* light;
	Camera* camera;

	Shader shader;

	void loadSceneResources();

public:

	void setSceneManager(SceneManager* sm);
	void setDisplayManager(DisplayManager* dm);

	void frame();
};