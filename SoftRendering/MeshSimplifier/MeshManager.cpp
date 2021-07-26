#include "MeshManager.h"
#include "MeshSimplifyElements.h"
#include <iostream>

MeshManager::MeshManager(const MeshVertType* InSrcVerts, const unsigned int InNumSrcVerts, 
	const unsigned int* InSrcIndexes, const unsigned int InNumSrcIndexes):
	NumSrcVerts(InNumSrcVerts),
	NumSrcTris(InNumSrcIndexes / 3),
	ReducedNumVerts(InNumSrcVerts),
	ReducedNumTris(InNumSrcIndexes / 3)
{
	VertArray.clear();
	TriArray.clear();

	VertArray.resize(NumSrcVerts);
	TriArray.resize(NumSrcTris);

	for (int i = 0; i < NumSrcVerts; ++i)
	{
		VertArray[i].vert = InSrcVerts[i];
	}

	for(int i = 0; i < NumSrcTris; ++i)
	{
		int offset = i * 3;
		for (int j = 0; j < 3; ++j)
		{
			int index = offset + j;
			int VertIndex = InSrcIndexes[index];

			TriArray[i].verts[j] = &VertArray[VertIndex];
		}
	}

	for (int i = 0; i < NumSrcTris; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			TriArray[i].verts[j]->adjTris.push_back(&TriArray[i]);
		}
	}

	GroupVerts(VertArray);

	SetAttributeIDS(VertArray);

	MakeEdges(VertArray, NumSrcTris, EdgeArray);

	GroupEdges(EdgeArray);

	for (unsigned int i = 0; i < EdgeArray.size(); ++i)
	{
		unsigned int hashValue = HashEdge(EdgeArray[i].v0, EdgeArray[i].v1);
		EdgeVertIdHashMap.insert(std::make_pair(hashValue, i));
	}
}

void MeshManager::GetVertsInGroup(const SimpVert& seedVert, std::vector<SimpVert*>& InOutVertGroup) const
{
	SimpVert* Vert = const_cast<SimpVert*>(&seedVert);
	SimpVert* v = Vert;
	do 
	{
		InOutVertGroup.push_back(v);
		v = v->next;
	} while (v != Vert);
}

void MeshManager::GroupVerts(std::vector<SimpVert>& Verts)
{
	int NumVerts = Verts.size();

	std::unordered_multimap<unsigned int, unsigned int> HashTable;
	std::vector<unsigned int> HashValues(NumVerts);

	for (int i = 0; i < NumVerts; ++i)
	{
		HashValues[i] = HashPoint(Verts[i].GetPos());
		HashTable.insert(std::make_pair(HashValues[i], i));
	}

	for (int i = 0; i < NumVerts; ++i)
	{
		SimpVert* v1 = &Verts[i];

		if(v1->next == v1) continue;

		const unsigned int hash = HashValues[i];
		auto range = HashTable.equal_range(hash);
		auto& start = range.first;
		auto& end = range.second;
		for (; start != end; ++start)
		{
			SimpVert* v2 = &Verts[start->second];
			if(v1 == v2) continue;

			if (v1->GetPos() == v2->GetPos())
			{
				check(v2->next == v2);
				check(v2->prev == v2);

				v2->next = v1->next;
				v2->next->prev = v2;

				v2->prev = v1;
				v1->next = v2;
			}
		}
	}
}

