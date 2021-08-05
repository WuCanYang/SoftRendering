#include "Simplifier.h"
#include "MeshSimplifyElements.h"



MeshSimplifier::MeshSimplifier(const MeshVertType* InSrcVerts, const unsigned int InNumSrcVerts,
	const unsigned int* InSrcIndexes, const unsigned int InNumSrcIndexes,
	const float VolumeImportanceValue, const bool VolumeConservation, const bool bEnforceBoundaries):
	VolumeImportance(VolumeImportanceValue),
	bPreserveVolume(VolumeConservation),
	bCheckBoneBoundaries(bEnforceBoundaries),
	meshManager(InSrcVerts, InNumSrcVerts, InSrcIndexes, InNumSrcIndexes)
{
	int BasicAttrNum = MeshVertType::NumBaseAttributes();
	BasicAttrWeights.Reset();
	check(BasicAttrNum == BasicAttrWeights.size());
	for (int i = 0; i < BasicAttrNum; ++i)
	{
		BasicAttrWeights[i] = 1.0f;
	}

	const int NumEdges = meshManager.TotalNumEdges();

	CollapseCostHeap.Resize(NumEdges, NumEdges);

	quadricCache.RegisterMesh(meshManager);
}

void MeshSimplifier::InitCost()
{
	const unsigned int NumEdge = meshManager.TotalNumEdges();
	for (int i = 0; i < NumEdge; ++i)
	{
		SimpEdge* edgePtr = meshManager.GetEdgePtr(i);
		double cost = ComputeEdgeCollapseCost(edgePtr);
		CollapseCostHeap.Add(cost, i);
	}
}

double MeshSimplifier::ComputeEdgeCollapseCost(SimpEdge* edge)
{
	if (edge->v0->TestFlags(SIMP_LOCKED) && edge->v1->TestFlags(SIMP_LOCKED))
	{
		return FLT_MAX;
	}

	EdgeVertTupleArray EdgeVerts;
	double cost = ComputeEdgeCollapseVertsAndCost(edge, EdgeVerts);	

	const Vector3 newPos = std::get<2>(EdgeVerts[0]).GetPos();
	SimpVert* u = edge->v0;
	SimpVert* v = edge->v1;

	//penalty
	double penalty = 0.;

	//�ڽ�����������̫�࣬��ʱ��̮�������ߵ�Ӱ��Ƚϴ󣬼���һ���ͷ�
	{
		unsigned int degree = meshManager.GetDegree(u);
		degree += meshManager.GetDegree(v);

		if (degree > DegreeLimit)
		{
			penalty += (degree - DegreeLimit) * DegreePenalty;
		}
	}


	//�����ͷ��ļ��㣬���һ��������������Ӱ��Ĺ�����һ������ô�ͼ���һ���ͷ�
	if (bCheckBoneBoundaries)
	{

	}


	//�����ζ����滻������������һ���ͷ�
	{
		const double penaltyToPreventEdgeFolding = invalidPenalty;
		float SpecialWeight = 0.0f;
		v->EnableAdjTriFlagsGroup(SIMP_MARK1);

		SimpVert* vert = u;
		do 
		{
			SpecialWeight = std::max(SpecialWeight, vert->vert.SpecializedWeight);		//���Ҳ�͹����й�
			for (auto it = vert->adjTris.begin(); it != vert->adjTris.end(); ++it)
			{
				SimpTri* tri = *it;
				if (!tri->TestFlags(SIMP_MARK1))
				{
					if (!tri->ReplaceVertexIsValid(vert, newPos))
					{
						penalty += penaltyToPreventEdgeFolding;
					}
				}
				tri->DisableFlags(SIMP_MARK1);
			}
			vert = vert->next;
		} while (vert != u);


		vert = v;
		do 
		{
			SpecialWeight = std::max(SpecialWeight, vert->vert.SpecializedWeight);
			for (auto it = vert->adjTris.begin(); it != vert->adjTris.end(); ++it)
			{
				SimpTri* tri = *it;
				if (tri->TestFlags(SIMP_MARK1))
				{
					if (!tri->ReplaceVertexIsValid(vert, newPos))
					{
						penalty += penaltyToPreventEdgeFolding;
					}
				}
				tri->DisableFlags(SIMP_MARK1);
			}
			vert = vert->next;
		} while (vert != v);

		penalty += SpecialWeight;
	}
	

	return cost + penalty;
}

