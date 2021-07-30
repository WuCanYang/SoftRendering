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


unsigned int MeshManager::RemoveEdge(SimpEdge& edge)
{
	edge.prev->next = edge.next;
	edge.next->prev = edge.prev;

	edge.next = &edge;
	edge.prev = &edge;

	unsigned int Idx = GetEdgeIndex(&edge);

	if (edge.TestFlags(SIMP_REMOVED))
	{
		Idx = UINT32_MAX;
	}
	else
	{

		// mark as removed
		edge.EnableFlags(SIMP_REMOVED);

		unsigned int Hash = HashEdge(edge.v0, edge.v1);
		
		auto range = EdgeVertIdHashMap.equal_range(Hash);
		auto& start = range.first;
		auto& end = range.second;
		while (start != end)
		{
			if (start->second == Idx)
			{
				EdgeVertIdHashMap.erase(start);
				break;
			}
			++start;
		}
	}
	// return the Idx
	return Idx;
}

unsigned int MeshManager::RemoveEdge(const SimpVert* VertAPtr, const SimpVert* VertBPtr)
{
	auto HashAndIdx = GetEdgeHashPair(VertAPtr, VertBPtr);

	unsigned int Idx = HashAndIdx.first;
	// Early out if this edge doesn't exist.
	if (Idx == UINT32_MAX)
	{
		return Idx;
	}

	SimpEdge& Edge = EdgeArray[Idx];
	if (Edge.TestFlags(SIMP_REMOVED))
	{
		Idx = UINT32_MAX;
	}
	else
	{
		// mark as removed
		Edge.EnableFlags(SIMP_REMOVED);

		auto range = EdgeVertIdHashMap.equal_range(HashAndIdx.second);
		auto& start = range.first;
		auto& end = range.second;
		while (start != end)
		{
			if (start->second == Idx)
			{
				EdgeVertIdHashMap.erase(start);
				break;
			}
			++start;
		}
	}

	// remove this edge from its edge group
	Edge.prev->next = Edge.next;
	Edge.next->prev = Edge.prev;

	Edge.next = &Edge;
	Edge.prev = &Edge;

	// return the Idx
	return Idx;
}

void MeshManager::GetAdjacentTopology(const SimpEdge* edge, std::vector<SimpTri*> DirtyTris, std::vector<SimpVert*> DirtyVerts, std::vector<SimpEdge*> DirtyEdges)
{
	const SimpVert* v = edge->v0;
	do
	{
		GetAdjacentTopology(v, DirtyTris, DirtyVerts, DirtyEdges);
		v = v->next;
	} while (v != edge->v0);

	v = edge->v1;
	do
	{
		GetAdjacentTopology(v, DirtyTris, DirtyVerts, DirtyEdges);
		v = v->next;
	} while (v != edge->v1);
}


void MeshManager::GetAdjacentTopology(const SimpVert* vert, std::vector<SimpTri*> DirtyTris, std::vector<SimpVert*> DirtyVerts, std::vector<SimpEdge*> DirtyEdges)
{
	// need this cast because the const version is missing on the vert..

	SimpVert* v = const_cast<SimpVert*>(vert);

	// Gather pointers to all the triangles that share this vert.

	// Update all tris touching collapse edge.
	for (auto it = v->adjTris.begin(); it != v->adjTris.end(); ++it)
	{
		bool exist = false;
		for (auto t : DirtyTris)
		{
			if (t == *it)
			{
				exist = true;
				break;
			}
		}

		if (!exist) DirtyTris.push_back(*it);
	}

	// Gather all verts that are adjacent to this one.

	std::vector< SimpVert*> adjVerts;
	v->FindAdjacentVerts(adjVerts);


	// Gather verts that are adjacent to VertPtr
	for (int i = 0, Num = adjVerts.size(); i < Num; i++)
	{
		bool exist = false;
		for (auto t : DirtyVerts)
		{
			if (t == adjVerts[i])
			{
				exist = true;
				break;
			}
		}

		if (!exist) DirtyVerts.push_back(adjVerts[i]);
	}



	// Gather verts that are adjacent to VertPtr
	for (int i = 0, Num = adjVerts.size(); i < Num; i++)
	{
		adjVerts[i]->EnableFlags(SIMP_MARK2);
	}

	// update the costs of all edges connected to any face adjacent to v
	for (int i = 0, iMax = adjVerts.size(); i < iMax; ++i)
	{
		SimpVert* AdjVert = adjVerts[i];
		AdjVert->EnableAdjVertFlags(SIMP_MARK1);

		for (auto triIter = AdjVert->adjTris.begin(); triIter != AdjVert->adjTris.end(); ++triIter)
		{
			SimpTri* tri = *triIter;
			for (int k = 0; k < 3; k++)
			{
				SimpVert* vert = tri->verts[k];
				if (vert->TestFlags(SIMP_MARK1) && !vert->TestFlags(SIMP_MARK2) && vert != AdjVert)
				{
					SimpEdge* edge = FindEdge(AdjVert, vert);

					bool exist = false;
					for (auto t : DirtyEdges)
					{
						if (t == edge)
						{
							exist = true;
							break;
						}
					}

					if (!exist) DirtyEdges.push_back(edge);
				}
				vert->DisableFlags(SIMP_MARK1);
			}
		}
		AdjVert->DisableFlags(SIMP_MARK2);
	}
}