void MeshManager::SetAttributeIDS(std::vector<SimpVert>& Verts)
{

	const int NumVerts = Verts.size();

	// set element ids on the verts.

	std::vector<SimpVert*> CoincidentSimpVerts;
	int NormalID = 0;
	int TangentID = 0;
	int BiTangetID = 0;
	int ColorID = 0;
	int UVIDs[MAX_TEXCOORDS];
	for (int i = 0; i < MAX_TEXCOORDS; ++i) UVIDs[i] = 0;

	for (int i = 0; i < NumVerts; ++i)
	{
		CoincidentSimpVerts.clear();
		SimpVert& HeadSimpVert = Verts[i];
		auto& HeadVertAttrs = HeadSimpVert.vert.BasicAttributes;

		// already processed this vert ( and all the coincident verts with it).
		if (HeadVertAttrs.ElementIDs.ColorID != -1) continue;

		// give this vert the next available element Id
		HeadVertAttrs.ElementIDs.NormalID = NormalID++;
		HeadVertAttrs.ElementIDs.TangentID = TangentID++;
		HeadVertAttrs.ElementIDs.BiTangentID = BiTangetID++;
		HeadVertAttrs.ElementIDs.ColorID = ColorID++;

		for (int t = 0; t < MAX_TEXCOORDS; ++t)
		{
			HeadVertAttrs.ElementIDs.TexCoordsID[t] = UVIDs[t]++;
		}

		// collect the verts in the linklist - they share the same location.
		GetVertsInGroup(HeadSimpVert, CoincidentSimpVerts);

		// The HeadSimpVert is the j=0 one..
		for (int j = 1; j < CoincidentSimpVerts.size(); ++j)
		{
			SimpVert* ActiveSimpVert = CoincidentSimpVerts[j];
			auto& ActiveVertAttrs = ActiveSimpVert->vert.BasicAttributes;
			check(ActiveVertAttrs.ElementIDs.NormalID == -1);

			// If normals match another vert at this location, they should share ID
			for (int k = 0; k < j; ++k)
			{
				SimpVert* ProcessedSimpVert = CoincidentSimpVerts[k];
				auto& ProcessedVertAttrs = ProcessedSimpVert->vert.BasicAttributes;

				if (ProcessedVertAttrs.Normal == ActiveVertAttrs.Normal)
				{
					ActiveVertAttrs.ElementIDs.NormalID = ProcessedVertAttrs.ElementIDs.NormalID;
					break;
				}
			}
			// If normal didn't mach with any earlier vert at this location. Give new ID
			if (ActiveVertAttrs.ElementIDs.NormalID == -1)
			{
				ActiveVertAttrs.ElementIDs.NormalID = NormalID++;
			}

			// If tangents match another vert at this location, they should share ID
			for (int k = 0; k < j; ++k)
			{
				SimpVert* ProcessedSimpVert = CoincidentSimpVerts[k];
				auto& ProcessedVertAttrs = ProcessedSimpVert->vert.BasicAttributes;

				if (ProcessedVertAttrs.Tangent == ActiveVertAttrs.Tangent)
				{
					ActiveVertAttrs.ElementIDs.TangentID = ProcessedVertAttrs.ElementIDs.TangentID;
					break;
				}

			}
			// If tangent didn't mach with any earlier vert at this location. Give new ID
			if (ActiveVertAttrs.ElementIDs.TangentID == -1)
			{
				ActiveVertAttrs.ElementIDs.TangentID = TangentID++;
			}

			// If Bitangents match another vert at this location, they should share ID
			for (int k = 0; k < j; ++k)
			{
				SimpVert* ProcessedSimpVert = CoincidentSimpVerts[k];
				auto& ProcessedVertAttrs = ProcessedSimpVert->vert.BasicAttributes;

				if (ProcessedVertAttrs.BiTangent == ActiveVertAttrs.BiTangent)
				{
					ActiveVertAttrs.ElementIDs.BiTangentID = ProcessedVertAttrs.ElementIDs.BiTangentID;
					break;
				}
			}
			// If BiTangent didn't mach with any earlier vert at this location. Give new ID
			if (ActiveVertAttrs.ElementIDs.BiTangentID == -1)
			{
				ActiveVertAttrs.ElementIDs.BiTangentID = BiTangetID++;
			}

			// If Color match another vert at this location, they should share ID
			for (int k = 0; k < j; ++k)
			{
				SimpVert* ProcessedSimpVert = CoincidentSimpVerts[k];
				auto& ProcessedVertAttrs = ProcessedSimpVert->vert.BasicAttributes;

				if (ProcessedVertAttrs.Color == ActiveVertAttrs.Color)
				{
					ActiveVertAttrs.ElementIDs.ColorID = ProcessedVertAttrs.ElementIDs.ColorID;
					break;
				}
			}
			//If  Color didn't mach with any earlier vert at this location. Give new ID
			if (ActiveVertAttrs.ElementIDs.ColorID == -1)
			{
				ActiveVertAttrs.ElementIDs.ColorID = ColorID++;
			}

			for (int t = 0; t < MAX_TEXCOORDS; ++t)
			{
				// look for texture match
				for (int k = 0; k < j; ++k)
				{
					SimpVert* ProcessedSimpVert = CoincidentSimpVerts[k];
					auto& ProcessedVertAttrs = ProcessedSimpVert->vert.BasicAttributes;

					if (ProcessedVertAttrs.TexCoords[t] == ActiveVertAttrs.TexCoords[t])
					{
						ActiveVertAttrs.ElementIDs.TexCoordsID[t] = ProcessedVertAttrs.ElementIDs.TexCoordsID[t];

						break;
					}
				}
				// If TexCoord didn't mach with any earlier vert at this location. Give new ID
				if (ActiveVertAttrs.ElementIDs.TexCoordsID[t] == -1)
				{
					ActiveVertAttrs.ElementIDs.TexCoordsID[t] = UVIDs[t]++;
				}
			}

		}
	}

	// @todo Maybe add one pass at to split element bow-ties.  
	// These could have formed along a UV seam if a both sides of the seam shared the same value at a isolated vertex - but in that case a split vertex might not have been generated..

}

