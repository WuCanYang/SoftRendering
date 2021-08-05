#include "MeshManager.h"
#include "MeshSimplifyElements.h"
#include <iostream>
#include <algorithm>

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

			check(index < InNumSrcIndexes);
			check(VertIndex < InNumSrcVerts);

			TriArray[i].verts[j] = &VertArray[VertIndex];
		}
	}

	for (int i = 0; i < NumSrcTris; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			SimpTri* TriPtr = &TriArray[i];
			SimpVert* VertPtr = TriPtr->verts[j];

			VertPtr->adjTris.push_back(TriPtr);
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

		if(v1->next != v1) continue;

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

	Edges.clear();
	for (int i = 0; i < NumVerts; i++)
	{
		AppendConnectedEdges(&Verts[i], Edges);
	}

	for (int i = 0; i < Edges.size(); ++i)
	{
		SimpEdge& edge = Edges[i];
		edge.next = &edge;
		edge.prev = &edge;
	}
}

void MeshManager::AppendConnectedEdges(const SimpVert* Vert, std::vector<SimpEdge>& Edges)
{
	SimpVert* v = const_cast<SimpVert*>(Vert);
	check(v->adjTris.size() > 0);

	std::vector<SimpVert*> adjVerts;
	v->FindAdjacentVerts(adjVerts);

	SimpVert* v0 = v;
	for (SimpVert* v1 : adjVerts)
	{
		if (v0 < v1)	//以地址存放顺序来保证边不重复
		{
			check(v0->GetMaterialIndex() == v1->GetMaterialIndex());

			Edges.emplace_back();
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
		if (Edges[i].next != &Edges[i])
		{
			continue;
		}

		unsigned int hash = HashValues[i];
		auto range = HashTable.equal_range(hash);
		auto& start = range.first;
		auto& end = range.second;
		for (; start != end; ++start)
		{
			SimpEdge* e1 = &Edges[i];
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

void MeshManager::GetAdjacentTopology(const SimpEdge* edge, std::vector<SimpTri*>& DirtyTris, std::vector<SimpVert*>& DirtyVerts, std::vector<SimpEdge*>& DirtyEdges)
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


void MeshManager::GetAdjacentTopology(const SimpVert* vert, std::vector<SimpTri*>& DirtyTris, std::vector<SimpVert*>& DirtyVerts, std::vector<SimpEdge*>& DirtyEdges)
{
	// need this cast because the const version is missing on the vert..

	SimpVert* v = const_cast<SimpVert*>(vert);

	// Gather pointers to all the triangles that share this vert.
	// Update all tris touching collapse edge.
	for (auto it = v->adjTris.begin(); it != v->adjTris.end(); ++it)
	{
		AddUnique(DirtyTris, *it);
	}
	
	// Gather all verts that are adjacent to this one.

	std::vector< SimpVert*> adjVerts;
	v->FindAdjacentVerts(adjVerts);


	// Gather verts that are adjacent to VertPtr
	for (int i = 0, Num = adjVerts.size(); i < Num; i++)
	{
		AddUnique(DirtyVerts, adjVerts[i]);
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

					AddUnique(DirtyEdges, edge);
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
				AddUnique(RemovedEdgeIdxArray, Idx);
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
	SimpVert* v0 = EdgePtr->v0;
	SimpVert* v1 = EdgePtr->v1;

	// Collapse the edge uv by moving vertex v0 onto v1
	check(v0 && v1);
	check(EdgePtr == FindEdge(v0, v1));

	check(v0->adjTris.size() > 0);
	check(v1->adjTris.size() > 0);
	check(v0->GetMaterialIndex() == v1->GetMaterialIndex());



	// Because another edge in the same edge group may share a vertex with this edge
	// and it might have already been collapsed, we can't do this check
	//check(! (v0->TestFlags(SIMP_LOCKED) && v1->TestFlags(SIMP_LOCKED)) );


	// Verify that this is truly an edge of a triangle

	v0->EnableAdjVertFlags(SIMP_MARK1);
	v1->DisableAdjVertFlags(SIMP_MARK1);

	if (v0->TestFlags(SIMP_MARK1))
	{
		// Invalid edge results from collapsing a bridge tri
		// There are no actual triangles connecting these verts
		v0->DisableAdjVertFlags(SIMP_MARK1);

		EdgePtr->EnableFlags(SIMP_REMOVED);
		const unsigned int Idx = RemoveEdge(*EdgePtr);
		if (Idx < UINT32_MAX)
		{
			AddUnique(RemovedEdgeIdxArray, Idx);
		}

		// return false because the was no real edge to collapse
		return false;
	}

	// update edges from v0 to v1

	// Note, the position and other attributes have already been corrected
	// to have the same values.  Here we are just propagating any locked state.
	if (v0->TestFlags(SIMP_LOCKED))
		v1->EnableFlags(SIMP_LOCKED);

	// this version of the vertex will be removed after the collapse
	v0->DisableFlags(SIMP_LOCKED); // we already shared the locked state with the remaining vertex

	// Update 'other'-u edges to 'other'-v edges ( where other != v) 

	for (auto triIter = v0->adjTris.begin(); triIter != v0->adjTris.end(); ++triIter)
	{
		SimpTri* TriPtr = *triIter;
		for (int j = 0; j < 3; j++)
		{
			SimpVert* VertPtr = TriPtr->verts[j];
			if (VertPtr->TestFlags(SIMP_MARK1))
			{

				// replace v0-vert with v1-vert
				// first remove v1-vert if it already exists ( it shouldn't..)
				{
					unsigned int Idx = RemoveEdge(VertPtr, v1);
					if (Idx < UINT32_MAX)
					{
						AddUnique(RemovedEdgeIdxArray, Idx);
					}

					ReplaceVertInEdge(v0, VertPtr, v1);
				}
				VertPtr->DisableFlags(SIMP_MARK1);
			}
		}
	}

	// For faces with verts: v0, v1, other
	// remove the v0-other edges.
	v0->EnableAdjVertFlags(SIMP_MARK1);
	v0->DisableFlags(SIMP_MARK1);
	v1->DisableFlags(SIMP_MARK1);

	for (auto triIter = v1->adjTris.begin(); triIter != v1->adjTris.end(); ++triIter)
	{
		SimpTri* TriPtr = *triIter;
		for (int j = 0; j < 3; j++)
		{
			SimpVert* VertPtr = TriPtr->verts[j];
			if (VertPtr->TestFlags(SIMP_MARK1))
			{
				const unsigned int Idx = RemoveEdge(v0, VertPtr);
				if (Idx < UINT32_MAX)
				{
					AddUnique(RemovedEdgeIdxArray, Idx);
				}
				//
				VertPtr->DisableFlags(SIMP_MARK1);
			}
		}
	}

	v1->DisableAdjVertFlags(SIMP_MARK1);

	// Remove collapsed triangles, and fix-up the others that now use v instead of u triangles

	std::vector<SimpTri*> v0AdjTris;
	{
		unsigned int i = 0;
		v0AdjTris.resize(v0->adjTris.size());

		for (auto triIter = v0->adjTris.begin(); triIter != v0->adjTris.end(); ++triIter)
		{
			v0AdjTris[i] = *triIter;
			i++;
		}
	}

	for (int i = 0, iMax = v0AdjTris.size(); i < iMax; ++i)
	{
		SimpTri* TriPtr = v0AdjTris[i];

		check(!TriPtr->TestFlags(SIMP_REMOVED));
		check(TriPtr->HasVertex(v0));

		if (TriPtr->HasVertex(v1))  // tri shared by v0 and v1.. 
		{
			// delete triangles on edge uv
			ReducedNumTris--;
			RemoveTri(*TriPtr);
		}
		else
		{
			// update triangles to have v1 instead of v0
			ReplaceTriVertex(*TriPtr, *v0, *v1);
		}
	}


	// remove modified verts and tris from cache
	v1->EnableAdjVertFlags(SIMP_MARK1);
	for (auto triIter = v1->adjTris.begin(); triIter != v1->adjTris.end(); ++triIter)
	{
		SimpTri* TriPtr = *triIter;

		for (int i = 0; i < 3; i++)
		{
			SimpVert* VertPtr = TriPtr->verts[i];
			if (VertPtr->TestFlags(SIMP_MARK1))
			{
				VertPtr->DisableFlags(SIMP_MARK1);
			}
		}
	}

	// mark v0 as dead.

	v0->adjTris.clear();	// u has been removed
	v0->EnableFlags(SIMP_REMOVED);

	// Remove the actual edge.
	const unsigned int Idx = RemoveEdge(*EdgePtr);
	if (Idx < UINT32_MAX)
	{
		AddUnique(RemovedEdgeIdxArray, Idx);
	}

	// record the reduced number of verts

	ReducedNumVerts--;

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

int MeshManager::RemoveIfDegenerate(std::vector<SimpTri*>& CandidateTrisPtrArray)
{
	int NumRemoved = 0;
	// remove degenerate triangles
	// not sure why this happens
	for (SimpTri* CandidateTriPtr : CandidateTrisPtrArray)
	{
		if (CandidateTriPtr->TestFlags(SIMP_REMOVED))
			continue;


		const Vector3& p0 = CandidateTriPtr->verts[0]->GetPos();
		const Vector3& p1 = CandidateTriPtr->verts[1]->GetPos();
		const Vector3& p2 = CandidateTriPtr->verts[2]->GetPos();
		const Vector3 n = (p2 - p0).cross(p1 - p0);
		float lenSq = n._x * n._x + n._y * n._y + n._z * n._z;

		if (lenSq == 0.0f)
		{
			NumRemoved++;
			CandidateTriPtr->EnableFlags(SIMP_REMOVED);

			// remove references to tri
			for (int j = 0; j < 3; j++)
			{
				SimpVert* vert = CandidateTriPtr->verts[j];
				vert->adjTris.remove(CandidateTriPtr);
				// orphaned verts are removed below
			}
		}
	}

	ReducedNumTris -= NumRemoved;
	return NumRemoved;
}

int MeshManager::RemoveIfDegenerate(std::vector<SimpVert*>& CandidateVertPtrArray)
{
	int NumRemoved = 0;
	// remove orphaned verts
	for (SimpVert* VertPtr : CandidateVertPtrArray)
	{
		if (VertPtr->TestFlags(SIMP_REMOVED))
			continue;

		if (VertPtr->adjTris.size() == 0)
		{
			NumRemoved++;
			VertPtr->EnableFlags(SIMP_REMOVED);

			// ungroup
			VertPtr->prev->next = VertPtr->next;
			VertPtr->next->prev = VertPtr->prev;
			VertPtr->next = VertPtr;
			VertPtr->prev = VertPtr;
		}
	}

	ReducedNumVerts -= NumRemoved;
	return NumRemoved;
}

int MeshManager::RemoveIfDegenerate(std::vector<SimpEdge*>& CandidateEdges, std::vector<unsigned int>& RemoveEdgeIdxArray)
{
	const unsigned int NumCandidateEdges = CandidateEdges.size();

	// add all grouped edges
	for (unsigned int i = 0; i < NumCandidateEdges; i++)
	{
		SimpEdge* edge = CandidateEdges[i];

		if (edge->TestFlags(SIMP_REMOVED))
			continue;

		SimpEdge* e = edge;
		do {
			AddUnique(CandidateEdges, e);
			e = e->next;
		} while (e != edge);
	}

	// remove dead edges from our edge hash.
	for (unsigned int i = 0, Num = CandidateEdges.size(); i < Num; i++)
	{
		SimpEdge* edge = CandidateEdges[i];

		if (edge->TestFlags(SIMP_REMOVED))
			continue;

		if (edge->v0 == edge->v1)
		{
			edge->EnableFlags(SIMP_REMOVED); // djh 8/3/18.  not sure why this happens

			unsigned int Idx = RemoveEdge(*edge);
			if (Idx < UINT32_MAX)
			{
				AddUnique(RemoveEdgeIdxArray, Idx);
			}
		}
		else if (edge->v0->TestFlags(SIMP_REMOVED) ||
			edge->v1->TestFlags(SIMP_REMOVED))
		{

			unsigned int Idx = RemoveEdge(*edge);
			if (Idx < UINT32_MAX)
			{
				AddUnique(RemoveEdgeIdxArray, Idx);
			}
		}
	}

	return RemoveEdgeIdxArray.size();
}

void MeshManager::RebuildEdgeLinkLists(std::vector<SimpEdge*>& CandidateEdgePtrArray)
{
	int NumEdges = CandidateEdgePtrArray.size();

	for (int i = 0; i < NumEdges; ++i)
	{
		SimpEdge* edge = CandidateEdgePtrArray[i];
		if(edge->TestFlags(SIMP_REMOVED)) continue;

		edge->next = edge;
		edge->prev = edge;
	}

	std::unordered_multimap<unsigned, unsigned> HashTable;

	for (int i = 0; i < NumEdges; ++i)
	{
		SimpEdge* edge = CandidateEdgePtrArray[i];
		if(edge->TestFlags(SIMP_REMOVED)) continue;

		HashTable.insert(std::make_pair(HashEdgePosition(*edge), i));
	}

	for (int i = 0; i < NumEdges; ++i)
	{
		SimpEdge* edge = CandidateEdgePtrArray[i];
		if(edge->TestFlags(SIMP_REMOVED) || edge->next != edge) continue;

		unsigned int HashValue = HashEdgePosition(*edge);
		SimpEdge* e1 = edge;

		auto range = HashTable.equal_range(HashValue);
		auto& start = range.first;
		auto& end = range.second;
		for (; start != end; ++start)
		{
			SimpEdge* e2 = CandidateEdgePtrArray[start->second];

			if (e1 == e2) continue;

			bool m1 =
				(e1->v0 == e2->v0 &&
					e1->v1 == e2->v1)
				||
				(e1->v0->GetPos() == e2->v0->GetPos() &&
					e1->v1->GetPos() == e2->v1->GetPos());

			bool m2 =
				(e1->v0 == e2->v1 &&
					e1->v1 == e2->v0)
				||
				(e1->v0->GetPos() == e2->v1->GetPos() &&
					e1->v1->GetPos() == e2->v0->GetPos());

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

int MeshManager::RemoveDegenerateTris()
{
	std::vector<SimpTri*> TriPtrArray(NumSrcTris);

	for (int i = 0; i < NumSrcTris; ++i)
	{
		TriPtrArray[i] = &TriArray[i];
	}
	return RemoveIfDegenerate(TriPtrArray);
}

int MeshManager::RemoveDegenerateVerts()
{
	std::vector<SimpVert*> VertPtrArray(NumSrcVerts);

	for (int i = 0; i < NumSrcVerts; ++i)
	{
		VertPtrArray[i] = &VertArray[i];
	}
	return RemoveIfDegenerate(VertPtrArray);
}

void MeshManager::FlagBoundary(const SimpElementFlags Flag)
{
	std::vector<SimpVert*> adjVerts;
	if (NumSrcVerts == 0 || NumSrcTris == 0)
	{
		//Avoid trying to compute an empty mesh
		return;
	}

	for (int i = 0; i < NumSrcVerts; i++)
	{

		SimpVert* v0 = &VertArray[i];
		check(v0 != NULL);
		check(v0->adjTris.size() > 0);

		// not sure if this test is valid.  
		if (v0->TestFlags(Flag))
		{
			// we must have visited this vert already in a vert group
			continue;
		}


		//Find all the verts that are adjacent to any vert in this group.
		adjVerts.clear();
		//the scope below replaces  v0->FindAdjacentVertsGroup(adjVerts);
		{
			SimpVert* v = v0;
			do {
				for (auto triIter = v->adjTris.begin(); triIter != v->adjTris.end(); ++triIter)
				{
					for (int j = 0; j < 3; j++)
					{
						SimpVert* TriVert = (*triIter)->verts[j];
						if (TriVert != v)
						{
							AddUnique(adjVerts, TriVert);
						}
					}
				}
				v = v->next;
			} while (v != v0);
		}

		for (SimpVert* v1 : adjVerts)
		{
			if (v0 < v1)
			{

				// set if this edge is boundary
				// find faces that share v0 and v1
				v0->EnableAdjTriFlagsGroup(SIMP_MARK1);
				v1->DisableAdjTriFlagsGroup(SIMP_MARK1);

				int faceCount = 0;
				SimpVert* vert = v0;
				do
				{
					for (auto j = vert->adjTris.begin(); j != vert->adjTris.end(); ++j)
					{
						SimpTri* tri = *j;
						faceCount += tri->TestFlags(SIMP_MARK1) ? 0 : 1;
					}
					vert = vert->next;
				} while (vert != v0);

				// reset v0-group flag.
				v0->DisableAdjTriFlagsGroup(SIMP_MARK1);

				if (faceCount == 1)
				{
					// only one face on this edge
					v0->EnableFlagsGroup(Flag);
					v1->EnableFlagsGroup(Flag);
				}
			}
		}
	}
}

void MeshManager::FlagEdge(std::function<bool(const SimpVert*, const SimpVert*)> IsDifferent, const SimpElementFlags Flag)
{
	int NumEdge = EdgeArray.size();
	for (int i = 0; i < NumEdge; ++i)
	{
		SimpEdge& edge = EdgeArray[i];
		if (IsDifferent(edge.v0, edge.v1))
		{
			edge.EnableFlags(Flag);
			edge.v0->EnableFlags(Flag);
			edge.v1->EnableFlags(Flag); 
		}
	}

	for (int i = 0; i < NumSrcVerts; ++i)
	{
		SimpVert* v1 = &VertArray[i];
		SimpVert* v = v1;
		if (!v || v->TestFlags(Flag) || v->next == v) continue;

		bool AddFlag = false;
		do
		{
			if (IsDifferent(v, v->next))
			{
				v->EnableFlags(Flag);
				AddFlag = true;
			}
			v = v->next;
		} while (v != v1);

		if (AddFlag)
		{
			v = v1;
			do
			{
				v->EnableFlags(Flag);
				v = v->next;
			} while (v != v1);
		}
	}
}

void MeshManager::GetCoincidentVertGroups(std::vector<SimpVert*>& CoincidentVertGroups)
{
	for (int i = 0; i < NumSrcVerts; ++i)
	{
		SimpVert* vert = &VertArray[i];

		if (vert->TestFlags(SIMP_REMOVED) || (vert->next == vert && vert->prev == vert))
		{
			continue;
		}

		SimpVert* tmp = vert;
		SimpVert* maxVert = vert;
		while (tmp->next != vert)
		{
			tmp = tmp->next;
			if (tmp > maxVert && !tmp->TestFlags(SIMP_REMOVED))
			{
				maxVert = tmp;
			}
		}
		AddUnique(CoincidentVertGroups, maxVert);
	}
}

void MeshManager::WeldNonSplitBasicAttributes(VtxElementWeld WeldType)
{
	// Gather the split-vertex groups.  
	std::vector<SimpVert*> CoincidentVertGroups;
	GetCoincidentVertGroups(CoincidentVertGroups);

	// For each split group, weld the attributes that have the same element ID
	int NumCoincidentVertGroups = CoincidentVertGroups.size();

	for (int i = 0; i < NumCoincidentVertGroups; ++i)
	{
		SimpVert* HeadVert = CoincidentVertGroups[i];

		if (!HeadVert) continue;

		// Get the verts that are in this group.
		std::vector<SimpVert*> VertGroup;
		GetVertsInGroup(*HeadVert, VertGroup);

		int  NumVertsInGroup = VertGroup.size();

		// reject any groups that weren't really split.
		if (NumVertsInGroup < 2) continue;


		// functor that partitions attributes by attribute ID and welds attributes in a partition to the average value.
		auto Weld = [&VertGroup, NumVertsInGroup](auto& IDAccessor, auto& ValueAccessor, auto ZeroValue)
		{
			// container used to sort coincident vert by ID.  
			// used to find groups with same ID to weld together.

			std::vector<VertAndID> VertAndIDArray;
			for (SimpVert* v : VertGroup)
			{
				VertAndIDArray.emplace_back(v, IDAccessor(v));
			}
			// sort by ID
			sort(VertAndIDArray.begin(), VertAndIDArray.end(), [](const VertAndID& A, const VertAndID& B)
				{
					return A.ID < B.ID;
				});

			// find and process the partitions.

			int PartitionStart = 0;
			while (PartitionStart < NumVertsInGroup)
			{
				auto AveValue = ZeroValue;
				int PartitionElID = VertAndIDArray[PartitionStart].ID;
				int PartitionEnd = NumVertsInGroup;
				for (int n = PartitionStart; n < NumVertsInGroup; ++n)
				{
					if (VertAndIDArray[n].ID == PartitionElID)
					{
						AveValue += ValueAccessor(VertAndIDArray[n].SrcVert);
					}
					else
					{
						PartitionEnd = n;
						break;
					}
				}
				AveValue /= (PartitionEnd - PartitionStart);
				for (int n = PartitionStart; n < PartitionEnd; ++n)
				{
					ValueAccessor(VertAndIDArray[n].SrcVert) = AveValue;
				}

				PartitionStart = PartitionEnd;
			}

		};

		// Weld Normals with the same NormalID.
		if (WeldType == VtxElementWeld::Normal)
		{
			auto NormalIDAccessor = [](SimpVert* SimpVert)->int
			{
				return SimpVert->vert.BasicAttributes.ElementIDs.NormalID;
			};
			auto NormalValueAccessor = [](SimpVert* SimpVert)->Vector3&
			{
				return SimpVert->vert.BasicAttributes.Normal;
			};

			Vector3 ZeroValue(0, 0, 0);
			Weld(NormalIDAccessor, NormalValueAccessor, ZeroValue);
		}
		// Weld Tangents with same TangentID
		if (WeldType == VtxElementWeld::Tangent)
		{

			auto TangentIDAccessor = [](SimpVert* SimpVert)->int
			{
				return SimpVert->vert.BasicAttributes.ElementIDs.TangentID;
			};
			auto TangentValueAccessor = [](SimpVert* SimpVert)->Vector3&
			{
				return SimpVert->vert.BasicAttributes.Tangent;
			};

			Vector3 ZeroValue(0, 0, 0);
			Weld(TangentIDAccessor, TangentValueAccessor, ZeroValue);
		}
		// Weld BiTangent with same BiTangentID
		if (WeldType == VtxElementWeld::BiTangent)
		{
			auto BiTangentIDAccessor = [](SimpVert* SimpVert)->int
			{
				return SimpVert->vert.BasicAttributes.ElementIDs.BiTangentID;
			};
			auto BiTangentValueAccessor = [](SimpVert* SimpVert)->Vector3&
			{
				return SimpVert->vert.BasicAttributes.BiTangent;
			};

			Vector3 ZeroValue(0, 0, 0);
			Weld(BiTangentIDAccessor, BiTangentValueAccessor, ZeroValue);
		}
		// Weld Color with same ColorID
		if (WeldType == VtxElementWeld::Color)
		{

			auto ColorIDAccessor = [](SimpVert* SimpVert)->int
			{
				return SimpVert->vert.BasicAttributes.ElementIDs.ColorID;
			};
			auto ColorValueAccessor = [](SimpVert* SimpVert)->Vector4&
			{
				return SimpVert->vert.BasicAttributes.Color;
			};

			Vector4 ZeroValue;
			Weld(ColorIDAccessor, ColorValueAccessor, ZeroValue);
		}
		// Weld UVs with same TexCoordsID
		if (WeldType == VtxElementWeld::UV)
		{
			int NumTexCoords = MAX_TEXCOORDS;
			for (int t = 0; t < NumTexCoords; ++t)
			{

				auto TexCoordIDAccessor = [t](SimpVert* SimpVert)->int
				{
					return SimpVert->vert.BasicAttributes.ElementIDs.TexCoordsID[t];
				};
				auto TexCoordValueAccessor = [t](SimpVert* SimpVert)->Vector2&
				{
					return SimpVert->vert.BasicAttributes.TexCoords[t];
				};

				Vector2 ZeroValue(0, 0);
				Weld(TexCoordIDAccessor, TexCoordValueAccessor, ZeroValue);
			}
		}

		// After welding, need to "correct" the vert to make sure the vector attributes are normalized etc.
		for (SimpVert* v : VertGroup)
		{
			v->vert.Correct();
		}
	}
}

void MeshManager::OutputMesh(MeshVertType* Verts, unsigned int* Indexes, std::vector<int>* LockedVerts /*= nullptr*/)
{
	int ValidVerts = 0;
	for (int i = 0; i < NumSrcVerts; ++i)
	{
		ValidVerts += VertArray[i].TestFlags(SIMP_REMOVED) ? 0 : 1;
	}
	check(ValidVerts <= ReducedNumVerts);

	std::unordered_multimap<unsigned int, unsigned int> HashTable;
	int NumV = 0;
	int NumI = 0;

	for (int i = 0; i < NumSrcTris; ++i)
	{
		if(TriArray[i].TestFlags(SIMP_REMOVED)) continue;

		for (int j = 0; j < 3; ++j)
		{
			SimpVert* vert = TriArray[i].verts[j];
			check(!vert->TestFlags(SIMP_REMOVED));
			check(vert->adjTris.size() != 0);

			unsigned int HashValue = HashPoint(vert->GetPos());
			unsigned int Idx = UINT32_MAX;

			auto range = HashTable.equal_range(HashValue);
			auto& start = range.first;
			auto& end = range.second;
			for (; start != end; ++start)
			{
				if (vert->vert == Verts[start->second])
				{
					Idx = start->second;
					break;
				}
			}

			if (Idx == UINT32_MAX)
			{
				if (LockedVerts && vert->TestFlags(SIMP_LOCKED))
				{
					LockedVerts->push_back(NumV);
				}

				HashTable.insert(std::make_pair(HashValue, NumV));
				Verts[NumV] = vert->vert;
				Indexes[NumI++] = NumV;
				++NumV;
			}
			else
			{
				Indexes[NumI++] = Idx;
			}
		}
	}
}
