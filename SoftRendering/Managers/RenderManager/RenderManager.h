#pragma once

class SceneManager;
class DisplayManager;

class RenderManager  //���ν׶�
{
	SceneManager* sceneManager;
	DisplayManager* displayManager;

public:
	inline void setSceneManager(SceneManager* sm);
	inline void setDisplayManager(DisplayManager* dm);



	void frame();
};