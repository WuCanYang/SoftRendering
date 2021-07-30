#pragma once
#include "SimplifierDataStructure.h"
#include "SimplifierVertex.h"

//边的二次误差度量，给边界边添加一个约束，让其代价变大，防止变形
class EdgeQuadric
{
	SymmetricMatrix		CMatrix;
	Vector3d			D0Vector;
	double				Scalar;

	friend class QuadricOptimizer;

public:

	EdgeQuadric() {}

	EdgeQuadric(const Vector3d& v0Pos, const Vector3d& v1Pos, const Vector3d& FaceNormal, const double EdgeWeight);

	EdgeQuadric(const EdgeQuadric& other) :
		CMatrix(other.CMatrix),
		D0Vector(other.D0Vector),
		Scalar(other.Scalar)
	{}

	EdgeQuadric& operator+=(const EdgeQuadric& other)
	{
		CMatrix += other.CMatrix;
		D0Vector += other.D0Vector;
		Scalar += other.Scalar;
		return *this;
	}

	// (n^T * v + d) ^ 2   =   v^T  (n * n^T ) v + 2d n^T v +  d * d
	// v = Pos
	// n * n^T  = CMatrix
	// d * d = Scalar
	double Evaluate(const Vector3d& Pos) const
	{
		Vector3d CPos = CMatrix * Pos;

		double Result = CPos.dot(Pos) + 2.0 * Pos.dot(D0Vector) + Scalar;
		return Result;
	}

	void Zero()
	{
		CMatrix.Zero();
		D0Vector.Zero();

		Scalar = 0.0;
	}


};

inline EdgeQuadric::EdgeQuadric(const Vector3d& v0Pos, const Vector3d& v1Pos, const Vector3d& FaceNormal, const double EdgeWeight)
{
	if (abs(FaceNormal.length() - 1.0) > 0.01) return;

	Vector3d Edge = v1Pos - v0Pos;
	double EdgeLength = Edge.length();
	const double Weight = EdgeLength * EdgeWeight;

	if (EdgeWeight < 1.e-8) return;
	else
	{
		Edge *= 1.0 / EdgeLength;
	}

	Vector3d N = Edge.dot(FaceNormal);
	double length = N.length();
	if (length < 1.e-8) return;
	else
	{
		N *= 1.0 / length;
	}

	double Dist = -N.dot(v0Pos);

	CMatrix  = ScaledProjectionOperator(N);
	D0Vector = N * Dist;
	Scalar   = Dist * Dist;

	CMatrix  *= Weight;
	D0Vector *= Weight;
	Scalar   *= Weight;
}


//---------------------------------------------------------------------------------


class QuadricBase
{
public:
	typedef typename BasicVertexAttrs<MAX_TEXCOORDS>::DenseBMatrixType			B1MatrixType;
	typedef Vector3d															D0VectorType;
	typedef typename BasicVertexAttrs<MAX_TEXCOORDS>::DenseVecDType				D1VectorType;

protected:

	SymmetricMatrix		CMatrix;
	B1MatrixType		B1Matrix;

	D0VectorType		D0Vector;
	D1VectorType		D1Vector;

	double Gamma = 0.;

	double CScalar = 0.;

	double			   VolDistConstraint = 0.;
	Vector3d           VolGradConstraint;


	AABB2d UVBox;

public:

	QuadricBase() {}

	QuadricBase(const QuadricBase& Other):
		CMatrix(Other.CMatrix),
		B1Matrix(Other.B1Matrix),
		D0Vector(Other.D0Vector),
		D1Vector(Other.D1Vector),
		Gamma(Other.Gamma),
		CScalar(Other.CScalar),
		VolDistConstraint(Other.VolDistConstraint),
		VolGradConstraint(Other.VolGradConstraint),
		UVBox(Other.UVBox)
	{}

