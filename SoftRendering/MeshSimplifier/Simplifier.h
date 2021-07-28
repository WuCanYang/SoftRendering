#pragma once
#include "MeshManager.h"
#include "BinaryHeap.h"
#include "SimplifierQuadrics.h"

class MeshSimplifier
{
	typedef typename std::tuple<SimpVert*, SimpVert*, MeshVertType>		EdgeVertTuple;
	typedef	typename std::vector<EdgeVertTuple>							EdgeVertTupleArray;

	typedef BasicVertexAttrs<MAX_TEXCOORDS>::DenseVecDType				DenseVecDType;

public:

	MeshManager			meshManager;

	DenseVecDType	BasicAttrWeights;


	FBinaryHeap<double>	CollapseCostHeap;


	double               VolumeImportance;			//�������Ȩ��
	bool                 bPreserveVolume;			//�Ƿ����������
	bool                 bCheckBoneBoundaries;		//�Ƿ��������ͷ�


	unsigned int		 DegreeLimit = 24;				//��̮�����ڽӵ������������������ʱ�� ����һ������ĳͷ�
	double               DegreePenalty = 100.0f;		//�ͷ� =  ���ڽӵ����������� - DegreeLimit��* DegreePenalty

	double               invalidPenalty = 1.e6;			//��������������Ӱ��Ĺ�����һ��ʱ�Լ� �����滻������ʱӦ�ø��ĳͷ�

	double               BoundaryConstraintWeight = 256.;		//�߽�Լ��Ȩ��



public:

	MeshSimplifier(const MeshVertType* InSrcVerts, const unsigned int InNumSrcVerts,
		const unsigned int* InSrcIndexes, const unsigned int InNumSrcIndexes,
		const float VolumeImportanceValue, const bool VolumeConservation, const bool bEnforceBoundaries);

	void InitCost();

	//�����ĸ��������ð������ϵ���
	double  ComputeEdgeCollapseCost(SimpEdge* edge);
	double  ComputeEdgeCollapseVertsAndCost(SimpEdge* edge, EdgeVertTupleArray& newVerts);
	void	ComputeEdgeCollapseVertsAndQuadrics(SimpEdge* edge, EdgeVertTupleArray& newVerts, EdgeQuadric& newEdgeQuadric, std::vector<WedgeQuadric>& newQuadrics);
	Vector3 ComputeEdgeCollapseVertsPos(SimpEdge* edge, EdgeVertTupleArray& newVerts, std::vector<WedgeQuadric>& quadrics, EdgeQuadric& edgeQuadric);


	float SimplifyMesh();
};