double MeshSimplifier::ComputeEdgeCollapseVertsAndCost(SimpEdge* edge, EdgeVertTupleArray& newVerts)
{
	check(newVerts.size() == 0);

	EdgeQuadric edgeQuadric;
	std::vector<WedgeQuadric> WedgeQuadricArray;

	ComputeEdgeCollapseVertsAndQuadrics(edge, newVerts, edgeQuadric, WedgeQuadricArray);

	double cost = edgeQuadric.Evaluate(std::get<2>(newVerts[0]).GetPos());
	int size = WedgeQuadricArray.size();
	for (int i = 0; i < size; ++i)
	{
		MeshVertType& Vert = std::get<2>(newVerts[i]);

		cost += WedgeQuadricArray[i].Evaluate(Vert, BasicAttrWeights);
	}
	return cost;
}

void MeshSimplifier::ComputeEdgeCollapseVertsAndQuadrics(SimpEdge* edge, EdgeVertTupleArray& newVerts, EdgeQuadric& newEdgeQuadric, std::vector<WedgeQuadric>& newQuadrics)
{
	Vector3 newPos = ComputeEdgeCollapseVertsPos(edge, newVerts, newQuadrics, newEdgeQuadric);

	for (int i = 0; i < newQuadrics.size(); ++i)
	{
		MeshVertType& Vert = std::get<2>(newVerts[i]);
		Vert.GetPos() = newPos;

		if (newQuadrics[i].TotalArea() > 1.e-6)
		{
			newQuadrics[i].CalcAttributes(Vert, BasicAttrWeights);
			Vert.Correct();
		}
	}
}

Vector3 MeshSimplifier::ComputeEdgeCollapseVertsPos(SimpEdge* edge, EdgeVertTupleArray& newVerts, std::vector<WedgeQuadric>& quadrics, EdgeQuadric& edgeQuadric)
{
	check(newVerts.size() == 0);
	check(quadrics.size() == 0);
	edgeQuadric.Zero();

	SimpEdge* e;
	SimpVert* v;
	
	edge->v0->EnableFlagsGroup(SIMP_MARK1);
	edge->v1->EnableFlagsGroup(SIMP_MARK1);

//------------------------------------Wedge Quadric
	e = edge;
	do 
	{
		check(e == meshManager.FindEdge(e->v0, e->v1));
		check(e->v0->adjTris.size() > 0);
		check(e->v1->adjTris.size() > 0);
		check(e->v0->GetMaterialIndex() == e->v1->GetMaterialIndex());

		newVerts.push_back(std::make_tuple(e->v0, e->v1, e->v1->vert));

		WedgeQuadric quadric = GetWedgeQuadric(e->v0);
		quadric += GetWedgeQuadric(e->v1);
		quadrics.push_back(quadric);

		e->v0->DisableFlags(SIMP_MARK1);
		e->v1->DisableFlags(SIMP_MARK1);

		e = e->next;
	} while (e != edge);

	v = edge->v0;
	do
	{
		if (v->TestFlags(SIMP_MARK1))
		{
			newVerts.push_back(std::make_tuple(v, nullptr, v->vert));

			WedgeQuadric quadric = GetWedgeQuadric(v);
			quadrics.push_back(quadric);

			v->DisableFlags(SIMP_MARK1);
		}
		v = v->next;
	} while (v != edge->v0);

	v = edge->v1;
	do 
	{
		if (v->TestFlags(SIMP_MARK1))
		{
			newVerts.push_back(std::make_tuple(nullptr, v, v->vert));

			WedgeQuadric quadric = GetWedgeQuadric(v);
			quadrics.push_back(quadric);

			v->DisableFlags(SIMP_MARK1);
		}
		v = v->next;
	} while (v != edge->v1);
//-------------------------------------Edge Quadric

	v = edge->v0;
	do 
	{
		edgeQuadric += GetEdgeQuadric(v);
		v = v->next;
	} while (v != edge->v0);

	v = edge->v1;
	do 
	{
		edgeQuadric += GetEdgeQuadric(v);
		v = v->next;
	} while (v != edge->v1);

//-------------------------------------���¶����λ��

	QuadricOptimizer optimizer;
	optimizer.AddEdgeQuadric(edgeQuadric);
	for (int i = 0; i < quadrics.size(); ++i)
	{
		optimizer.AddWedgeQuadric(quadrics[i]);
	}

	Vector3 newPos;
	if (edge->v0->TestFlags(SIMP_LOCKED))
	{
		newPos = edge->v0->GetPos();
	}
	else if (edge->v1->TestFlags(SIMP_LOCKED))
	{
		newPos = edge->v1->GetPos();
	}
	else
	{
		Vector3d optPos;
		bool valid = optimizer.Optimize(optPos, bPreserveVolume, VolumeImportance);
		if (valid)
		{
			newPos._x = optPos[0];
			newPos._y = optPos[1];
			newPos._z = optPos[2];
		}
		else
		{
			newPos = (edge->v0->GetPos() + edge->v1->GetPos()) * 0.5f;
		}

		//newPos = (edge->v0->GetPos() + edge->v1->GetPos()) * 0.5f;
	}

	return newPos;
}

