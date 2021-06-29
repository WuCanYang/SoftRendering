#include "DisplayManager.h"
#include "Managers/RenderManager/RenderManager.h"
#include "Model/Triangle.h"
#include "Shader/Shader.h"
#include "Model/Constant.h"
#include "Model/Light.h"
#include "Camera/Camera.h"


DisplayManager::~DisplayManager()
{
	delete front;
	delete back;
	delete depthBuffer;
	delete shadowDepthBuffer;

	releasePalette();
	releaseDIB(dibDC);
	DeleteDC(dibDC);
	ReleaseDC(hwnd, hdc);

}


void DisplayManager::Init(RenderManager* rm, HWND hwnd)
{
	renderManger = rm;

	front = new FrameBuffer;
	front->height = SCREEN_HEIGHT;
	front->width = SCREEN_WIDTH;
	front->data = new unsigned char[SCREEN_WIDTH * SCREEN_HEIGHT * 3];

	back = new FrameBuffer;
	back->height = SCREEN_HEIGHT;
	back->width = SCREEN_WIDTH;
	back->data = new unsigned char[SCREEN_WIDTH * SCREEN_HEIGHT * 3];

	depthBuffer = new DepthBuffer;
	depthBuffer->height = SCREEN_HEIGHT;
	depthBuffer->width = SCREEN_WIDTH;
	depthBuffer->data = new float[SCREEN_WIDTH * SCREEN_HEIGHT];

	shadowDepthBuffer = new DepthBuffer;
	shadowDepthBuffer->height = SCREEN_HEIGHT;
	shadowDepthBuffer->width = SCREEN_WIDTH;
	shadowDepthBuffer->data = new float[SCREEN_WIDTH * SCREEN_HEIGHT];


	this->hwnd = hwnd;
	hdc = GetDC(hwnd);
	checkDisplayMode(hdc);
	createPalette(hdc);
	dibDC = CreateCompatibleDC(hdc);
	releaseDIB(dibDC);
	initDIB(dibDC);

	if (hPalette) {
		SelectPalette(hdc, hPalette, FALSE);
		RealizePalette(hdc);
	}
}

void DisplayManager::frame()
{
	BitBlt(hdc, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, dibDC, 0, 0, SRCCOPY);
}

void DisplayManager::ClearBuffer()
{
	memset(back->data, 0, SCREEN_HEIGHT * SCREEN_WIDTH * 3 * sizeof(unsigned char));
	for (int i = 0; i < depthBuffer->height * depthBuffer->width; ++i)
	{
		depthBuffer->data[i] = 1.0f;
	}
}

void DisplayManager::SwapBuffer()
{
	FrameBuffer* fb = front;
	front = back;
	back = fb;

	for (int i = 0; i < front->height; ++i)
	{
		for (int j = 0; j < front->width; ++j)
		{
			int index = (i * front->width + j) * 3;
			unsigned char red = front->data[index];
			unsigned char green = front->data[index + 1];
			unsigned char blue = front->data[index + 2];
			ScreenBits[index] = blue;
			ScreenBits[index + 1] = green;
			ScreenBits[index + 2] = red;
		}
	}
}

bool DisplayManager::pointInTriangle(Vector3& point, Triangle& triangle)
{
	Vector3 vertexA = Vector3(triangle.va.Position.x(), triangle.va.Position.y(), 1.0f);
	Vector3 vertexB = Vector3(triangle.vb.Position.x(), triangle.vb.Position.y(), 1.0f);
	Vector3 vertexC = Vector3(triangle.vc.Position.x(), triangle.vc.Position.y(), 1.0f);

	Vector3 AB = vertexB - vertexA;
	Vector3 BC = vertexC - vertexB;
	Vector3 CA = vertexA - vertexC;
	Vector3 AP = point - vertexA;
	Vector3 BP = point - vertexB;
	Vector3 CP = point - vertexC;

	Vector3 cross1 = AB.cross(AP);
	Vector3 cross2 = BC.cross(BP);
	Vector3 cross3 = CA.cross(CP);

	if ((cross1.dot(cross2) > 0 && cross2.dot(cross3) > 0 && cross3.dot(cross1) > 0)
		|| (cross1.dot(cross2) < 0 && cross2.dot(cross3) < 0 && cross3.dot(cross1) < 0))
	{
		return true;
	}
	return false;
}

