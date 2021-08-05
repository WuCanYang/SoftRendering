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

	SimpVert() :flags(SIMP_DEFAULT)
	{
		prev = this;
		next = this;
	}

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

	SimpTri() :flags(SIMP_DEFAULT)
	{
		verts[0] = nullptr;
		verts[1] = nullptr;
		verts[2] = nullptr;
	}

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

	SimpEdge() :flags(SIMP_DEFAULT), v0(nullptr), v1(nullptr)
	{
		prev = this;
		next = this;
	}


	void EnableFlags(int f);
	void DisableFlags(int f);
	bool TestFlags(int f) const;
};

//SimpVert Functions
//----------------------------------------------------------------

inline void SimpVert::EnableFlags(int f)
{
	flags |= f;
}

inline void SimpVert::DisableFlags(int f)
{
	flags &= ~f;
}

inline bool SimpVert::TestFlags(int f) const
{
	return (flags & f) == f;
}


//SimpTri Functions
//----------------------------------------------------------------


inline void SimpTri::EnableFlags(int f)
{
	flags |= f;
}

inline void SimpTri::DisableFlags(int f)
{
	flags &= ~f;
}

inline bool SimpTri::TestFlags(int f) const
{
	return (flags & f) == f;
}

inline bool SimpTri::HasVertex(const SimpVert* v) const
{
	return v == verts[0] || v == verts[1] || v == verts[2];
}

inline Vector3 SimpTri::GetNormal() const
{
	Vector3 n = (verts[2]->GetPos() - verts[0]->GetPos()).cross(verts[1]->GetPos() - verts[0]->GetPos());
	n.Normalize();
	return n;
}



//SimpEdge Functions
//----------------------------------------------------------------


inline void SimpEdge::EnableFlags(int f)
{
	flags |= f;
}

inline void SimpEdge::DisableFlags(int f)
{
	flags &= ~f;
}

inline bool SimpEdge::TestFlags(int f) const
{
	return (flags & f) == f;
}