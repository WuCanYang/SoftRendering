#pragma once

#include "Triangle.h"
#include "Texture.h"

class Model
{
public:
	Vector3 Position;

	std::vector<Triangle> faces;

	Texture* modelTexture;

	Model() : Position(0.0f), modelTexture(nullptr) {}
	~Model()
	{
		if (modelTexture)
			delete modelTexture;
	}
};