	QuadricBase& operator=(const QuadricBase& Other)
	{
		CMatrix = Other.CMatrix;
		B1Matrix = Other.B1Matrix;
		D0Vector = Other.D0Vector;
		D1Vector = Other.D1Vector;
		Gamma = Other.Gamma;
		CScalar = Other.CScalar;
		VolDistConstraint = Other.VolDistConstraint;
		VolGradConstraint = Other.VolGradConstraint;
		UVBox = Other.UVBox;
		return *this;
	}

	QuadricBase& operator+=(const QuadricBase& Other)
	{
		CMatrix += Other.CMatrix;
		B1Matrix += Other.B1Matrix;
		D0Vector += Other.D0Vector;
		D1Vector += Other.D1Vector;
		Gamma += Other.Gamma;
		CScalar += Other.CScalar;
		VolDistConstraint += Other.VolDistConstraint;
		VolGradConstraint += Other.VolGradConstraint;
		UVBox += Other.UVBox;
		return *this;
	}

	double EvaluateQaudric(const D0VectorType& Pos, const D1VectorType& S1) const;
};

inline double QuadricBase::EvaluateQaudric(const D0VectorType& Pos, const D1VectorType& S1) const
{
	// S = {pos, s1, s2}

// < s | Quadric_Matrix * s> = 
//                            < pos | C * pos> + 2 ( < pos | B1 * s1 > + < pos | B2 * s2 >  ) + Gamma* ( < s1 | s1 > + < s2 | s2 > )
	double  SQmS = 0.;
	{
		double Pt1 = Pos.dot(CMatrix * Pos);
		double Pt2 = 2. * Pos.dot(B1Matrix * S1);
		double Pt3 = Gamma * (S1.L2NormSqr());

		SQmS = Pt1 + Pt2 + Pt3;
	}

	// quadric_vector = {D0Vector, D1Vector, D2Vector}
	//
	// 2. * < s | quadratic_vector > = 
	//                                2.* (  < pos | D0Vector > + < s1 | D1Vector > + < s2 | D2Vector > )
	double CrossTerm = 0.;
	{
		double Pt1 = Pos.dot(D0Vector);
		double Pt2 = S1.dot(D1Vector);

		CrossTerm = 2. * (Pt1 + Pt2);
	}

	// < s | Quadric_Matrix * s > + 2 < s | quadric_vector > + quadric_scalar

	return SQmS + CrossTerm + CScalar;
}


//---------------------------------------------------------------------------------


class WedgeQuadric : public QuadricBase
{
public:

	typedef QuadricBase		MyBase;
	typedef BasicVertexAttrs<MAX_TEXCOORDS>::DenseAttrAccessor		DenseAttrAccessor;
	
	typedef MyBase::B1MatrixType		B1MatrixType;
	typedef MyBase::D0VectorType		D0VectorType;
	typedef MyBase::D1VectorType		D1VectorType;


public:

	WedgeQuadric() :MyBase() {}

	WedgeQuadric(const WedgeQuadric& Other) : MyBase(Other) {}

