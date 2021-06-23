#pragma once

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
};