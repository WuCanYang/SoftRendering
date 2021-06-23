#include "SoftRender.h"

void SoftRender::Init()
{
	sceneManager.setRenderManager(&renderManager);

	renderManager.setSceneManager(&sceneManager);
	renderManager.setDisplayManager(&displayManager);

	displayManager.setRenderManager(&renderManager);
}

void SoftRender::frame()
{
	sceneManager.frame();
	renderManager.frame();
	displayManager.frame();
}
