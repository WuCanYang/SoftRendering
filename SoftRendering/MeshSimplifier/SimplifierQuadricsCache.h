#pragma once
#include "SimplifierQuadrics.h"
#include <functional>
#include <vector>

class QuadricCache		//保存已计算的Quadric来进行加速
{
private:

	std::vector<bool>		  VertQuadricsValid;
	std::vector<WedgeQuadric> VertQuadrics;

	std::vector<bool>		  TriQuadricsValid;
	std::vector<WedgeQuadric> TriQuadrics;

	std::vector<bool>		  EdgeQuadricsValid;
	std::vector<EdgeQuadric>  EdgeQuadrics;

	const SimpVert* VertArray;
	const SimpTri* TriArray;

private:

	void RegisterCache(const int NumTris, const int NumVert)
	{
		VertQuadricsValid.resize(NumVert);
		VertQuadrics.resize(NumVert);

		TriQuadricsValid.resize(NumTris);
		TriQuadrics.resize(NumTris);

		EdgeQuadricsValid.resize(NumVert);
		EdgeQuadrics.resize(NumVert);
	}

	unsigned int GetVertIndex(const SimpVert* vert) const
	{
		return (unsigned int)(vert - VertArray);
	}

	unsigned int GetTriIndex(const SimpTri* tri) const
	{
		return (unsigned int)(tri - TriArray);
	}

public:

	void RegisterMesh(MeshManager& mesh)
	{
		VertArray = &mesh.VertArray[0];
		TriArray = &mesh.TriArray[0];

		RegisterCache(mesh.NumSrcTris, mesh.NumSrcVerts);
	}

	void DirtyVertQuadric(const SimpVert* v)
	{
		unsigned int idx = GetVertIndex(v);
		VertQuadricsValid[idx] = false;
	}

	void DirtyTriQuadric(const SimpTri* tri)
	{
		unsigned int idx = GetTriIndex(tri);
		TriQuadricsValid[idx] = false;
	}

	void DirtyEdgeQuadric(const SimpVert* v)
	{
		unsigned int idx = GetVertIndex(v);
		EdgeQuadricsValid[idx] = false;
	}


	WedgeQuadric GetWedgeQuadric(SimpVert* v, std::function<WedgeQuadric(const SimpTri&)> QuadricFactory)
	{
		unsigned int VertIndex = GetVertIndex(v);
		if (VertQuadricsValid[VertIndex])
		{
			return VertQuadrics[VertIndex];
		}


		WedgeQuadric vertQuadric;

		for (auto it = v->adjTris.begin(); it != v->adjTris.end(); ++it)
		{
			SimpTri* tri = *it;
			unsigned int TriIndex = GetTriIndex(tri);
			if (TriQuadricsValid[TriIndex])
			{
				vertQuadric += TriQuadrics[TriIndex];
			}
			else
			{
				WedgeQuadric quadric = QuadricFactory(*tri);
				vertQuadric += quadric;

				TriQuadricsValid[TriIndex] = true;
				TriQuadrics[TriIndex] = quadric;
			}
		}

		VertQuadricsValid[VertIndex] = true;
		VertQuadrics[VertIndex] = vertQuadric;
		return vertQuadric;
	}

	EdgeQuadric GetEdgeQuadric(SimpVert* v, std::function<EdgeQuadric(const Vector3&, const Vector3&, const Vector3&)> QuadricFactory)
	{
		unsigned int VertIndex = GetVertIndex(v);
		if (EdgeQuadricsValid[VertIndex])
		{
			return EdgeQuadrics[VertIndex];
		}


		EdgeQuadric edgeQuadric;
		edgeQuadric.Zero();

		std::vector<SimpVert*> adjVerts;
		v->FindAdjacentVerts(adjVerts);

		v->EnableAdjTriFlags(SIMP_MARK1);

		for (SimpVert* vert : adjVerts)
		{
			SimpTri* face = nullptr;
			int count = 0;
			for (auto it = vert->adjTris.begin(); it != vert->adjTris.end(); ++it)
			{
				SimpTri* tri = *it;
				if (tri->TestFlags(SIMP_MARK1))
				{
					face = tri;
					++count;
				}
			}
			if (count == 1)
			{
				edgeQuadric += QuadricFactory(v->GetPos(), vert->GetPos(), face->GetNormal());
			}
		}
		v->DisableAdjTriFlags(SIMP_MARK1);

		EdgeQuadricsValid[VertIndex] = true;
		EdgeQuadrics[VertIndex] = edgeQuadric;
		return edgeQuadric;
	}

};