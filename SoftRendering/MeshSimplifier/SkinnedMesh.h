#pragma once
#include "SimplifierVertex.h"
#include "SimplifierMacros.h"
#include <iostream>

class SkinnedMesh		//将原模型Mesh 转化为可以用于Simplifier的Mesh
{
	int TrisNum;
	int VertsNum;
	int TexCoordsNum;

public:

	MeshVertType* VertexBuffer;
	unsigned int* IndexBuffer;

	SkinnedMesh() :
		VertexBuffer(nullptr),
		IndexBuffer(nullptr),
		TrisNum(0),
		VertsNum(0),
		TexCoordsNum(0)
	{}

	SkinnedMesh(int NumTriangles, int NumVertices)
	{
		Resize(NumTriangles, NumVertices);
	}

	~SkinnedMesh()
	{
		Empty();
	}

	void Resize(int NumTriangles, int NumVertices)
	{
		if (IndexBuffer == nullptr)  delete[] IndexBuffer;
		if (VertexBuffer == nullptr) delete[] VertexBuffer;

		TrisNum = NumTriangles;
		VertsNum = NumVertices;

		IndexBuffer = new unsigned int[TrisNum * 3];
		VertexBuffer = new MeshVertType[VertsNum];
	}

	void Empty()
	{
		if (IndexBuffer == nullptr)  delete[] IndexBuffer;
		if (VertexBuffer == nullptr) delete[] VertexBuffer;

		TrisNum = 0;
		VertsNum = 0;
	}

	int NumIndices() const { return TrisNum * 3; }

	int NumVertices() const { return VertsNum; }

	int NumTris() const { return TrisNum; }

	int TexCoordCount() const { return TexCoordsNum; }

	void SetTexCoordCount(int c)
	{
		TexCoordsNum = c;
	}


	//删除VertexBuffer中没有在IndexBuffer中用到的顶点
	void Compact()
	{
		if (IndexBuffer == nullptr)
		{
			return;
		}

		int* Mask = new int[VertsNum];
		for (int i = 0; i < VertsNum; ++i)
		{
			Mask[i] = 0;
		}

		for (int i = 0; i < TrisNum * 3; ++i)
		{
			unsigned int VertId = IndexBuffer[i];
			Mask[VertId] = 1;
		}

		int RequiredVertCount = 0;
		for (int i = 0; i < VertsNum; ++i)
		{
			RequiredVertCount += Mask[i];
		}

		if (RequiredVertCount == VertsNum)
		{
			delete[] Mask;
			return;
		}

		// stash the pointers to the current buffers
		unsigned int* OldIndexBuffer = IndexBuffer;
		MeshVertType* OldVertexBuffer = VertexBuffer;

		if (OldVertexBuffer != nullptr && OldIndexBuffer != nullptr)
		{
			int OldNumTris = TrisNum;
			int OldNumVerts = VertsNum;

			// null the pointers to keep the resize from deleting the arrays.
			IndexBuffer = nullptr;
			VertexBuffer = nullptr;

			// Allocate memory for the compacted mesh.
			Resize(TrisNum, RequiredVertCount);

			// Copy the verts into the new vertex array

			for (int i = 0, j = 0; i < OldNumVerts; ++i)
			{
				if (Mask[i] == 0) continue;

				check(j < RequiredVertCount);

				VertexBuffer[j] = OldVertexBuffer[i];
				j++;
			}

			// record offsets the Mask
			// so that Mask[i] will be the number of voids prior to and including i.
			{
				int VoidCount = 0;
				for (int i = 0; i < OldNumVerts; ++i)
				{
					VoidCount += (1 - Mask[i]);
					Mask[i] = VoidCount;
				}
			}
			check(OldNumTris == TrisNum);

			// translate the offsets in the index buffer 
			for (int i = 0; i < OldNumTris * 3; ++i)
			{
				int OldVertIdx = OldIndexBuffer[i];
				int VoidCount = Mask[OldVertIdx];
				int NewVertIdx = OldVertIdx - VoidCount;

				check(NewVertIdx > -1);

				IndexBuffer[i] = NewVertIdx;
			}


			// Clean up the temporary 
			delete[] OldVertexBuffer;
			delete[] OldIndexBuffer;
		}
		delete[] Mask;
	}
};