#pragma once

class RenderManager;

class DisplayManager  //��դ���׶�
{
	RenderManager* renderManger;

public:
	inline void setRenderManager(RenderManager* rm);


	void frame();
};