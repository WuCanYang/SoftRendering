#pragma once
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "SimplifierMacros.h"
#include "SimplifierDataStructure.h"

template<int NumTexCoords>
class BasicVertexAttrs
{
public:
	typedef DenseBMatrix<13 + NumTexCoords>			DenseBMatrixType;		//属性写成的矩阵
	typedef DenseVecD<13 + NumTexCoords>			DenseVecDType;			//属性写成的向量


	Vector3			Normal;      // 0, 1, 2
	Vector3			Tangent;     // 3, 4, 5
	Vector3         BiTangent;   // 6, 7, 8
	Vector4			Color;       // 9, 10, 11, 12
	Vector2			TexCoords[NumTexCoords];  // 13, .. 13 + NumTexCoords * 2 - 1


	// used to manage identity of split/non-split vertex attributes.
	struct FElementIDs
	{
		int NormalID;
		int TangentID;
		int BiTangentID;
		int ColorID;
		int TexCoordsID[NumTexCoords];

		enum { InvalidID = -1 };

		// construct with invalid ID values.
		FElementIDs()
		{
			NormalID = InvalidID;
			TangentID = InvalidID;
			BiTangentID = InvalidID;
			ColorID = InvalidID;
			for (int i = 0; i < NumTexCoords; ++i)
			{
				TexCoordsID[i] = InvalidID;
			}
		}

		FElementIDs(const  FElementIDs& other)
		{
			NormalID = other.NormalID;
			TangentID = other.TangentID;
			BiTangentID = other.BiTangentID;
			ColorID = other.ColorID;
			for (int i = 0; i < NumTexCoords; ++i)
			{
				TexCoordsID[i] = other.TexCoordsID[i];
			}
		}

		// Subtract other ID struct from this one.
		FElementIDs operator-(const  FElementIDs& other) const
		{
			FElementIDs Result;
			Result.NormalID = NormalID - other.NormalID;
			Result.TangentID = TangentID - other.TangentID;
			Result.BiTangentID = BiTangentID - other.BiTangentID;
			Result.ColorID = ColorID - other.ColorID;
			for (int i = 0; i < NumTexCoords; ++i)
			{
				Result.TexCoordsID[i] = TexCoordsID[i] - other.TexCoordsID[i];
			}
			return Result;
		}

		// copy other ID if the mask value is zero
		void MaskedCopy(const  FElementIDs& IDMask, const  FElementIDs& other)
		{
			NormalID = (IDMask.NormalID == 0) ? other.NormalID : NormalID;
			TangentID = (IDMask.TangentID == 0) ? other.TangentID : TangentID;
			BiTangentID = (IDMask.BiTangentID == 0) ? other.BiTangentID : BiTangentID;
			ColorID = (IDMask.ColorID == 0) ? other.ColorID : ColorID;
			for (int i = 0; i < NumTexCoords; ++i)
			{
				TexCoordsID[i] = (IDMask.TexCoordsID[i] == 0) ? other.TexCoordsID[i] : TexCoordsID[i];
			}
		}

		// copy IDs values over to this for elemenets where IDMask == 0 and InverseIDMask != 0
		void MaskedCopy(const  FElementIDs& IDMask, const  FElementIDs& InverseIDMask, const  FElementIDs& BIDs)
		{
			if (InverseIDMask.NormalID != 0 && IDMask.NormalID == 0)
			{
				NormalID = BIDs.NormalID;
			}
			if (InverseIDMask.TangentID != 0 && IDMask.TangentID == 0)
			{
				TangentID = BIDs.TangentID;
			}
			if (InverseIDMask.BiTangentID != 0 && IDMask.BiTangentID == 0)
			{
				BiTangentID = BIDs.BiTangentID;
			}
			if (InverseIDMask.ColorID != 0 && IDMask.ColorID == 0)
			{
				ColorID = BIDs.ColorID;
			}
			for (int i = 0; i < NumTexCoords; ++i)
			{
				if (InverseIDMask.TexCoordsID[i] != 0 && IDMask.TexCoordsID[i] == 0)
				{
					TexCoordsID[i] = BIDs.TexCoordsID[i];
				}
			}
		}

	};

	FElementIDs ElementIDs;

public:

	typedef	DenseArrayWrapper<float>		DenseAttrAccessor;

	BasicVertexAttrs() :
		Normal(1),
		Tangent(1),
		BiTangent(1),
		Color(1)
	{
		for (int i = 0; i < NumTexCoords; ++i)
		{
			TexCoords[i] = Vector2(1);
		}
	}

