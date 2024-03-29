#pragma once
#include "MeshManager.h"
#include "BinaryHeap.h"
#include "SimplifierQuadrics.h"
#include "SimplifierQuadricsCache.h"

class SimplifierTerminator
{
public:

	int   MinTriNumToRetain;
	int   MinVertNumToRetain;
	float MaxFeatureCost;
	float MaxDistance;

	SimplifierTerminator(int minTri, int minVert, float maxCost, float maxDist):
		MinTriNumToRetain(minTri),
		MinVertNumToRetain(minVert),
		MaxFeatureCost(maxCost),
		MaxDistance(maxDist)
	{}

	bool operator()(const int TriNum, const int VertNum, const float Error)
	{
		if (TriNum < MinTriNumToRetain || VertNum < MinVertNumToRetain || Error > MaxFeatureCost)
		{
			return true;
		}
		return false;
	}
};

class MeshSimplifier
{
	typedef typename std::tuple<SimpVert*, SimpVert*, MeshVertType>		EdgeVertTuple;
	typedef	typename std::vector<EdgeVertTuple>							EdgeVertTupleArray;

public:
	typedef BasicVertexAttrs<MAX_TEXCOORDS>::DenseVecDType				DenseVecDType;

public:

	MeshManager			meshManager;

	DenseVecDType		BasicAttrWeights;

	FBinaryHeap<double>	CollapseCostHeap;

	QuadricCache		quadricCache;


	double               VolumeImportance;			//体积保护权重
	bool                 bPreserveVolume;			//是否开启体积保护
	bool                 bCheckBoneBoundaries;		//是否开启骨骼惩罚


	unsigned int		 DegreeLimit = 24;				//当坍缩边邻接的三角形数量超过这个时， 给它一个下面的惩罚
	double               DegreePenalty = 100.0f;		//惩罚 =  （邻接的三角形数量 - DegreeLimit）* DegreePenalty

	double               invalidPenalty = 1.e6;			//当边两个顶点受影响的骨骼不一样时以及 顶点替换不合理时应该给的惩罚

	double               BoundaryConstraintWeight = 256.;		//边界约束权重

protected:

	void InitCost();

	//下面四个函数调用包含从上到下， 流程：计算边和面的Quadric  --->  求得一个坍缩后的位置   --->   根据位置计算出顶点属性   --->    根据位置和属性评估一个代价
	double  ComputeEdgeCollapseCost(SimpEdge* edge);
	double  ComputeEdgeCollapseVertsAndCost(SimpEdge* edge, EdgeVertTupleArray& newVerts);
	void	ComputeEdgeCollapseVertsAndQuadrics(SimpEdge* edge, EdgeVertTupleArray& newVerts, EdgeQuadric& newEdgeQuadric, std::vector<WedgeQuadric>& newQuadrics);
	Vector3 ComputeEdgeCollapseVertsPos(SimpEdge* edge, EdgeVertTupleArray& newVerts, std::vector<WedgeQuadric>& quadrics, EdgeQuadric& edgeQuadric);


	void ComputeEdgeCollapseVertsAndFixBones(SimpEdge* edge, EdgeVertTupleArray& newVerts);
	void ComputeEdgeCollapseVerts(SimpEdge* edge, EdgeVertTupleArray& newVerts);

	void UpdateEdgeCollapseCost(std::vector<SimpEdge*>& DirtyEdges);

	WedgeQuadric GetWedgeQuadric(SimpVert* v);

	EdgeQuadric  GetEdgeQuadric(SimpVert* v);


	void DirtyTriQuadricCache(std::vector<SimpTri*>& DirtyTri);

	void DirtyVertAndEdgeQuadricCache(std::vector<SimpVert*>& DirtyVert);


public:

	MeshSimplifier(const MeshVertType* InSrcVerts, const unsigned int InNumSrcVerts,
		const unsigned int* InSrcIndexes, const unsigned int InNumSrcIndexes,
		const float VolumeImportanceValue, const bool VolumeConservation, const bool bEnforceBoundaries);


	int GetNumVerts() const { return meshManager.ReducedNumVerts; }

	int GetNumTris() const { return meshManager.ReducedNumTris; }

	void SetBoundaryConstraintWeight(const double Weight)
	{
		BoundaryConstraintWeight = Weight;
	}

	void SetAttributeWeights(const DenseVecDType& Weights)
	{
		BasicAttrWeights = Weights;
	}

	void SetBoundaryLocked()		//对于一条边，如果它的邻接三角形只有一个（边界），就加锁
	{
		meshManager.FlagBoundary(SimpElementFlags::SIMP_LOCKED);
	}

	void SetColorEdgeLocked(float ColorDistThreshold = 1.e-3)		//对于一条边，如果两个顶点颜色不一样，就加锁
	{
		auto IsDifferent = [ColorDistThreshold](const SimpVert* A, const SimpVert* B) -> bool
		{
			if (A && B)
			{
				const Vector4& ColorA = A->vert.BasicAttributes.Color;
				const Vector4& ColorB = A->vert.BasicAttributes.Color;

				return (ColorA - ColorB).length() > ColorDistThreshold;
			}
			return true;
		};
		meshManager.FlagEdge(IsDifferent, SIMP_LOCKED);
	}

	float SimplifyMesh(SimplifierTerminator Terminator);

	void OutputMesh(MeshVertType* Verts, unsigned int* Indexes, bool MergeCoincidentVertBones = true, bool WeldVtxColorAttrs = true, std::vector<int>* LockedVerts = nullptr);
};