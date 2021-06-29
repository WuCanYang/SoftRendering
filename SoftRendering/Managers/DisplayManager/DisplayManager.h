#pragma once
#include "Buffer.h"
#include <windows.h>

class RenderManager;

class DisplayManager  //光栅化阶段
{
	RenderManager* renderManger;

	FrameBuffer* front;
	FrameBuffer* back;

	DepthBuffer* depthBuffer;
	DepthBuffer* shadowDepthBuffer;


	//显示所需
	HWND hwnd;
	unsigned char* ScreenBits;
	HBITMAP screenDIB = NULL, dibBefore;
	HPALETTE hPalette = NULL;
	HDC hdc, dibDC;
	BOOL mode256 = false;
	void initDIB();
	void initDIB(HDC dibDC);
	void releaseDIB();
	void releaseDIB(HDC dibDC);
	void checkDisplayMode(HDC hdc);
	void createPalette(HDC hdc);
	void releasePalette();



	bool pointInTriangle(class Vector3& point, class Triangle& triangle);
	class Vector3 computeBarycentricCoords(class Vector3& point, class Triangle& triangle);
	void convertToRGB(Vector3& color, unsigned char& r, unsigned char& g, unsigned char& b);

	float readDepth(int x, int y);
	void writeDepth(int x, int y, float depth);
	void writeFrameBuffer(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b);

public:
	~DisplayManager();

	void Init(RenderManager* rm, HWND hwnd);

	void ClearBuffer();
	void SwapBuffer();
	void Rasterization(class Triangle& triangle, class Shader& shader);

	void frame();

};