int MeshManager::RemoveEdgeIfInvalid(std::vector<SimpEdge*>& CandidateEdges, std::vector<unsigned int>& RemovedEdgeIdxArray)
{
	const unsigned int NumCandidateEdges = CandidateEdges.size();
	for (unsigned int i = 0; i < NumCandidateEdges; ++i)
	{

		SimpEdge* EdgePtr = CandidateEdges[i];

		// Edge has already been removed..
		if (!EdgePtr) continue;


		// Verify the edge has an adjacent face.
		auto HasAdjacentFace = [](SimpEdge* e)->bool
		{
			// Verify that this is truly an edge of a triangle

			e->v0->EnableAdjVertFlags(SIMP_MARK1);
			e->v1->DisableAdjVertFlags(SIMP_MARK1);

			if (e->v0->TestFlags(SIMP_MARK1))
			{
				// Invalid edge results from collapsing a bridge tri
				// There are no actual triangles connecting these verts
				e->v0->DisableAdjVertFlags(SIMP_MARK1);

				return false;
			}

			return true;
		};

		// remove invalid faces from the hash, edge group and flag.
		if (IsInvalid(EdgePtr) || !HasAdjacentFace(EdgePtr)) // one of the verts touches zero triangles
		{
			// unlinks the edge from the edge group, remove from edge hash, add removed flag.

			const unsigned int Idx = RemoveEdge(*EdgePtr);

			// Record the index of the edge we remove.

			if (Idx < UINT32_MAX)
			{
				bool exist = false;
				for (auto val : RemovedEdgeIdxArray)
				{
					if (val == Idx)
					{
						exist = true;
						break;
					}
				}
				if (!exist) RemovedEdgeIdxArray.push_back(Idx);
			}
			CandidateEdges[i] = NULL;
		}
	}

	return RemovedEdgeIdxArray.size();
}

