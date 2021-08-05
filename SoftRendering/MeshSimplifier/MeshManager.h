#pragma once
#include "SimplifierVertex.h"
#include "MeshSimplifyElements.h"
#include "Hash.h"
#include <vector>
#include <unordered_map>
#include <functional>

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

	template<typename ArrayElement>
	void AddUnique(std::vector<ArrayElement>& array, const ArrayElement& val)
	{
		bool exist = false;
		for (ArrayElement& ele : array)
		{
			if (ele == val)
			{
				exist = true;
				break;
			}
		}
		if (!exist) array.push_back(val);
	}

	void GetVertsInGroup(const SimpVert& seedVert, std::vector<SimpVert*>& InOutVertGroup) const;	//获取同一个组里面的顶点（具有相同位置的顶点）

	unsigned int GetVertIndex(const SimpVert* vert) const
	{
		return (unsigned int)(vert - &VertArray[0]);
	}

	unsigned int GetTriIndex(const SimpTri* tri) const
	{
		return (unsigned int)(tri - &TriArray[0]);
	}

	unsigned int TotalNumEdges() const
	{
		return EdgeArray.size();
	}

	SimpEdge* GetEdgePtr(unsigned int index)
	{
		check(index < EdgeArray.size());
		return &EdgeArray[index];
	}

	unsigned int GetDegree(const SimpVert* vert) const		//获取顶点邻接三角形的数量
	{
		unsigned int Degree = 0;

		const SimpVert* StartVertPtr = vert;

		const SimpVert* VertPtr = StartVertPtr;
		do {
			Degree += VertPtr->adjTris.size();
			VertPtr = VertPtr->next;
		} while (VertPtr != StartVertPtr);


		return Degree;
	}

	void GetEdgesInGroup(const SimpEdge* seedEdge, std::vector<SimpEdge*>& InOutEdgeGroup) const
	{
		SimpEdge* EdgePtr = const_cast<SimpEdge*>(seedEdge);
		do {
			InOutEdgeGroup.push_back(EdgePtr);
			EdgePtr = EdgePtr->next;
		} while (EdgePtr != seedEdge);
	}

	bool HasLockedVerts(const SimpEdge* edge) const
	{
		return edge->v0->TestFlags(SIMP_LOCKED) || edge->v1->TestFlags(SIMP_LOCKED);
	}

	bool IsLockedGroup(const std::vector<SimpEdge*>& EdgeGroup) const
	{
		bool locked = false;
		int NumEdgesInGroup = EdgeGroup.size();
		for (int i = 0; i < NumEdgesInGroup; ++i)
		{
			const SimpEdge* edge = EdgeGroup[i];

			if (edge->v0->TestFlags(SIMP_LOCKED) && edge->v1->TestFlags(SIMP_LOCKED))
			{
				locked = true;
				break;
			}
		}
		return locked;
	}

	SimpEdge* FindEdge(const SimpVert* u, const SimpVert* v)
	{
		unsigned int idx = GetEdgeHashPair(u, v).first;

		return (idx < UINT32_MAX) ? &EdgeArray[idx] : nullptr;

	}

	bool IsRemoved(const SimpEdge* edge) const
	{
		return edge->TestFlags(SIMP_REMOVED);
	}

	bool IsInvalid(const SimpEdge* edge) const
	{
		return edge->v0->adjTris.size() == 0 || edge->v1->adjTris.size() == 0;
	}

	unsigned int GetEdgeIndex(const SimpEdge* edge) const
	{
		return (unsigned int)(edge - &EdgeArray[0]);
	}

	unsigned int RemoveTri(SimpTri& Tri)
	{
		Tri.EnableFlags(SIMP_REMOVED);

		for (int j = 0; j < 3; ++j)
		{
			SimpVert* V = Tri.verts[j];
			V->adjTris.remove(&Tri);
		}

		return GetTriIndex(&Tri);
	}

	unsigned int RemoveEdge(SimpEdge& edge);

	unsigned int RemoveEdge(const SimpVert* VertAPtr, const SimpVert* VertBPtr);

	void GetAdjacentTopology(const SimpVert* vert, std::vector<SimpTri*>& DirtyTris, std::vector<SimpVert*>& DirtyVerts, std::vector<SimpEdge*>& DirtyEdges);

	void GetAdjacentTopology(const SimpEdge* edge, std::vector<SimpTri*>& DirtyTris, std::vector<SimpVert*>& DirtyVerts, std::vector<SimpEdge*>& DirtyEdges);

	int RemoveEdgeIfInvalid(std::vector<SimpEdge*>& CandidateEdges, std::vector<unsigned int>& RemovedEdgeIdxArray);

	void UpdateVertexAttriuteIDs(std::vector<SimpEdge*>& InCoincidentEdges);

	bool CollapseEdge(SimpEdge* EdgePtr, std::vector<unsigned int>& RemovedEdgeIdxArray);		

	unsigned int ReplaceVertInEdge(const SimpVert* VertAPtr, const SimpVert* VertBPtr, SimpVert* VertAprimePtr);	//边VertAPtr--VertBPtr	变为  VertAprimePtr--VertBPtr

	unsigned int ReplaceTriVertex(SimpTri& Tri, SimpVert& OldVert, SimpVert& NewVert)
	{
		Tri.ReplaceVertex(&OldVert, &NewVert);
		NewVert.adjTris.push_back(&Tri);
		OldVert.adjTris.remove(&Tri);

		return GetTriIndex(&Tri);
	}

