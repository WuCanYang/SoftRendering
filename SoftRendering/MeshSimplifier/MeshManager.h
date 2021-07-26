#pragma once
#include "SimplifierVertex.h"
#include "MeshSimplifyElements.h"
#include "Hash.h"
#include <vector>
#include <unordered_map>

class MeshManager
{
public:

	const int NumSrcVerts = 0;
	const int NumSrcTris = 0;

	int   ReducedNumVerts;
	int   ReducedNumTris;

	std::vector<SimpVert> VertArray;
	std::vector<SimpTri>  TriArray;
	std::vector<SimpEdge> EdgeArray;

	std::unordered_multimap<unsigned int, unsigned int> EdgeVertIdHashMap;

public:

	MeshManager(const MeshVertType* InSrcVerts, const unsigned int InNumSrcVerts,
		const unsigned int* InSrcIndexes, const unsigned int InNumSrcIndexes);

	void GetVertsInGroup(const SimpVert& seedVert, std::vector<SimpVert*>& InOutVertGroup) const;	//��ȡͬһ��������Ķ��㣨������ͬλ�õĶ��㣩

	unsigned int GetVertIndex(const SimpVert* vert) const
	{
		return (unsigned int)(vert - &VertArray[0]);
	}

	unsigned int HashEdge(const SimpVert* u, const SimpVert* v) const
	{
		unsigned int ui = GetVertIndex(u);
		unsigned int vi = GetVertIndex(v);
		
		return Murmur32({ std::min(ui, vi), std::max(ui, vi) });
	}

	unsigned int TotalNumEdges() const
	{
		return EdgeArray.size();
	}

private:

	void GroupVerts(std::vector<SimpVert>& Verts);		//�Ծ�����ͬλ�õĶ������ һ�������棬ͨ��˫��ѭ�������������
	void SetAttributeIDS(std::vector<SimpVert>& Verts);		//�Զ�������Է���ID�����ͬһ�����������������ĳ��������ͬ����ô���ǵ�ID������ͬ��
	void MakeEdges(const std::vector<SimpVert>& Verts, const int NumTris, std::vector<SimpEdge>& Edges);		
	void AppendConnectedEdges(const SimpVert* Vert, std::vector<SimpEdge>& Edges);
	void GroupEdges(std::vector<SimpEdge>& Edges);		//�Ѿ�������������ͬλ�õı����һ����
};