Vector3 DisplayManager::computeBarycentricCoords(Vector3& point, Triangle& triangle)
{
	float common = (triangle.va.Position.y() - triangle.vc.Position.y()) * (triangle.vb.Position.x() - triangle.vc.Position.x())
		+ (triangle.vb.Position.y() - triangle.vc.Position.y()) * (triangle.vc.Position.x() - triangle.va.Position.x());

	float a = (point.y() - triangle.vc.Position.y()) *(triangle.vb.Position.x() - triangle.vc.Position.x())
		+ (triangle.vb.Position.y() - triangle.vc.Position.y()) * (triangle.vc.Position.x() - point.x());

	float b = (point.y() - triangle.va.Position.y()) * (triangle.vc.Position.x() - triangle.va.Position.x())
		+ (triangle.vc.Position.y() - triangle.va.Position.y()) * (triangle.va.Position.x() - point.x());

	float c = (point.y() - triangle.vb.Position.y()) * (triangle.va.Position.x() - triangle.vb.Position.x())
		+ (triangle.va.Position.y() - triangle.vb.Position.y()) * (triangle.vb.Position.x() - point.x());

	return Vector3(a / common, b / common, c / common);
}

float DisplayManager::readDepth(int x, int y)
{
	y = SCREEN_HEIGHT - 1 - y;		

	int index = y * SCREEN_WIDTH + x;
	return depthBuffer->data[index];
}

void DisplayManager::writeDepth(int x, int y, float depth)
{
	y = SCREEN_HEIGHT - 1 - y;

	int index = y * SCREEN_WIDTH + x;
	depthBuffer->data[index] = depth;
}

void DisplayManager::convertToRGB(Vector3& color, unsigned char& r, unsigned char& g, unsigned char& b)
{
	float red = color.x() * 255;
	float green = color.y() * 255;
	float blue = color.z() * 255;
	r = red > 255 ? 255 : red;
	g = green > 255 ? 255 : green;
	b = blue > 255 ? 255 : blue;
}

void DisplayManager::writeFrameBuffer(int x, int y, unsigned char& r, unsigned char& g, unsigned char& b)
{
	y = SCREEN_HEIGHT - 1 - y;		//不加图像倒立

	int index = (y * SCREEN_WIDTH + x) * 3;
	back->data[index] = r;
	back->data[index + 1] = g;
	back->data[index + 2] = b;
}

