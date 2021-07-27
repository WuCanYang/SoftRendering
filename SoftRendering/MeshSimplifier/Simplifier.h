#pragma once
#include "MeshManager.h"
#include "BinaryHeap.h"

class MeshSimplifier
{
	typedef typename std::tuple<SimpVert*, SimpVert*, MeshVertType>		EdgeVertTuple;
	typedef	typename std::vector<EdgeVertTuple>							EdgeVertTupleArray;

public:

	MeshManager			meshManager;

	std::vector<int>	BasicAttrWeights;


	FBinaryHeap<double>	CollapseCostHeap;

	double               coAlignmentLimit = .0871557f;

	double               VolumeImportance;
	bool                 bPreserveVolume;
	bool                 bCheckBoneBoundaries;


	unsigned int		 DegreeLimit = 24;
	double               DegreePenalty = 100.0f;

	double               invalidPenalty = 1.e6;

	double               BoundaryConstraintWeight = 256.;



public:

	MeshSimplifier(const MeshVertType* InSrcVerts, const unsigned int InNumSrcVerts,
		const unsigned int* InSrcIndexes, const unsigned int InNumSrcIndexes,
		const float CoAlignmentLimit, const float VolumeImportanceValue, const bool VolumeConservation, const bool bEnforceBoundaries);

	void InitCost();

	double ComputeEdgeCollapseCost(SimpEdge* edge);
	double ComputeEdgeCollapseVertsAndCost(SimpEdge* edge, EdgeVertTupleArray& EdgeVerts);

	float SimplifyMesh();
};