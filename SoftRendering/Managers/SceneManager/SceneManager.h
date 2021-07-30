#pragma once
#include <string>
#include <vector>
#include "Model/Model.h"
#include "Model/Light.h"
#include "Camera/Camera.h"

class RenderManager;

class SceneManager  //Ӧ�ó���׶�
{
	RenderManager* renderManager;

	std::vector<Model*> models;
	Light* light;
	Camera* camera;

private:

	void ComputeUVBounds(class SkinnedMesh& Mesh, std::vector<Vector2>& UVBounds);
	void SimplifyMesh(Model* m);

	void ConvertToSkinnedMesh(Model* m, class SkinnedMesh& outMesh);	//��ģ�͵�����ת��Ϊ�ܹ����� ��������ݽṹ
	void ConvertToModelMesh(class SkinnedMesh& inMesh, Model* m);		//����������������ת��Ϊ	ԭģ�����ݸ�ʽ


	void loadModel(std::string path);
	void loadLight();
	void loadCamera();

	void loadPlane();  //��Ӱģʽ��������Ӱ�ĳ���

public:
	SceneManager();
	~SceneManager();

	void setRenderManager(RenderManager* rm);

	std::vector<Model*>* GetSceneModels();
	Light* GetLight();
	Camera* GetCamera();

	void frame();
};