void DisplayManager::Rasterization(Triangle& triangle, Shader& shader)
{
	float minX = min(triangle.va.Position.x(), min(triangle.vb.Position.x(), triangle.vc.Position.x()));
	float minY = min(triangle.va.Position.y(), min(triangle.vb.Position.y(), triangle.vc.Position.y()));
	float maxX = max(triangle.va.Position.x(), max(triangle.vb.Position.x(), triangle.vc.Position.x()));
	float maxY = max(triangle.va.Position.y(), max(triangle.vb.Position.y(), triangle.vc.Position.y()));

	int startX = max(0, (int)minX);
	int startY = max(0, (int)minY);
	int endX = min(SCREEN_WIDTH - 1, (int)(maxX + 0.5f));
	int endY = min(SCREEN_HEIGHT - 1, (int)(maxY + 0.5f));

	for (int j = startY; j <= endY; ++j)
	{
		for (int i = startX; i <= endX; ++i)
		{
			Vector3 point((float)i + 0.5f, (float)j + 0.5f, 1.0f);
			if (!pointInTriangle(point, triangle)) continue;

			Vector3 Coords = computeBarycentricCoords(point, triangle);
			Vector3 zVal(1.0f / triangle.va.Position.z(), 1.0f / triangle.vb.Position.z(), 1.0f / triangle.vc.Position.z());

			float depth = 1.0f / (Coords.dot(zVal));		//透视矫正
			if (depth < readDepth(i, j))
			{
				writeDepth(i, j, depth);

				Vector2 interpolateTexCoord = (triangle.va.TextureCoordinate * Coords.x() * zVal.x() + triangle.vb.TextureCoordinate * Coords.y() * zVal.y()
					+ triangle.vc.TextureCoordinate * Coords.z() * zVal.z()) * depth;

				Vector3 interpolateFragPos = (triangle.va.WorldPos * Coords.x() * zVal.x() + triangle.vb.WorldPos * Coords.y() * zVal.y()
					+ triangle.vc.WorldPos * Coords.z() * zVal.z()) * depth;

				Vector3 interpolateNormal = (triangle.va.WorldNormal * Coords.x() * zVal.x() + triangle.vb.WorldNormal * Coords.y() * zVal.y()
					+ triangle.vc.WorldNormal * Coords.z() * zVal.z()) * depth;

				Vector3 color = shader.FragmentShader(interpolateFragPos, interpolateNormal, interpolateTexCoord);
				unsigned char r, g, b;
				convertToRGB(color, r, g, b);
				writeFrameBuffer(i, j, r, g, b);
			}
		}
	}
}

void DisplayManager::initDIB()
{
	initDIB(dibDC);
}

void DisplayManager::initDIB(HDC dibDC)
{
	BITMAPINFO bmi;
	memset(&bmi, 0, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = SCREEN_WIDTH;
	bmi.bmiHeader.biHeight = -SCREEN_HEIGHT;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	//创建应用程序直接写入与设备无关的位图

	screenDIB = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&ScreenBits, NULL, 0);

	//把一个位图对象选入到指定的设备描述表当中，来替换原来的旧对象
	dibBefore = (HBITMAP)SelectObject(dibDC, screenDIB);
}

void DisplayManager::releaseDIB()
{
	releaseDIB(dibDC);
}

void DisplayManager::releaseDIB(HDC dibDC)
{
	if (screenDIB != NULL) {
		SelectObject(dibDC, dibBefore);
		DeleteObject(screenDIB);
		screenDIB = NULL;
	}
}

void DisplayManager::checkDisplayMode(HDC hdc)
{
	int palSize = GetDeviceCaps(hdc, SIZEPALETTE);
	if (palSize == 256)
		mode256 = true;
}

void DisplayManager::createPalette(HDC hdc)
{
	if (!mode256)
		return;
	printf("create palette\n");
	//	hPalette=CreateHalftonePalette(hdc);
	int i = 0, incr = 8, red = 0, green = 0, blue = 0;
	LOGPALETTE* plp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + 246 * sizeof(PALETTEENTRY));
	plp->palVersion = 0x0300;
	plp->palNumEntries = 247;
	for (green = 0; green <= 255; green += incr) {
		plp->palPalEntry[i].peRed = (BYTE)green;
		plp->palPalEntry[i].peGreen = (BYTE)green;
		plp->palPalEntry[i].peBlue = (BYTE)green;
		plp->palPalEntry[i].peFlags = 0;
		incr = (incr == 9) ? 8 : 9;
		i++;
	}
	for (red = 0; red <= 255; red += 51) {
		for (green = 0; green <= 255; green += 51) {
			for (blue = 0; blue <= 255; blue += 51) {
				plp->palPalEntry[i].peRed = (BYTE)red;
				plp->palPalEntry[i].peGreen = (BYTE)green;
				plp->palPalEntry[i].peBlue = (BYTE)blue;
				plp->palPalEntry[i].peFlags = 0;
				i++;
			}
		}
	}
	hPalette = CreatePalette(plp);
	free(plp);
}

void DisplayManager::releasePalette()
{
	if (hPalette) {
		DeleteObject(hPalette);
		hPalette = NULL;
		printf("release palette\n");
	}
}