void MeshSimplifier::ComputeEdgeCollapseVertsAndFixBones(SimpEdge* edge, EdgeVertTupleArray& newVerts)
{
	ComputeEdgeCollapseVerts(edge, newVerts);

	// Positions of the two edges
	const SimpVert* Vert0 = edge->v0;
	const SimpVert* Vert1 = edge->v1;

	const Vector3 Pos0 = Vert0->vert.GetPos();
	const Vector3 Pos1 = Vert1->vert.GetPos();

	// Position of the collapsed vert

	const Vector3 CollapsedPos = std::get<2>(newVerts[0]).GetPos();

	// Find edge endpoint that is closest to the collapsed vert location.

	const float DstSqr0 = (CollapsedPos - Pos0).length();
	const float DstSqr1 = (CollapsedPos - Pos1).length();

	const auto& ClosestSimpliferVert = (DstSqr1 < DstSqr0) ? Vert1->vert : Vert0->vert;

	const auto& SrcBones = ClosestSimpliferVert.GetSparseBones();
	const int MasterVertIndex = ClosestSimpliferVert.MasterVertIndex;
	const int NumNewVerts = newVerts.size();

	for (int i = 0; i < NumNewVerts; ++i)
	{
		MeshVertType& simpVert = std::get<2>(newVerts[i]);
		simpVert.SparseBones = SrcBones;
		simpVert.MasterVertIndex = MasterVertIndex;
	}
}

void MeshSimplifier::ComputeEdgeCollapseVerts(SimpEdge* edge, EdgeVertTupleArray& newVerts)
{
	check(newVerts.size() == 0);

	EdgeQuadric edgeQuadric;
	std::vector<WedgeQuadric> WedgeQuadricArray;

	ComputeEdgeCollapseVertsAndQuadrics(edge, newVerts, edgeQuadric, WedgeQuadricArray);
}

void MeshSimplifier::UpdateEdgeCollapseCost(std::vector<SimpEdge*>& DirtyEdges)
{
	uint32 NumEdges = DirtyEdges.size();
	// update edges
	for (uint32 i = 0; i < NumEdges; i++)
	{
		SimpEdge* edge = DirtyEdges[i];

		if (edge->TestFlags(SIMP_REMOVED))
			continue;

		double cost = ComputeEdgeCollapseCost(edge);

		SimpEdge* e = edge;
		do {
			uint32 EdgeIndex = meshManager.GetEdgeIndex(e);
			if (CollapseCostHeap.IsPresent(EdgeIndex))
			{
				CollapseCostHeap.Update(cost, EdgeIndex);
			}
			e = e->next;
		} while (e != edge);
	}
}

WedgeQuadric MeshSimplifier::GetWedgeQuadric(SimpVert* v)
{
	const auto QuadricFactory = [this](const SimpTri& tri) -> WedgeQuadric
	{
		return WedgeQuadric(tri.verts[0]->vert, tri.verts[1]->vert, tri.verts[2]->vert, this->BasicAttrWeights);
	};
	return quadricCache.GetWedgeQuadric(v, QuadricFactory);
}

