#pragma once
#include <vector>
#include "Model/Model.h"
#include "Model/Light.h"
#include "Camera/Camera.h"
#include "Shader/Shader.h"
#include "Model/Constant.h"

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
	void Render_ShadowMapMode(class ShadowMapShader& shader, Model* model);		//ShadowMap下渲染场景
	void GeneralMode();

	void Render_useQuaternion(class Shader& shader, Model* model);		//有点绕的使用四元数旋转
	void QuaternionMode();

	void RenderingShadowMap(Matrix4X4& lightSpaceView, Matrix4X4& lightSpaceProjection); //用于阴影模式得到shadowmap
	void RenderingScene_ShadowMap(Matrix4X4& lightSpaceView, Matrix4X4& lightSpaceProjection);	 //使用shadowmap渲染场景
	void RenderingLight();		//渲染光源
	void ShadowMode();


	void WireframeMode();



	void testFunc();

public:

	RenderManager() :sceneManager(nullptr), displayManager(nullptr), models(nullptr), light(nullptr), camera(nullptr) {}

	void setSceneManager(SceneManager* sm);
	void setDisplayManager(DisplayManager* dm);

	void frame();
};