	BasicVertexAttrs(const BasicVertexAttrs& other) :
		Normal(other.Normal),
		Tangent(other.Normal),
		BiTangent(other.BiTangent),
		Color(other.Color),
		ElementIDs(other.ElementIDs)
	{
		for (int i = 0; i < NumTexCoords; ++i)
		{
			TexCoords[i] = other.TexCoords[i];
		}
	}

	static int Size() { return 13 + NumTexCoords; }

	DenseAttrAccessor  AsDenseAttrAccessor() { return DenseAttrAccessor((float*)&Normal, Size()); }
	const DenseAttrAccessor  AsDenseAttrAccessor() const { return DenseAttrAccessor((float*)&Normal, Size()); }

	void Correct()
	{
		Normal.Normalize();
		Tangent -= Tangent.dot(Normal) * Normal;
		Tangent.Normalize();
		BiTangent -= BiTangent.dot(Normal) * Normal;
		BiTangent -= BiTangent.dot(Tangent) * Tangent;
		BiTangent.Normalize();

		auto Clamp = [](float val, float mmin, float mmax) -> float
		{
			return val < mmin ? mmin : val > mmax ? mmax : val;
		};
		Color._x = Clamp(Color._x, 0.0f, 1.0f);
		Color._y = Clamp(Color._y, 0.0f, 1.0f);
		Color._z = Clamp(Color._z, 0.0f, 1.0f);
		Color._w = Clamp(Color._w, 0.0f, 1.0f);
	}
};

class BoneSparseVertexAttrs
{
public:

	void Correct()
	{

	}
};


class MeshVertType
{
public:
	//原版各种template和typedef混合，这里简化一下，与原来实际用法的表达一致

	typedef BasicVertexAttrs<MAX_TEXCOORDS>			BasicAttrContainerType;
	typedef BoneSparseVertexAttrs					AdditionalAttrContainerType;
	typedef BoneSparseVertexAttrs					BoneContainer;

	//离边最近的那个顶点的索引
	int MasterVertIndex;

	unsigned int	MaterialIndex;
	Vector3			Position;

	//Additional weight
	float           SpecializedWeight;


	BasicAttrContainerType				BasicAttributes;
	
	//附加属性原程序中好像就没有使用，只用了基本属性和骨骼，因此后面实现中都没有包括附加属性
	AdditionalAttrContainerType			AdditionalAttributes;

	BoneContainer						SparseBone;



public:
	typedef typename  BasicAttrContainerType::DenseAttrAccessor    DenseAttrAccessor;

	unsigned int GetMaterialIndex() const { return MasterVertIndex; }
	Vector3& GetPos() { return Position; }
	const Vector3& GetPos() const { return Position; }


	MeshVertType():
		MasterVertIndex(-1),
		MaterialIndex(0),
		Position(0),
		SpecializedWeight(0.f),
		BasicAttributes(),
		AdditionalAttributes(),
		SparseBone()
	{}

	MeshVertType(const MeshVertType& other):
		MasterVertIndex(other.MasterVertIndex),
		MaterialIndex(other.MaterialIndex),
		Position(other.Position),
		SpecializedWeight(other.SpecializedWeight),
		BasicAttributes(other.BasicAttributes),
		AdditionalAttributes(other.AdditionalAttributes),
		SparseBone(other.SparseBone)
	{}


	AdditionalAttrContainerType& GetAdditionalAttrContainer() { return AdditionalAttributes; }
	const AdditionalAttrContainerType& GetAdditionalAttrContainer() const { return AdditionalAttributes; }

	BoneContainer& GetBoneContainer() { return SparseBone; }
	const BoneContainer& GetBoneContainer() const { return SparseBone; }

	static int NumBaseAttributes() { return BasicAttrContainerType::Size(); }
	DenseAttrAccessor GetBasicAttrAccessor() { return BasicAttributes.AsDenseAttrAccessor(); }
	const DenseAttrAccessor GetBasicAttrAccessor() const { return BasicAttributes.AsDenseAttrAccessor(); }

	void Correct()
	{
		BasicAttributes.Correct();
		AdditionalAttributes.Correct();
		SparseBone.Correct();
	}

	MeshVertType& operator=(const MeshVertType& other)
	{
		MasterVertIndex			= other.MasterVertIndex;
		MaterialIndex			= other.MaterialIndex;
		Position				= other.Position;
		SpecializedWeight		= other.SpecializedWeight;
		BasicAttributes			= other.BasicAttributes;
		AdditionalAttributes	= other.AdditionalAttributes;
		SparseBone				= other.SparseBone;

		return *this;
	}
};