EdgeQuadric MeshSimplifier::GetEdgeQuadric(SimpVert* v)
{
	const double Weight = BoundaryConstraintWeight;
	const auto QuadricFactory = [this, Weight](const Vector3& Pos1, const Vector3& Pos2, const Vector3& Normal) -> EdgeQuadric
	{
		return EdgeQuadric(Pos1, Pos2, Normal, Weight);
	};
	return quadricCache.GetEdgeQuadric(v, QuadricFactory);
}

void MeshSimplifier::DirtyTriQuadricCache(std::vector<SimpTri*>& DirtyTri)
{
	for (SimpTri* tri : DirtyTri)
	{
		quadricCache.DirtyTriQuadric(tri);
	}
}

void MeshSimplifier::DirtyVertAndEdgeQuadricCache(std::vector<SimpVert*>& DirtyVert)
{
	for (SimpVert* vert : DirtyVert)
	{
		quadricCache.DirtyVertQuadric(vert);
		quadricCache.DirtyEdgeQuadric(vert);
	}
}

float MeshSimplifier::SimplifyMesh(SimplifierTerminator Terminator)
{
	InitCost();

	//�����������ô��ĳһ��̮���߲������¶��㵽��Χĳһ����ľ��� ���� Terminator.MaxDistance��ʱ�򣬽�����
	bool CheckDistance = Terminator.MaxDistance < FLT_MAX;


	//��̮������Ӱ��Ķ��㡢�����κͱ�
	std::vector<SimpVert*> DirtyVerts;
	std::vector<SimpTri*>  DirtyTris;
	std::vector<SimpEdge*> DirtyEdges;

	double maxError = 0.0f;
	float  distError = 0.0f;		//����CheckDistance ����ʱ�㵽���������

	while (CollapseCostHeap.Num() > 0)
	{
		unsigned int TopIndex = CollapseCostHeap.Top();
		const double Error = CollapseCostHeap.GetKey(TopIndex);

		const int NumTri = meshManager.ReducedNumTris;
		const int NumVert = meshManager.ReducedNumVerts;
		if (Terminator(NumTri, NumVert, Error) || distError > Terminator.MaxDistance)
		{
			break;
		}

		maxError = std::max(maxError, Error);
		CollapseCostHeap.Pop();

		SimpEdge* TopEdgePtr = meshManager.GetEdgePtr(TopIndex);
		check(TopEdgePtr);

		std::vector<SimpEdge*> CoincidentEdges;
		meshManager.GetEdgesInGroup(TopEdgePtr, CoincidentEdges);
		
		if (meshManager.HasLockedVerts(TopEdgePtr) || meshManager.IsLockedGroup(CoincidentEdges))
		{
			continue;
		}

		meshManager.GetAdjacentTopology(TopEdgePtr, DirtyTris, DirtyVerts, DirtyEdges);

		int CoincidentEdgesNum = CoincidentEdges.size();

		std::vector<unsigned int> InvalidIdxArray;
		int RemovedEdgeNum = meshManager.RemoveEdgeIfInvalid(CoincidentEdges, InvalidIdxArray);
		if (RemovedEdgeNum == CoincidentEdgesNum)
		{
			continue;
		}


		//�����ǰ���Ѿ��Ƴ����߲��Ϸ��ˣ����ڽ������Σ����ʹ�ͬһ��������ѡһ���������
		if (meshManager.IsRemoved(TopEdgePtr) || meshManager.IsInvalid(TopEdgePtr))
		{
			for (SimpEdge* EPtr : CoincidentEdges)
			{
				if (EPtr && !meshManager.IsRemoved(EPtr))
				{
					TopEdgePtr = EPtr;
					break;
				}
			}
		}

		if (meshManager.IsRemoved(TopEdgePtr) || meshManager.IsInvalid(TopEdgePtr)) continue;


		EdgeVertTupleArray EdgeVerts;
		ComputeEdgeCollapseVertsAndFixBones(TopEdgePtr, EdgeVerts);

		if (CheckDistance)	//�����²����ĵ㵽��Χ���������
		{
			float Dist = 0.0f;
			const Vector3 newPos = std::get<2>(EdgeVerts[0]).GetPos();
			for (SimpTri* tri : DirtyTris)
			{
				Vector3 Normal = tri->GetNormal();
				Vector3 Tri2Pos = newPos - tri->verts[0]->GetPos();
				Dist = std::max(Dist, abs(Normal.dot(Tri2Pos)));
			}

			distError = std::max(distError, Dist);
		}


		for (int i = 0, Imax = EdgeVerts.size(); i < Imax; ++i)		//�����������ĵ�����λ��Ϊ�µ�����λ��
		{
			EdgeVertTuple& EdgeV = EdgeVerts[i];
			const MeshVertType& CollapseVert = std::get<2>(EdgeV);

			SimpVert* vert = std::get<0>(EdgeV);
			if (vert)
			{
				vert->vert = CollapseVert;
			}

			vert = std::get<1>(EdgeV);
			if (vert)
			{
				vert->vert = CollapseVert;
			}
		}


		meshManager.UpdateVertexAttriuteIDs(CoincidentEdges);	//�������µĵ������id


		for (int i = 0; i < CoincidentEdgesNum; ++i)	//��̮��
		{
			SimpEdge* EdgePtr = CoincidentEdges[i];

			if(!EdgePtr || meshManager.IsRemoved(EdgePtr)) continue;

			bool collapsed = meshManager.CollapseEdge(EdgePtr, InvalidIdxArray);

			if (collapsed)
			{
				CoincidentEdges[i] = nullptr;
			}

			meshManager.RemoveEdgeIfInvalid(CoincidentEdges, InvalidIdxArray);
		}

		meshManager.MergeGroups(TopEdgePtr->v0, TopEdgePtr->v1);
		meshManager.PropagateFlag<SIMP_LOCKED>(*TopEdgePtr->v1);
		meshManager.PruneVerts<SIMP_REMOVED>(*TopEdgePtr->v1);


		DirtyTriQuadricCache(DirtyTris);
		DirtyVertAndEdgeQuadricCache(DirtyVerts);

		//ɾ���˻���ͼ�Σ����������α����һ���ߣ��߱����һ����
		meshManager.RemoveIfDegenerate(DirtyTris);

		meshManager.RemoveIfDegenerate(DirtyVerts);

		meshManager.RemoveIfDegenerate(DirtyEdges, InvalidIdxArray);

		meshManager.RebuildEdgeLinkLists(DirtyEdges);

		const int InvalidNum = InvalidIdxArray.size();
		for (int i = 0; i < InvalidNum; ++i)
		{
			CollapseCostHeap.Remove(InvalidIdxArray[i]);
		}

		UpdateEdgeCollapseCost(DirtyEdges);

		DirtyVerts.clear();
		DirtyEdges.clear();
		DirtyTris.clear();
		
	}

	meshManager.RemoveDegenerateTris();
	meshManager.RemoveDegenerateVerts();

	return CheckDistance ? distError : maxError;

/*
	// Build the cost heap
InitCost();

// Do the distance check?
const bool bCheckDistance = (Terminator.MaxDistance < FLT_MAX);

std::vector<SimpTri*>  DirtyTris;
std::vector<SimpVert*> DirtyVerts;
std::vector<SimpEdge*> DirtyEdges;

double maxError = 0.0f;
float  distError = 0.0f;

while (CollapseCostHeap.Num() > 0)
{


	// get the next vertex to collapse
	uint32 TopIndex = CollapseCostHeap.Top();

	const double error = CollapseCostHeap.GetKey(TopIndex);
	//std::cout << error << std::endl;

	// Check for termination.
	{
		const int numTris = meshManager.ReducedNumTris;
		const int numVerts = meshManager.ReducedNumVerts;
		if (Terminator(numTris, numVerts, error) || distError > Terminator.MaxDistance)
		{
			break;
		}
	}

	maxError = std::max(maxError, error);

	CollapseCostHeap.Pop();

	// Pointer to the candidate edge (link list) for collapse
	SimpEdge* TopEdgePtr = meshManager.GetEdgePtr(TopIndex);
	check(TopEdgePtr);


	// Gather all the edges that are really in this group.
	std::vector<SimpEdge*> CoincidentEdges;
	meshManager.GetEdgesInGroup(TopEdgePtr, CoincidentEdges);

	if (meshManager.HasLockedVerts(TopEdgePtr) || meshManager.IsLockedGroup(CoincidentEdges))
	{
		// this edge shouldn't be collapsed.
		continue;
	}

	// Before changing any of the mesh topology, we capture lists
	// of the tris, verts, and edges that may need new quadrics
	// computed to updated edge collapse values. 
	meshManager.GetAdjacentTopology(TopEdgePtr, DirtyTris, DirtyVerts, DirtyEdges);

	const int NumCoincidentEdges = CoincidentEdges.size();

	// Remove any edges in from this group that happen to be degenerate (no adjacent triangles)
	// and capture the Idx of the dead edges.  Also removes the edges from the edge-index heap.
	std::vector<unsigned int> InvalidCostIdxArray;  // Keep track of the Idx to remove from the cost heap
	const int NumRemovedEdges = meshManager.RemoveEdgeIfInvalid(CoincidentEdges, InvalidCostIdxArray);

	// if none of the edges in this group were valid, just continue.
	if (NumCoincidentEdges == NumRemovedEdges)
	{
		continue;
	}

	// The representative edge in our edge group was removed.  Replace it with another from the group that is valid.
	if (meshManager.IsRemoved(TopEdgePtr) || meshManager.IsInvalid(TopEdgePtr))
	{
		// try to find a valid edge in the batch.. 
		for (SimpEdge* EPtr : CoincidentEdges)
		{
			if (EPtr && !meshManager.IsRemoved(EPtr))
			{
				TopEdgePtr = EPtr;
				break;
			}
		}
	}

	// continue if no edge actually exists.
	if (meshManager.IsRemoved(TopEdgePtr) || meshManager.IsInvalid(TopEdgePtr))
	{
		continue;
	}

	// update Edge->v1 to new locations :  move verts to new verts
	{

		// Copy the  edge->v1->verts and update the location & attributes
		// capturing the corresponding SimpVert.
		// The VertexUpdate Array is built by adding 
		// 1) the two vertices for each edge
		// 2) Elements from TopEdge->v0 vert group not already added
		// 3) Elements from TopEdge->v1 vert group not already added
		EdgeVertTupleArray VertexUpdateArray;
		ComputeEdgeCollapseVertsAndFixBones(TopEdgePtr, VertexUpdateArray); // re-targets using closest bone

		// Compute the distance of the new vertex to each plane of the affected triangles. 

		if (bCheckDistance)
		{
			const Vector3 NewPos = std::get<2>(VertexUpdateArray[0]).GetPos();

			float Dist = 0.f;
			for (SimpTri* Tri : DirtyTris)
			{
				Vector3 TriNorm = Tri->GetNormal();
				Vector3 PosToTri = NewPos - Tri->verts[0]->GetPos();
				Dist = std::max(abs(TriNorm.dot(PosToTri)), Dist);
			}

			distError = std::max(distError, Dist);

		}


		// Update both verts in the mesh to have the new location and attribute values.

		for (int i = 0, Imax = VertexUpdateArray.size(); i < Imax; ++i)
		{
			EdgeVertTuple& EdgeUpdate = VertexUpdateArray[i];


			// New vertex attribute values for the collapsed vertex 
			const MeshVertType& CollapsedAttributes = std::get<2>(EdgeUpdate);

			// update the first edge vertex
			SimpVert* VtxPtr = std::get<0>(EdgeUpdate);
			if (VtxPtr != nullptr)
			{
				VtxPtr->vert = CollapsedAttributes;
			}

			// update the second edge vertex
			VtxPtr = std::get<1>(EdgeUpdate);
			if (VtxPtr != nullptr)
			{
				VtxPtr->vert = CollapsedAttributes;
			}

		}
	}

	// This manages the complicated logic of making sure the attribute element IDs after the collapse
	// will be in the correct state for a collapse of of a split attribute or non-split attribute.  This applies
	// to the vertices in the edges, and the loose vertices that don't share a triangle
	// with the a vertex on the opposite end of the edge.

	meshManager.UpdateVertexAttriuteIDs(CoincidentEdges);

	// collapse all edges by moving edge->v0 to edge->v1
	{

		// All positions and attributes should be fixed now, but we need to update the
		// mesh connectivity. 
		for (int i = 0; i < NumCoincidentEdges; ++i)
		{
			SimpEdge* EdgePtr = CoincidentEdges[i];

			bool bSkip = !EdgePtr || meshManager.IsRemoved(EdgePtr);

			if (bSkip) continue;

			// Collapse the edge, delete triangles that become degenerate
			// and update any edges that used to include v0 to include v1 now.
			// V1 acquires any lock from v0
			bool bCollapsed = meshManager.CollapseEdge(EdgePtr, InvalidCostIdxArray);

			// this edge has been collapsed.  Remove it from the CoincidentEdges array
			if (bCollapsed)
			{
				CoincidentEdges[i] = NULL;
			}

			// NB: two edges in this group may have shared a single vertex. 
			// so the above collapse could have also collapsed other edges in the group.
			meshManager.RemoveEdgeIfInvalid(CoincidentEdges, InvalidCostIdxArray);

		}

		// add v0 remainder verts to v1

		{
			// I'm not totally okay with this.. this adds all the v0 verts to the v1 group,
			// resulting in v1, v1', v1''.. v0, v0', ..
			// but doesn't change their Position()
			meshManager.MergeGroups(TopEdgePtr->v0, TopEdgePtr->v1);

			// if any of the verts in the group is locked, make them all locked
			meshManager.PropagateFlag<SIMP_LOCKED>(*TopEdgePtr->v1);

			// prune any SIMP_REMOVED verts from the v1 link list.
			meshManager.PruneVerts<SIMP_REMOVED>(*TopEdgePtr->v1);
		}
	}

	// Dirty the quadric cache 
	// NB: these early out if the objects are marked as removed.
	// i think that is too clever and should be changed so they
	// always dirty everything in the list.
	//DirtyTriQuadricCache(DirtyTris);
	//DirtyVertAndEdgeQuadricsCache(DirtyVerts);


	// If a dirty tri has zero area, remove it and tag as SIMP_REMOVED
	meshManager.RemoveIfDegenerate(DirtyTris);


	// Tag verts that aren't part of tris ad SIMP_REMOVED
	// and remove them from the mesh vert groups.
	meshManager.RemoveIfDegenerate(DirtyVerts);

	// Remove edges with out verts and update
	// the cost & heap for all the remaining dirty edges.
	// this triggers updates of the tri and vert cached quadrics.
	meshManager.RemoveIfDegenerate(DirtyEdges, InvalidCostIdxArray);

	// If an edge collapses on a triangle, the other two edges are now
	// one.. this accounts for that sort of thing.
	meshManager.RebuildEdgeLinkLists(DirtyEdges);

	// Update the cost heap by removing the dead edges we just pruned.
	int NumRemoved = InvalidCostIdxArray.size();
	for (int i = 0; i < NumRemoved; ++i)
	{
		CollapseCostHeap.Remove(InvalidCostIdxArray[i]);
	}
	// update the Quadric Caches associated with the dirty edges 
	// and update the heap.
	UpdateEdgeCollapseCost(DirtyEdges);

	DirtyTris.clear();
	DirtyVerts.clear();
	DirtyEdges.clear();
}

// remove degenerate triangles
// not sure why this happens
int NumTrisIJustRemoved = meshManager.RemoveDegenerateTris();

// remove orphaned verts
int NumVertsIJustRemoved = meshManager.RemoveDegenerateVerts();


return (bCheckDistance) ? distError : maxError;*/

}

