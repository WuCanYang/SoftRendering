#pragma once
#include <stdlib.h>
#include "Math/Vector3.h"

class Texture
{
public:
	unsigned char* data;
	int width, height;
	int channel;

	Texture(unsigned char* t, int w, int h, int c) : data(t), width(w), height(h), channel(c) {}
	~Texture()
	{
		free(data);
	}

	Vector3 GetTexturePixel(int index)
	{
		return Vector3(data[index], data[index + 1], data[index + 2]);
	}
	
	Vector3 texture2D(float u, float v)  //Ë«ÏßÐÔ²åÖµ
	{
		int x = (int)(u * (width - 1));
		int y = (int)(v * (height - 1));
		int index = (y * width + x) * channel;

		int x1 = x + 1 <= width - 1 ? x + 1 : width - 1;
		int y1 = y;
		int index1 = (y1 * width + x1) * channel;

		float weight = (float)x1 - u * (float)(width - 1);
		Vector3 interX1 = GetTexturePixel(index) * (weight) + GetTexturePixel(index1) * (1.0f - weight);

		int x2 = x;
		int y2 = y + 1 <= height - 1 ? y + 1 : height - 1;
		int index2 = (y2 * width + x2) * channel;

		int x3 = x + 1 <= width - 1 ? x + 1 : width - 1;
		int y3 = y + 1 <= height - 1 ? y + 1 : height - 1;
		int index3 = (y3 * width + x3) * channel;

		Vector3 interX2 = GetTexturePixel(index2) * (weight) + GetTexturePixel(index3) * (1.0f - weight);

		weight = (float)y2 - v * (float)(height - 1);
		Vector3 interY = interX1 * (weight)+interX2 * (1.0f - weight);
		return Vector3(interY.x() / 255, interY.y() / 255, interY.z() / 255);
	}
};