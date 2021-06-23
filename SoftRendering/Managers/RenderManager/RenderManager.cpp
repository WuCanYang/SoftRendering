#include "RenderManager.h"
#include "../SceneManager/SceneManager.h"
#include "../DisplayManager/DisplayManager.h"

void RenderManager::setSceneManager(SceneManager* sm)
{
	sceneManager = sm;
}

void RenderManager::setDisplayManager(DisplayManager* dm)
{
	displayManager = dm;
}

void RenderManager::frame()
{
}
