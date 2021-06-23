#pragma once
#include "../Math/Vector2.h"
#include "../Math/Vector3.h"

class Vertex
{
public:
	Vector3 Position;
	Vector3 Normal;
	Vector2 TextureCoordinate;
};

class Triangle
{
public:
	Vertex va, vb, vc;

	Triangle() {}
	Triangle(const Vertex& a, const Vertex& b, const Vertex& c) : va(a), vb(b), vc(c) {}
};