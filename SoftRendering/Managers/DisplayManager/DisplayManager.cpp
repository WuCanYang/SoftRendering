#include "DisplayManager.h"
#include "../RenderManager/RenderManager.h"
#include "../../Model/Triangle.h"
#include "../../Shader/Shader.h"
#include "../../Model/Constant.h"


DisplayManager::~DisplayManager()
{
	delete front;
	delete back;
	delete depthBuffer;
	delete shadowDepthBuffer;
}


void DisplayManager::Init(RenderManager* rm)
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
}

void DisplayManager::frame()
{
	Display();
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

void DisplayManager::Rasterization(Triangle& triangle, Shader& shader)
{
	float minX = std::min(triangle.va.Position.x(), std::min(triangle.vb.Position.x(), triangle.vc.Position.x()));
	float minY = std::min(triangle.va.Position.y(), std::min(triangle.vb.Position.y(), triangle.vc.Position.y()));
	float maxX = std::max(triangle.va.Position.x(), std::max(triangle.vb.Position.x(), triangle.vc.Position.x()));
	float maxY = std::max(triangle.va.Position.y(), std::max(triangle.vb.Position.y(), triangle.vc.Position.y()));

	int startX = std::max(0, (int)minX);
	int startY = std::max(0, (int)minY);
	int endX = std::min(SCREEN_HEIGHT - 1, (int)(maxX + 0.5f));
	int endY = std::min(SCREEN_WIDTH - 1, (int)(maxY + 0.5f));

	for (int i = startX; i <= endX; ++i)
	{
		for (int j = startY; j <= endY; ++j)
		{
			Vector3 point((float)i + 0.5f, (float)j + 0.5f, 1.0f);
			if (!pointInTriangle(point, triangle)) continue;

			Vector3 Coords = computeBarycentricCoords(point, triangle);
			Vector3 zVal(1.0f / triangle.va.Position.z(), 1.0f / triangle.vb.Position.z(), 1.0f / triangle.vc.Position.z());

			float depth = 1.0f / (Coords.dot(zVal));		//Í¸ÊÓ½ÃÕý


			Vector2 interpolateTexCoord = (triangle.va.TextureCoordinate * Coords.x() * zVal.x() + triangle.vb.TextureCoordinate * Coords.y() * zVal.y()
				+ triangle.vc.TextureCoordinate * Coords.z() * zVal.z()) * depth;

			Vector3 interpolateFragPos = (triangle.va.WorldPos * Coords.x() * zVal.x() + triangle.vb.WorldPos * Coords.y() * zVal.y()
				+ triangle.vc.WorldPos * Coords.z() * zVal.z()) * depth;

			Vector3 interpolateNormal = (triangle.va.WorldNormal * Coords.x() * zVal.x() + triangle.vb.WorldNormal * Coords.y() * zVal.y()
				+ triangle.vc.WorldNormal * Coords.z() * zVal.z()) * depth;

		}
	}
}

void DisplayManager::Display()
{

}


