#pragma once

class RenderManager;

class DisplayManager  //��դ���׶�
{
	RenderManager* renderManger;

public:
	void setRenderManager(RenderManager* rm);


	void frame();
};