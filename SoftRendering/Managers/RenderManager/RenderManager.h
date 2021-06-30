#pragma once
#include <vector>
#include "Model/Model.h"
#include "Model/Light.h"
#include "Camera/Camera.h"
#include "Shader/Shader.h"

class SceneManager;
class DisplayManager;

class RenderManager  //���ν׶�
{
	SceneManager* sceneManager;
	DisplayManager* displayManager;

	std::vector<Model*>* models;
	Light* light;
	Camera* camera;

	void loadSceneResources();

	void Render(class Shader& shader, Model* model);		//��ȡ������generalģʽ��shadowģʽ�й����Ĳ���
	void GeneralMode();

	void RenderingShadowMap(Matrix4X4& lightSpaceView, Matrix4X4& lightSpaceProjection); //������Ӱģʽ�õ�shadowmap
	void RenderingScene_ShadowMap(Matrix4X4& lightSpaceView, Matrix4X4& lightSpaceProjection);	 //ʹ��shadowmap��Ⱦ����
	void RenderingLight();		//��Ⱦ��Դ
	void ShadowMode();


public:

	RenderManager() :models(nullptr), light(nullptr), camera(nullptr) {}

	void setSceneManager(SceneManager* sm);
	void setDisplayManager(DisplayManager* dm);

	void frame();
};