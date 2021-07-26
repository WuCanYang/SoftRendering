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

	void GetVertsInGroup(const SimpVert& seedVert, std::vector<SimpVert*>& InOutVertGroup) const;	//获取同一个组里面的顶点（具有相同位置的顶点）

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

	void GroupVerts(std::vector<SimpVert>& Verts);		//对具有相同位置的顶点放在 一个组里面，通过双向循环链表进行连接
	void SetAttributeIDS(std::vector<SimpVert>& Verts);		//对顶点的属性分配ID，如果同一个组里面的两个顶点某个属性相同，那么它们的ID就是相同的
	void MakeEdges(const std::vector<SimpVert>& Verts, const int NumTris, std::vector<SimpEdge>& Edges);		
	void AppendConnectedEdges(const SimpVert* Vert, std::vector<SimpEdge>& Edges);
	void GroupEdges(std::vector<SimpEdge>& Edges);		//把具有两个顶点相同位置的边组成一个组
};