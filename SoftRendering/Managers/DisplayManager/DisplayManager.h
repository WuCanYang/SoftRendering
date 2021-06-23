#pragma once

class RenderManager;

class DisplayManager  //¹âÕ¤»¯½×¶Î
{
	RenderManager* renderManger;

public:
	inline void setRenderManager(RenderManager* rm);


	void frame();
};