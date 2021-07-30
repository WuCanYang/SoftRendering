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

	void ComputeUVBounds(class SkinnedMesh& Mesh, std::vector<Vector2>& UVBounds);
	void SimplifyMesh(Model* m);

	void ConvertToSkinnedMesh(Model* m, class SkinnedMesh& outMesh);	//将模型的数据转换为能够进行 减面的数据结构
	void ConvertToModelMesh(class SkinnedMesh& inMesh, Model* m);		//将减面后的数据重新转化为	原模型数据格式


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