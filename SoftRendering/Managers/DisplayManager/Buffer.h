#pragma once

struct FrameBuffer
{
	unsigned char* data;
	int width;
	int height;

	~FrameBuffer()
	{
		delete data;
	}
};

struct DepthBuffer
{
	float* data;
	int width;
	int height;

	~DepthBuffer()
	{
		delete data;
	}
};