public:

	void MergeGroups(SimpVert* A, SimpVert* B)
	{
		A->next->prev = B->prev;
		B->prev->next = A->next;

		A->next = B;
		B->prev = A;
	}

	template <SimpElementFlags FlagToPropagate >
	void PropagateFlag(SimpVert& MemberOfGroup)
	{
		// spread locked flag to vert group
		unsigned int flags = 0;

		SimpVert* v = &MemberOfGroup;
		do {
			flags |= v->flags & FlagToPropagate;
			v = v->next;
		} while (v != &MemberOfGroup);

		v = &MemberOfGroup;
		do {
			v->flags |= flags;
			v = v->next;
		} while (v != &MemberOfGroup);
	}

	// Remove any verts tagged as with FlagValue (e.g. SIMP_REMOVED)  NB: this may unlink the seed vert!
	template <SimpElementFlags FlagValue >
	void PruneVerts(SimpVert& seedVert)
	{
		// Get all the verts that are link-listed together
		std::vector<SimpVert*> VertsInVertGroup;
		GetVertsInGroup(seedVert, VertsInVertGroup);

		// Unlink any verts that are marked as SIMP_REMOVED
		for (int i = 0, Imax = VertsInVertGroup.size(); i < Imax; ++i)
		{
			SimpVert* v = VertsInVertGroup[i];
			if (v->TestFlags(FlagValue))
			{
				// ungroup
				v->prev->next = v->next;
				v->next->prev = v->prev;
				v->next = v;
				v->prev = v;

				// cleanup.  Insure that pruned verts aren't marked as locekd.
				v->DisableFlags(SIMP_LOCKED);
			}
		}

	}

	int RemoveIfDegenerate(std::vector<SimpTri*>& CandidateTrisPtrArray);

	int RemoveIfDegenerate(std::vector<SimpVert*>& CandidateVertPtrArray);

	int RemoveIfDegenerate(std::vector<SimpEdge*>& CandidateEdges, std::vector<unsigned int>& RemoveEdgeIdxArray);

	void RebuildEdgeLinkLists(std::vector<SimpEdge*>& CandidateEdgePtrArray);		//重新对容器中的边进行分组

	int RemoveDegenerateTris();

	int RemoveDegenerateVerts();

public:

	void FlagBoundary(const SimpElementFlags Flag);

	void FlagEdge(std::function<bool(const SimpVert*, const SimpVert*)> IsDifferent, const SimpElementFlags Flag);

	void GetCoincidentVertGroups(std::vector<SimpVert*>& CoincidentVertGroups);

	struct VertAndID
	{
		int ID;
		SimpVert* SrcVert;

		VertAndID() {};
		VertAndID(SimpVert* SV, int InID)
		{
			ID = InID;
			SrcVert = SV;
		}
	};

	enum class VtxElementWeld
	{
		Normal,
		Tangent,
		BiTangent,
		Color,
		UV,
	};

	void WeldNonSplitBasicAttributes(VtxElementWeld WeldType);

	void OutputMesh(MeshVertType* Verts, unsigned int* Indexes, std::vector<int>* LockedVerts = nullptr);

private:

	void GroupVerts(std::vector<SimpVert>& Verts);		//对具有相同位置的顶点放在 一个组里面，通过双向循环链表进行连接
	void SetAttributeIDS(std::vector<SimpVert>& Verts);		//对顶点的属性分配ID，如果同一个组里面的两个顶点某个属性相同，那么它们的ID就是相同的
	void MakeEdges(const std::vector<SimpVert>& Verts, const int NumTris, std::vector<SimpEdge>& Edges);		
	void AppendConnectedEdges(const SimpVert* Vert, std::vector<SimpEdge>& Edges);
	void GroupEdges(std::vector<SimpEdge>& Edges);		//把具有两个顶点相同位置的边组成一个组


	unsigned int HashEdge(const SimpVert* u, const SimpVert* v) const
	{
		unsigned int ui = GetVertIndex(u);
		unsigned int vi = GetVertIndex(v);

		return Murmur32({ std::min(ui, vi), std::max(ui, vi) });
	}

	unsigned int HashEdgePosition(const SimpEdge& Edge)
	{
		return HashPoint(Edge.v0->GetPos()) ^ HashPoint(Edge.v1->GetPos());
	}

	std::pair<unsigned int, unsigned int> GetEdgeHashPair(const SimpVert* u, const SimpVert* v) const
	{
		unsigned int hashValue = HashEdge(u, v);

		std::pair<unsigned int, unsigned int> Result = std::make_pair(UINT32_MAX, hashValue);
		auto range = EdgeVertIdHashMap.equal_range(hashValue);
		auto& start = range.first;
		auto& end = range.second;
		for (; start != end; ++start)
		{
			unsigned int idx = start->second;

			if ((EdgeArray[idx].v0 == u && EdgeArray[idx].v1 == v) ||
				(EdgeArray[idx].v0 == v && EdgeArray[idx].v1 == u))
			{
				Result.first = idx;
				break;
			}
		}
		return Result;
	}
};