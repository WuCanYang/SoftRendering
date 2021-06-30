#pragma once
#include <vector>
#include "Model/Model.h"
#include "Model/Light.h"
#include "Camera/Camera.h"
#include "Shader/Shader.h"

class SceneManager;
class DisplayManager;

class RenderManager  //几何阶段
{
	SceneManager* sceneManager;
	DisplayManager* displayManager;

	std::vector<Model*>* models;
	Light* light;
	Camera* camera;

	void loadSceneResources();

	void Render(class Shader& shader, Model* model);		//提取出来的general模式和shadow模式中公共的部分
	void GeneralMode();

	void RenderingShadowMap(Matrix4X4& lightSpaceView, Matrix4X4& lightSpaceProjection); //用于阴影模式得到shadowmap
	void RenderingScene_ShadowMap(Matrix4X4& lightSpaceView, Matrix4X4& lightSpaceProjection);	 //使用shadowmap渲染场景
	void RenderingLight();		//渲染光源
	void ShadowMode();


public:

	RenderManager() :models(nullptr), light(nullptr), camera(nullptr) {}

	void setSceneManager(SceneManager* sm);
	void setDisplayManager(DisplayManager* dm);

	void frame();
};