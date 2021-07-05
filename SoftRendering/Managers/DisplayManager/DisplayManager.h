#pragma once
#include "Buffer.h"
#include <windows.h>

class RenderManager;

class DisplayManager  //��դ���׶�
{
	RenderManager* renderManger;

	FrameBuffer* front;
	FrameBuffer* back;

	DepthBuffer* depthBuffer;
	DepthBuffer* ShadowMap;


	//��ʾ����
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
	//class Vector3 computeBarycentricCoords(class Vector3& point, class Triangle& triangle);
	void convertToRGB(Vector3& color, unsigned char& r, unsigned char& g, unsigned char& b);

	float readDepth(int x, int y);
	void writeDepth(int x, int y, float depth);
	//void writeFrameBuffer(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b);

	float readShadowMap(int x, int y);
	void writeShadowMap(int x, int y, float depth);

	//void drawLine(int x0, int y0, int x1, int y1);

public:
	~DisplayManager();

	void Init(RenderManager* rm, HWND hwnd);

	void ClearShadowMap();
	void ClearBuffer();
	void SwapBuffer();
	DepthBuffer* GetShadowMap();

	void Rasterization(class Triangle& triangle, class Shader& shader);
	void Rasterization_ShadowMapMode(class Triangle& triangle, class ShadowMapShader& shader); //ʹ��ShadowMapShader��Ⱦ��������ɫ��������һ��
	void Rasterization_ShadowMap(class Triangle& triangle);  //��ȡshadow map

	void Draw_WireFrame(class Triangle& triangle);  //�߿�ģʽ

	class Vector3 computeBarycentricCoords(class Vector3& point, class Triangle& triangle);
	void writeFrameBuffer(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b);
	void drawLine(int x0, int y0, int x1, int y1);

	void frame();

};