	WedgeQuadric(const MeshVertType& TriVert0, const MeshVertType& TriVert1, const MeshVertType& TriVert2, const D1VectorType& BasicWeights)
	{
		const Vector3d Vert0Pos(TriVert0.GetPos());
		const Vector3d Vert1Pos(TriVert1.GetPos());
		const Vector3d Vert2Pos(TriVert2.GetPos());

		Vector3d FaceNormal = (Vert2Pos - Vert0Pos).cross(Vert1Pos - Vert0Pos);
		double length = FaceNormal.length();
		double Area;

		if (length < 0.)
		{
			B1Matrix.Reset();
			D1Vector.Reset();

			return;
		}
		else
		{
			FaceNormal *= 1. / length;
			Area = 0.5 * length;
		}

		double Dist = -FaceNormal.dot(Vert0Pos);

		CMatrix = ScaledProjectionOperator(FaceNormal);
		D0Vector = FaceNormal * Dist;
		CScalar = Dist * Dist;

		//体积约束
		VolGradConstraint = FaceNormal * (1. / 3.);
		VolDistConstraint = Dist * (1. / 3.);


		InverseGradientProjection GradientTool(DMatrix(Vert0Pos, Vert1Pos, Vert2Pos), FaceNormal);

		//将基本属性编入 B1Matrix 和 D1Vector
		{

			//防止插值纹理UV 超过包围盒
			UVBox.ExpandToInclude(TriVert0.BasicAttributes.TexCoords[0]);
			UVBox.ExpandToInclude(TriVert1.BasicAttributes.TexCoords[0]);
			UVBox.ExpandToInclude(TriVert2.BasicAttributes.TexCoords[0]);

			DenseAttrAccessor Vert0BasicAttr = TriVert0.GetBasicAttrAccessor();
			DenseAttrAccessor Vert1BasicAttr = TriVert1.GetBasicAttrAccessor();
			DenseAttrAccessor Vert2BasicAttr = TriVert2.GetBasicAttrAccessor();


			//三个顶点的属性编入B1Matrix 和 D1Vector
			bool hasGradient = EncodeAttrGradient(GradientTool, Vert0BasicAttr, Vert1BasicAttr, Vert2BasicAttr, BasicWeights, B1Matrix, D1Vector);
			CScalar += D1Vector.L2NormSqr();

			if (hasGradient)
			{
				//将B1Matrix、CMatrix中的梯度和距离加到  CMatrix 和 D0Vector 中
				SumOuterProducts(B1Matrix, D1Vector, CMatrix, D0Vector);
			}
		}


		Gamma = Area;
		WeightByArea(Area);
	}

	WedgeQuadric& operator+=(const WedgeQuadric& Other)
	{
		MyBase::operator+=(Other);
		return *this;
	}

	double Evaluate(const MeshVertType& Vert, const D1VectorType& BasicWeights) const
	{
		const Vector3d Pos = Vert.GetPos();
		
		const auto BasicAttrAccessor = Vert.GetBasicAttrAccessor();
		D1VectorType S1;
		check(BasicAttrAccessor.Num() == BasicWeights.size());
		check(BasicAttrAccessor.Num() == S1.size());

		for (int i = 0; i < BasicAttrAccessor.Num(); ++i)
		{
			S1.SetElement(i, BasicAttrAccessor[i] * BasicWeights[i]);
		}

		return MyBase::EvaluateQaudric(Pos, S1);
	}

	void CalcAttributes(MeshVertType& Vert, const D1VectorType& BasicWeights) const
	{
		Vector3d Pos = Vert.GetPos();
		auto BasicAttrAccessor = Vert.GetBasicAttrAccessor();		//通过这个来改变顶点中的属性
		ComputeAttrs(Gamma, B1Matrix, D1Vector, Pos, BasicWeights, BasicAttrAccessor);

		const float PaddingFactor = 0.2f;
		UVBox.ClampPoint(Vert.BasicAttributes.TexCoords[0], PaddingFactor);
	}

	double TotalArea() const { return Gamma; }


private:

	void WeightByArea(const double Area)
	{
		CMatrix *= Area;
		B1Matrix *= Area;

		D0Vector *= Area;
		D1Vector *= Area;
		CScalar *= Area;

		VolDistConstraint *= Area;
		VolGradConstraint *= Area;
	}

