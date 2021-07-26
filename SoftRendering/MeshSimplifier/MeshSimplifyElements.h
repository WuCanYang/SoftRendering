#pragma once
#include "SimplifierVertex.h"
#include <list>
#include <vector>
#include <iostream>

enum SimpElementFlags
{
	SIMP_DEFAULT	= 0,
	SIMP_REMOVED	= 1 << 0,
	SIMP_MARK1		= 1 << 1,
	SIMP_MARK2		= 1 << 2,
	SIMP_LOCKED		= 1 << 3,
};

class SimpTri;

class SimpVert
{
public:

	MeshVertType vert;

	SimpVert* prev;
	SimpVert* next;

	int flags;

	std::list<SimpTri*> adjTris;

public:

	SimpVert() :flags(SIMP_DEFAULT), prev(nullptr), next(nullptr) {}

	Vector3& GetPos() { return vert.GetPos(); }
	const Vector3& GetPos() const { return vert.GetPos(); }
	unsigned int GetMaterialIndex() { return vert.GetMaterialIndex(); }

	void EnableFlags(int f);
	void DisableFlags(int f);
	bool TestFlags(int f) const;

	void EnableAdjVertFlags(int f);
	void DisableAdjVertFlags(int f);

	void EnableAdjTriFlags(int f);
	void DisableAdjTriFlags(int f);


	void FindAdjacentVerts(std::vector<SimpVert*>& adjVerts);

	// 同一组中的顶点位置相同
	void EnableFlagsGroup(int f);
	void DisableFlagsGroup(int f);

	void EnableAdjVertFlagsGroup(int f);
	void DisableAdjVertFlagsGroup(int f);

	void EnableAdjTriFlagsGroup(int f);
	void DisableAdjTriFlagsGroup(int f);

	void FindAdjacentVertsGroup(std::vector<SimpVert*>& adjVerts);
};


class SimpTri
{
public:

	SimpVert* verts[3];

	int flags;

public:

	SimpTri() :flags(SIMP_DEFAULT) {}

	void EnableFlags(int f);
	void DisableFlags(int f);
	bool TestFlags(int f) const;

	bool HasVertex(const SimpVert* v) const;
	Vector3 GetNormal() const;

	bool ReplaceVertexIsValid(const SimpVert* oldV, const Vector3& pos) const;
	void ReplaceVertex(SimpVert* oldV, SimpVert* newV);
};

class SimpEdge
{
public:
	SimpVert* v0;
	SimpVert* v1;

	SimpEdge* prev;
	SimpEdge* next;

	int flags;

public:

	SimpEdge() :flags(SIMP_DEFAULT), v0(nullptr), v1(nullptr), prev(nullptr), next(nullptr) {}


	void EnableFlags(int f);
	void DisableFlags(int f);
	bool TestFlags(int f) const;
};


