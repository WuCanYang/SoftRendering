#pragma once

class SceneManager;
class DisplayManager;

class RenderManager  //¼¸ºÎ½×¶Î
{
	SceneManager* sceneManager;
	DisplayManager* displayManager;

public:
	void setSceneManager(SceneManager* sm);
	void setDisplayManager(DisplayManager* dm);



	void frame();
};