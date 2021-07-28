#include "Simplifier.h"
#include "MeshSimplifyElements.h"

extern enum SimpElementFlags;

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
	if (edge->v0->TestFlags(SIMP_REMOVED) && edge->v1->TestFlags(SIMP_REMOVED))
	{
		return FLT_MAX;
	}

	EdgeVertTupleArray EdgeVerts;
	double cost = ComputeEdgeCollapseVertsAndCost(edge, EdgeVerts);

	//penalty



	return cost;
}

double MeshSimplifier::ComputeEdgeCollapseVertsAndCost(SimpEdge* edge, EdgeVertTupleArray& newVerts)
{
	check(newVerts.size() == 0);

	EdgeQuadric edgeQuadric;
	std::vector<WedgeQuadric> WedgeQuadricArray;

	ComputeEdgeCollapseVertsAndQuadrics(edge, newVerts, edgeQuadric, WedgeQuadricArray);

	double cost = edgeQuadric.Evaluate(std::get<2>(newVerts[0]).GetPos());
	int size = newVerts.size();
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


}

Vector3 MeshSimplifier::ComputeEdgeCollapseVertsPos(SimpEdge* edge, EdgeVertTupleArray& newVerts, std::vector<WedgeQuadric>& quadrics, EdgeQuadric& edgeQuadric)
{
	return Vector3();
}

float MeshSimplifier::SimplifyMesh()
{
	return 0;
}
