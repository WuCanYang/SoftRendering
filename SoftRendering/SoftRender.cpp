#include "SoftRender.h"
#include <iostream>
#include <ctime>

void SoftRender::Init(HWND hwnd)
{
	sceneManager.setRenderManager(&renderManager);

	renderManager.setSceneManager(&sceneManager);
	renderManager.setDisplayManager(&displayManager);

	displayManager.Init(&renderManager, hwnd);
}

void SoftRender::frame()
{
	double start = clock();
	sceneManager.frame();
	renderManager.frame();
	displayManager.frame();
	double end = clock();
	std::cout << "FPS: " << CLOCKS_PER_SEC / (end - start) << std::endl;
}