	bool EncodeAttrGradient(const InverseGradientProjection& GradientTool,
		const DenseAttrAccessor& Vert0Attrs,
		const DenseAttrAccessor& Vert1Attrs,
		const DenseAttrAccessor& Vert2Attrs,
		const D1VectorType& Weights,
		B1MatrixType& GradientMatrix,
		D1VectorType& DistanceVector)
	{
		GradientMatrix.Reset();
		DistanceVector.Reset();

		bool valid = GradientTool.IsValid();
		if (valid)
		{
			Vector3d Attr;
			int AttrNum = Weights.size();
			for (int i = 0; i < AttrNum; ++i)
			{
				const double weight = Weights[i];
				if (weight < 1.e-6) continue;

				Attr._x = Vert0Attrs[i];
				Attr._y = Vert1Attrs[i];
				Attr._z = Vert2Attrs[i];
				Attr *= weight;

				Vector3d Grad;
				double Dist = GradientTool.ComputeGradient(Attr, Grad);

				GradientMatrix.SetColumn(i, -1. * Grad);
				DistanceVector.SetElement(i, -1. * Dist);
			}
		}
		else
		{
			Vector3d Attr;
			int AttrNum = Weights.size();
			for (int i = 0; i < AttrNum; ++i)
			{
				const double weight = Weights[i];
				if (weight < 1.e-6) continue;

				Attr._x = Vert0Attrs[i];
				Attr._y = Vert1Attrs[i];
				Attr._z = Vert2Attrs[i];
				Attr *= weight;

				double AverageAttr = (1. / 3.) * (Attr[0], Attr[1], Attr[2]);

				GradientMatrix.SetColumn(i, Vector3d());
				DistanceVector.SetElement(i, -1. * AverageAttr);
			}
		}
		return valid;
	}


	static void ComputeAttrs(const double Area, const B1MatrixType& GradientMatrix, const D1VectorType& DistVector, const Vector3d& Pos, const D1VectorType& Weights, DenseAttrAccessor& Attrs);


	static void SumOuterProducts(const B1MatrixType& GradientArray, const D1VectorType& DistArray, SymmetricMatrix& OuterProductSum, Vector3d& DistGradientSum);

};

inline void WedgeQuadric::ComputeAttrs(const double Area, const B1MatrixType& GradientMatrix, const D1VectorType& DistVector, const Vector3d& Pos, const D1VectorType& Weights, DenseAttrAccessor& Attrs)
{
	int Num = DistVector.size();
	for (int i = 0; i < Num; ++i)
	{
		const Vector3d& Grad = GradientMatrix.GetColumn(i);
		const double Dist = DistVector.GetElement(i);
		const double weight = Weights.GetElement(i);

		double AttrValue = 0.;
		if (!(weight < 1.e-6))
		{
			AttrValue = Pos.dot(Grad) + Dist;
			AttrValue /= weight * Area;
		}
		Attrs[i] = -AttrValue;
	}
}

inline void WedgeQuadric::SumOuterProducts(const B1MatrixType& GradientArray, const D1VectorType& DistArray, SymmetricMatrix& OuterProductSum, Vector3d& DistGradientSum)
{
	int Num = GradientArray.NumCols();
	for (int i = 0; i < Num; ++i)
	{
		const Vector3d& Grad = GradientArray.GetColumn(i);
		const double Dist = DistArray.GetElement(i);

		DistGradientSum += Dist * Grad;
		OuterProductSum += ScaledProjectionOperator(Grad);
	}
}


//---------------------------------------------------------------------------------

class QuadricOptimizer : public QuadricBase
{
public:
	typedef QuadricBase		MyBase;

	QuadricOptimizer() :MyBase() {}

	void AddEdgeQuadric(const EdgeQuadric& eq)
	{
		CMatrix += eq.CMatrix;
		D0Vector += eq.D0Vector;
		CScalar += eq.Scalar;
	}

	void AddWedgeQuadric(const WedgeQuadric& wq)
	{
		MyBase::operator+=(wq);
	}

