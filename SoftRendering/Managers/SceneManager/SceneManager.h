#pragma once
#include <string>
#include <vector>

class RenderManager;
class Model;
class Light;

class SceneManager  //Ӧ�ó���׶�
{
	RenderManager* renderManager;

	std::vector<Model*> models;
	Light* light;

private:

	void loadModel(std::string path);
	void loadLight();

public:
	SceneManager();
	~SceneManager();

	inline void setRenderManager(RenderManager* rm);


	void frame();
};