void MeshSimplifier::OutputMesh(MeshVertType* Verts, unsigned int* Indexes, bool MergeCoincidentVertBones /*= true*/, bool WeldVtxColorAttrs /*= true*/, std::vector<int>* LockedVerts /*= nullptr*/)
{
	if (MergeCoincidentVertBones)		//ȷ��ͬһ��λ�õĶ��� �ܵ��Ĺ���Ӱ������ͬ��
	{
		std::vector<SimpVert*> CoincidentVertGroups;
		meshManager.GetCoincidentVertGroups(CoincidentVertGroups);

		int size = CoincidentVertGroups.size();
		for (int i = 0; i < size; ++i)
		{
			SimpVert* vert = CoincidentVertGroups[i];
			const auto& SparseBone = vert->vert.SparseBones;
			SimpVert* tmp = vert->next;
			while (tmp != vert)
			{
				tmp->vert.SparseBones = SparseBone;
				tmp = tmp->next;
			}
		}
	}

	if (WeldVtxColorAttrs)		//��ͬһ������ ������ͬID�Ķ��������ȡƽ��
	{
		meshManager.WeldNonSplitBasicAttributes(MeshManager::VtxElementWeld::Color);
	}

	meshManager.OutputMesh(Verts, Indexes, LockedVerts);
}