	bool Optimize(Vector3d& OptimalPosition, const bool PreserveVolume, const double VolumeImportance) const
	{
		/**
			* Optimizing the quadric requires inverting a matrix system:
			*
			*                         3            N          M
			* Quadric Matrix    ( C_Matrix,    B1_Matrix, B2_Matrix )   3
			*                   ( B1_Matrix^T, Gamma * I,    0      )   N
			*                   ( B2_Matrix^T,     0    ,  Gamma *I )   M
			*
			* Quadric Vector            ( D0_Vector )  3
			*                           ( D1_Vector )  N
			*                           ( D2_Vector )  M
			*
			*  Solve:
			*     Quadric_Matrix * Vector = -Quadric_Vector
			*
			* for Vector.    Here Vector = (  Pos )  3
			*                              (   S1 )  N
			*                              (   S2 )  M
			* with "Pos" being the position and S1 & S2 are state for the dense (S1) and sparse (S2) attributes.
			*
			* To solve, a little manipulation shows that the state can be computed from the position
			*
			*    S1 = -1/gamma ( D1_Vector + B1_Matrix^T * Pos )
			*    S2 = -1/gamma ( D2_Vector + B2_Matrix^T * Pos )
			*
			*  and the position can be obtained by inverting a symmetric 3x3 matrix,
			*
			*  [ C_Matrix - 1/gamma (B1_Matrix * B1_Matrix^T + B2_Matrix * B2_Matrix^T ) ] * Pos =
			*                                     1/gamma [B1_Matrix * D1_Vector + B2_Matrix * D2_Vector] - D0_Vector
			*
			*   Lhs_Matrix * Pos = Rhs_Vector
			*
			*      The optimal position is given by   Pos = Inverse(Lhs_Matrix) * Rhs_Vector
			*
			*  If Volume Preservation is desired, a scalar Lagrange multiplier 'lm' is used to inflate the system
			*
			*
			*            3                 1
			*      ( Lhs_Matrix,      Vol_Gradient )  ( Pos )    = ( Rhs_Vector     )
			*      ( Vol_Gradient^T,       0       )  (  lm )      ( -Vol_Grad_Dist )
			*
			*
			*     lm  =  (Vol_Grad_Dist +  <Vol_Grad | InvL * RHS_Vector>  ) / <Vol_Grad | InvL * Vol_Grad>
			*
			*     Pos  = InvL * Rhs_Vec - lm InvL * Vol_Grad
			*
			*     where InvL = Lhs_Matrix.Inverse()
			*
			*     Note:  InvL * Rhs_Vec is the unconstrained solution (if you ignored volume preservation)
			*            and  -lm * InvL * Vol_Grad   is the correction.
			*
			*    notation:
			*    <VectorA | VectorB> = DotProduct(VectorA, VectorB).
			*/


		const double Threshold = 1.e-12;

		if (Gamma < Threshold) return false;

		// LHS

		SymmetricMatrix LhsMatrix = CMatrix - (1. / Gamma) * (OuterProductOperator(B1Matrix));

		// Invert:

		bool bSucces = true;
		const SymmetricMatrix InvLhsMatrix = LhsMatrix.Inverse(bSucces, Threshold);

		// Return false if we can't find any optimal position.

		if (bSucces)
		{

			// RHS

			Vector3d RhsVector = (1. / Gamma) * (B1Matrix * D1Vector) - D0Vector;

			// The optimal position with out the volume constraint

			OptimalPosition = InvLhsMatrix * RhsVector;

			if (PreserveVolume)
			{
				const Vector3d InvLhsGVol = InvLhsMatrix * VolGradConstraint;
				const double GVolInvLhsGVol = VolGradConstraint.dot(InvLhsGVol);

				// Check that the constraint can be satisfied.
				if (abs(GVolInvLhsGVol) > Threshold)
				{
					const double LagrangeMultiplier = (1. / GVolInvLhsGVol) * (VolDistConstraint + VolGradConstraint.dot(OptimalPosition));
					const Vector3d VolumeCorrection = -LagrangeMultiplier * InvLhsGVol;

					OptimalPosition += VolumeImportance * VolumeCorrection;
				}
			}
		}
		return bSucces;
	}

};