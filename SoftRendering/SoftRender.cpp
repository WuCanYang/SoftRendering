#include "SoftRender.h"
#include <iostream>

void SoftRender::Init(HWND hwnd)
{
	sceneManager.setRenderManager(&renderManager);

	renderManager.setSceneManager(&sceneManager);
	renderManager.setDisplayManager(&displayManager);

	displayManager.Init(&renderManager, hwnd);
}

void SoftRender::frame()
{
	sceneManager.frame();
	renderManager.frame();
	displayManager.frame();
}
