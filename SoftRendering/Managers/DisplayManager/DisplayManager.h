#pragma once
#include "Buffer.h"

class RenderManager;

class DisplayManager  //¹âÕ¤»¯½×¶Î
{
	RenderManager* renderManger;

	FrameBuffer* front;
	FrameBuffer* back;

	DepthBuffer* depthBuffer;
	DepthBuffer* shadowDepthBuffer;

	bool pointInTriangle(class Vector3& point, class Triangle& triangle);
	class Vector3 computeBarycentricCoords(class Vector3& point, class Triangle& triangle);

public:
	~DisplayManager();

	void Init(RenderManager* rm);

	void ClearBuffer();
	void SwapBuffer();
	void Rasterization(class Triangle& triangle, class Shader& shader);
	void Display();

	void frame();
};