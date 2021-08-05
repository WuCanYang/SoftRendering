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

	//邻接三角形数量太多，这时候坍缩这条边的影响比较大，加上一个惩罚
	{
		unsigned int degree = meshManager.GetDegree(u);
		degree += meshManager.GetDegree(v);

		if (degree > DegreeLimit)
		{
			penalty += (degree - DegreeLimit) * DegreePenalty;
		}
	}


	//骨骼惩罚的计算，如果一条边两个顶点受影响的骨骼不一样，那么就加上一个惩罚
	if (bCheckBoneBoundaries)
	{

	}


	//三角形顶点替换不合理所给的一个惩罚
	{
		const double penaltyToPreventEdgeFolding = invalidPenalty;
		float SpecialWeight = 0.0f;
		v->EnableAdjTriFlagsGroup(SIMP_MARK1);

		SimpVert* vert = u;
		do 
		{
			SpecialWeight = std::max(SpecialWeight, vert->vert.SpecializedWeight);		//这个也和骨骼有关
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

//-------------------------------------求新顶点的位置

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

	//如果开启，那么当某一次坍缩边产生的新顶点到周围某一个面的距离 大于 Terminator.MaxDistance的时候，结束简化
	bool CheckDistance = Terminator.MaxDistance < FLT_MAX;


	//边坍缩后受影响的顶点、三角形和边
	std::vector<SimpVert*> DirtyVerts;
	std::vector<SimpTri*>  DirtyTris;
	std::vector<SimpEdge*> DirtyEdges;

	double maxError = 0.0f;
	float  distError = 0.0f;		//保存CheckDistance 开启时点到面的最大距离

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


		//如果当前边已经移除或者不合法了（无邻接三角形），就从同一个组里面选一条边来替代
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

		if (CheckDistance)	//保存新产生的点到周围面的最大距离
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


		for (int i = 0, Imax = EdgeVerts.size(); i < Imax; ++i)		//更新留下来的点属性位置为新的属性位置
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


		meshManager.UpdateVertexAttriuteIDs(CoincidentEdges);	//更新留下的点的属性id


		for (int i = 0; i < CoincidentEdgesNum; ++i)	//边坍缩
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

		//删除退化的图形，比如三角形变成了一条边，边变成了一个点
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
}

void MeshSimplifier::OutputMesh(MeshVertType* Verts, unsigned int* Indexes, bool MergeCoincidentVertBones /*= true*/, bool WeldVtxColorAttrs /*= true*/, std::vector<int>* LockedVerts /*= nullptr*/)
{
	if (MergeCoincidentVertBones)		//确保同一个位置的顶点 受到的骨骼影响是相同的
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

	if (WeldVtxColorAttrs)		//把同一个组下 具有相同ID的顶点的属性取平均
	{
		meshManager.WeldNonSplitBasicAttributes(MeshManager::VtxElementWeld::Color);
	}

	meshManager.OutputMesh(Verts, Indexes, LockedVerts);
}
