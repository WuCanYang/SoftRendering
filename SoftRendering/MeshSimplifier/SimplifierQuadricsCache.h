#pragma once
#include "SimplifierQuadrics.h"
#include <functional>

class QuadricCache
{


public:

	WedgeQuadric GetWedgeQuadric(SimpVert* v, std::function<WedgeQuadric(const SimpTri&)> QuadricFactory)
	{
		WedgeQuadric vertQuadric;

		for (auto it = v->adjTris.begin(); it != v->adjTris.end(); ++it)
		{
			SimpTri* tri = *it;

			WedgeQuadric quadric = QuadricFactory(*tri);
			vertQuadric += quadric;
		}
		return vertQuadric;
	}

	EdgeQuadric GetEdgeQuadric(SimpVert* v, std::function<EdgeQuadric(const Vector3&, const Vector3&, const Vector3&)> QuadricFactory)
	{
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

		return edgeQuadric;
	}

};