void MeshManager::UpdateVertexAttriuteIDs(std::vector<SimpEdge*>& InCoincidentEdges)
{

	// some of the edges may be null / removed.  Need to filter these out.

	std::vector<SimpEdge*> CoincidentEdges;
	for (SimpEdge* EdgePtr : InCoincidentEdges)
	{
		if (!EdgePtr || !EdgePtr->v0 || !EdgePtr->v1 || IsRemoved(EdgePtr))
		{
			continue;
		}

		CoincidentEdges.push_back(EdgePtr);
	}


	int NumCoincidentEdges = CoincidentEdges.size();

	if (NumCoincidentEdges == 0)
	{
		return;
	}

	if (NumCoincidentEdges > 2)
	{
		// this is the collapse of a non-manifold edge.  Not going to track the element Ids in this badness. 
		return;
	}

	// Update the Attribute element IDs on the loose v0 verts (since v0 collapses onto v1)
	// Loose verts updated any non-split element IDs to those of the v1.
	if (NumCoincidentEdges == 1)
	{

		SimpEdge* EdgePtr = CoincidentEdges[0];


		auto v0IDs = EdgePtr->v0->vert.BasicAttributes.ElementIDs;
		auto v1IDs = EdgePtr->v1->vert.BasicAttributes.ElementIDs;

		// loop over the v0 loose verts and update any non-split element IDs to v1ID.
		SimpVert* v0 = EdgePtr->v0;
		SimpVert* vCurrent = v0;
		while (vCurrent->next != v0)
		{
			vCurrent = vCurrent->next;
			// create a mask that is zero where the elements are the same as the v0 elements.
			// those elements should be merged with their v1 counterparts, retaining the v1 ids. 
			auto MaskIDs = vCurrent->vert.BasicAttributes.ElementIDs - v0IDs;

			// copy element ids from v1IDs to vCurrent where the mask is off (zero)
			vCurrent->vert.BasicAttributes.ElementIDs.MaskedCopy(MaskIDs, v1IDs);
		}

	}

	if (NumCoincidentEdges == 2)
	{
		// the edge is split.  There are two verts at each location
		SimpVert* v0[2];
		SimpVert* v1[2];

		for (int i = 0; i < 2; ++i)
		{
			SimpEdge* EdgePtr = CoincidentEdges[i];

			v0[i] = EdgePtr->v0;
			v1[i] = EdgePtr->v1;
		}

		// force attrs that are split on v0 to be split on v1.  If an attribute is split on v0 and not on v1
		// then this copies the ids from v0 to v1 for this attribute.
		// Why do this?  When an attribute seam starts on a mesh, one vertex may have a split and the other not.
		// This can even happen in the middle of a seam, for example to neighboring UV patches might agree at a single
		// point.
		//  An Example helps:  Say color ID is split on the v0 vertex but not the v1 vertex
		//      v0[0] ColorID = 7,  v0[1] ColorID =8       v1[0] ColorID = 11, v1[1] ColorID = 11    
		//
		//      this should result in copying that split to the v1 vertex.  =>  v1[0] ColorID =7  v1[1] ColorID = 8
		//
		if (v1[0] != v1[1] && v0[0] != v0[1]) // only apply this to the case of two distinct edges.
		{
			// Identify the elements that are not split on the v0 vertex (zeros of mask)
			auto v0MaskIDs = v0[0]->vert.BasicAttributes.ElementIDs - v0[1]->vert.BasicAttributes.ElementIDs;

			// Identify the elements that are not split on the v1 vertex ( zero of mask )
			auto v1MaskIDs = v1[0]->vert.BasicAttributes.ElementIDs - v1[1]->vert.BasicAttributes.ElementIDs;

			// Copy values from v0 IDs where v1Mask is off (zero) and v0 mask is on (non-zero)
			v1[0]->vert.BasicAttributes.ElementIDs.MaskedCopy(v1MaskIDs, v0MaskIDs, v0[0]->vert.BasicAttributes.ElementIDs);
			v1[1]->vert.BasicAttributes.ElementIDs.MaskedCopy(v1MaskIDs, v0MaskIDs, v0[1]->vert.BasicAttributes.ElementIDs);
		}

		// Update the Attribute element IDs on the loose v0 verts (since v0 collapses onto v1)
		// Loose verts updated any non-split element IDs to those of the v1.
		for (int i = 0; i < 2; ++i)
		{
			SimpEdge* EdgePtr = CoincidentEdges[i];
			auto v0IDs = v0[i]->vert.BasicAttributes.ElementIDs;
			auto v1IDs = v1[i]->vert.BasicAttributes.ElementIDs;

			SimpVert* vCurrent = v0[i];
			while (vCurrent->next != v0[i])
			{
				vCurrent = vCurrent->next;
				if (vCurrent != v0[0] && vCurrent != v0[1]) // only look at the v0 loose verts
				{
					// create a mask where the elements are the same
					auto MaskIDs = vCurrent->vert.BasicAttributes.ElementIDs - v0IDs;

					// copy elements from V1IDs to tmp where the mask is off (zero)
					vCurrent->vert.BasicAttributes.ElementIDs.MaskedCopy(MaskIDs, v1IDs);
				}
			}
		}

	}

}

bool MeshManager::CollapseEdge(SimpEdge* EdgePtr, std::vector<unsigned int>& RemovedEdgeIdxArray)
{
	return true;
}

unsigned int MeshManager::ReplaceVertInEdge(const SimpVert* VertAPtr, const SimpVert* VertBPtr, SimpVert* VertAprimePtr)
{
	std::pair<unsigned int, unsigned int> HashAndIdx = GetEdgeHashPair(VertAPtr, VertBPtr);
	const unsigned int Idx = HashAndIdx.first;
	const unsigned int HashValue = HashAndIdx.second;

	check(Idx != UINT32_MAX);
	SimpEdge* edge = &EdgeArray[Idx];

	auto range = EdgeVertIdHashMap.equal_range(HashValue);
	auto& start = range.first;
	auto& end = range.second;
	while (start != end)
	{
		if (start->second == Idx)
		{
			EdgeVertIdHashMap.erase(start);
			break;
		}
		++start;
	}

	EdgeVertIdHashMap.insert(std::make_pair(HashEdge(VertAprimePtr, VertBPtr), Idx));

	if (edge->v0 == VertAPtr)
		edge->v0 = VertAprimePtr;
	else
		edge->v1 = VertAprimePtr;

	return Idx;
}
