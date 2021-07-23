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

	SimpVert() :flags(SIMP_DEFAULT) {}

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

	SimpEdge() :flags(SIMP_DEFAULT) {}


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

void SimpVert::EnableAdjVertFlags(int f)
{
	for (auto it = adjTris.begin(); it != adjTris.end(); ++it)
	{
		for (int j = 0; j < 3; ++j)
		{
			(*it)->verts[j]->EnableFlags(f);
		}
	}
}

void SimpVert::DisableAdjVertFlags(int f)
{
	for (auto it = adjTris.begin(); it != adjTris.end(); ++it)
	{
		for (int j = 0; j < 3; ++j)
		{
			(*it)->verts[j]->DisableFlags(f);
		}
	}
}

void SimpVert::EnableAdjTriFlags(int f)
{
	for (auto it = adjTris.begin(); it != adjTris.end(); ++it)
	{
		(*it)->EnableFlags(f);
	}
}

void SimpVert::DisableAdjTriFlags(int f)
{
	for (auto it = adjTris.begin(); it != adjTris.end(); ++it)
	{
		(*it)->DisableFlags(f);
	}
}

void SimpVert::FindAdjacentVerts(std::vector<SimpVert*>& adjVerts)
{
	for (auto it = adjTris.begin(); it != adjTris.end(); ++it)
	{
		for (int j = 0; j < 3; ++j)
		{
			SimpVert* v = (*it)->verts[j];
			if (v != this)
			{
				// add unique
				bool exist = false;
				for (auto val : adjVerts)
				{
					if (val == v)
					{
						exist = true;
						break;
					}
				}
				if (!exist) adjVerts.push_back(v);
			}
		}
	}
}

void SimpVert::EnableFlagsGroup(int f)
{
	SimpVert* v = this;
	do 
	{
		v->EnableFlags(f);
		v = v->next;
	} while (v != this);
}

void SimpVert::DisableFlagsGroup(int f)
{
	SimpVert* v = this;
	do
	{
		v->DisableFlags(f);
		v = v->next;
	} while (v != this);
}

void SimpVert::EnableAdjVertFlagsGroup(int f)
{
	SimpVert* v = this;
	do
	{
		v->EnableAdjVertFlags(f);
		v = v->next;
	} while (v != this);
}

void SimpVert::DisableAdjVertFlagsGroup(int f)
{
	SimpVert* v = this;
	do
	{
		v->DisableAdjVertFlags(f);
		v = v->next;
	} while (v != this);
}

void SimpVert::EnableAdjTriFlagsGroup(int f)
{
	SimpVert* v = this;
	do
	{
		v->EnableAdjTriFlags(f);
		v = v->next;
	} while (v != this);
}

void SimpVert::DisableAdjTriFlagsGroup(int f)
{
	SimpVert* v = this;
	do
	{
		v->DisableAdjTriFlags(f);
		v = v->next;
	} while (v != this);
}

void SimpVert::FindAdjacentVertsGroup(std::vector<SimpVert*>& adjVerts)
{
	SimpVert* v = this;
	do
	{
		for (auto it = adjTris.begin(); it != adjTris.end(); ++it)
		{
			for (int j = 0; j < 3; ++j)
			{
				SimpVert* tmp = (*it)->verts[j];
				if (tmp != v)
				{
					//add unique
					bool exist = false;
					for (auto val : adjVerts)
					{
						if (val == tmp)
						{
							exist = true;
							break;
						}
					}
					if (!exist) adjVerts.push_back(tmp);
				}
			}
		}
		v = v->next;
	} while (v != this);
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

bool SimpTri::ReplaceVertexIsValid(const SimpVert* oldV, const Vector3& pos) const
{
	check(oldV);
	check(oldV == verts[0] || oldV == verts[1] || oldV == verts[2]);

	unsigned int k;
	if (oldV == verts[0])
		k = 0;
	else if (oldV == verts[1])
		k = 1;
	else
		k = 2;

	const Vector3& v0 = verts[k]->GetPos();
	const Vector3& v1 = verts[k = (1 << k) & 3]->GetPos();
	const Vector3& v2 = verts[k = (1 << k) & 3]->GetPos();

	const Vector3 d21 = v2 - v1;
	const Vector3 d01 = v0 - v1;
	const Vector3 dp1 = pos - v1;

	Vector3 n0 = d01.cross(d21);
	Vector3 n1 = dp1.cross(d21);

	return n0.dot(n1) > 0.0f;
}

void SimpTri::ReplaceVertex(SimpVert* oldV, SimpVert* newV)
{
	check(oldV && newV);
	check(oldV == verts[0] || oldV == verts[1] || oldV == verts[2]);
	check(newV != verts[0] && newV != verts[1] && newV != verts[2]);

	if (oldV == verts[0])
		verts[0] = newV;
	else if (oldV == verts[1])
		verts[1] = newV;
	else
		verts[2] = newV;

	check(!HasVertex(oldV));
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