void MeshManager::MakeEdges(const std::vector<SimpVert>& Verts, const int NumTris, std::vector<SimpEdge>& Edges)
{
	const int NumVerts = Verts.size();

	int maxEdgeSize = std::min(3 * NumTris, 3 * NumVerts - 6);
	Edges.clear();
	for (int i = 0; i < NumVerts; i++)
	{
		AppendConnectedEdges(&Verts[i], Edges);
	}

	// Guessed wrong on num edges. Array was resized so fix up pointers.

	if (Edges.size() > maxEdgeSize)
	{

		for (int i = 0; i < Edges.size(); ++i)
		{
			SimpEdge& edge = Edges[i];
			edge.next = &edge;
			edge.prev = &edge;
		}
	}
}

void MeshManager::AppendConnectedEdges(const SimpVert* Vert, std::vector<SimpEdge>& Edges)
{
	SimpVert* v = const_cast<SimpVert*>(Vert);

	std::vector<SimpVert*> adjVerts;
	v->FindAdjacentVerts(adjVerts);

	SimpVert* v0 = v;
	for (SimpVert* v1 : adjVerts)
	{
		if (v0 < v1)	//以地址存放顺序来保证边不重复
		{
			Edges.push_back(SimpEdge());
			SimpEdge& edge = Edges.back();
			edge.v0 = v0;
			edge.v1 = v1;
		}
	}
}

void MeshManager::GroupEdges(std::vector<SimpEdge>& Edges)
{
	int NumEdges = Edges.size();
	
	std::unordered_multimap<unsigned int, unsigned int> HashTable;
	std::vector<unsigned int> HashValues(NumEdges);

	for (int i = 0; i < NumEdges; ++i)
	{
		unsigned int Hash0 = HashPoint(Edges[i].v0->GetPos());
		unsigned int Hash1 = HashPoint(Edges[i].v1->GetPos());
		HashValues[i] = Murmur32({ std::min(Hash0, Hash1), std::max(Hash0, Hash1) });

		HashTable.insert(std::make_pair(HashValues[i], i));
	}

	for (int i = 0; i < NumEdges; ++i)
	{
		SimpEdge* e1 = &Edges[i];
		if (e1->next == e1) continue;

		unsigned int hash = HashValues[i];
		auto range = HashTable.equal_range(hash);
		auto& start = range.first;
		auto& end = range.second;
		for (; start != end; ++start)
		{
			SimpEdge* e2 = &Edges[start->second];
			if(e1 == e2) continue;

			bool m1 =
				(e1->v0 == e2->v0 || e1->v0->GetPos() == e2->v0->GetPos()) &&
				(e1->v1 == e2->v1 || e1->v1->GetPos() == e2->v1->GetPos());

			bool m2 =
				(e1->v0 == e2->v1 || e1->v0->GetPos() == e2->v1->GetPos()) &&
				(e1->v1 == e2->v0 || e1->v1->GetPos() == e2->v0->GetPos());

			if (m2)
			{
				SimpVert* tmp = e2->v0;
				e2->v0 = e2->v1;
				e2->v1 = tmp;
			}

			if (m1 || m2)
			{
				check(e2->next == e2);
				check(e2->prev == e2);

				e2->next = e1->next;
				e2->prev = e1;
				e2->next->prev = e2;
				e2->prev->next = e2;
			}
		}
	}
}
