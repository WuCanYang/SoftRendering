#pragma once

class RenderManager;

class DisplayManager  //¹âÕ¤»¯½×¶Î
{
	RenderManager* renderManger;

public:
	void setRenderManager(RenderManager* rm);


	void frame();
};