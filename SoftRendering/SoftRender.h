#pragma once
#include "Managers/SceneManager/SceneManager.h"
#include "Managers/RenderManager/RenderManager.h"
#include "Managers/DisplayManager/DisplayManager.h"
#include <windows.h>

class SoftRender
{
	SceneManager sceneManager;
	RenderManager renderManager;
	DisplayManager displayManager;

public:
	void Init(HWND hwnd);
	void frame();
};