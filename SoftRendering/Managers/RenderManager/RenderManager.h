#pragma once

class SceneManager;
class DisplayManager;

class RenderManager  //���ν׶�
{
	SceneManager* sceneManager;
	DisplayManager* displayManager;

public:
	void setSceneManager(SceneManager* sm);
	void setDisplayManager(DisplayManager* dm);



	void frame();
};