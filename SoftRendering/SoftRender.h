#pragma once
#include "Managers/SceneManager/SceneManager.h"
#include "Managers/RenderManager/RenderManager.h"
#include "Managers/DisplayManager/DisplayManager.h"

class SoftRender
{
	SceneManager sceneManager;
	RenderManager renderManager;
	DisplayManager displayManager;

public:
	void Init();
	void frame();
};