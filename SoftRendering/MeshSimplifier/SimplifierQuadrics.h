#pragma once
#include "SimplifierDataStructure.h"
#include "SimplifierVertex.h"

//边的二次误差度量，给边界边添加一个约束，让其代价变大，防止变形
class EdgeQuadric
{
	SymmetricMatrix		CMatrix;
	Vector3d			D0Vector;
	double				Scalar;

public:

	EdgeQuadric() {}

	EdgeQuadric(const Vector3d& v0Pos, const Vector3d& v1Pos, const Vector3d& FaceNormal, const double EdgeWeight);

	EdgeQuadric(EdgeQuadric& other) :
		CMatrix(other.CMatrix),
		D0Vector(other.D0Vector),
		Scalar(other.Scalar)
	{}

	EdgeQuadric& operator+=(const EdgeQuadric& other)
	{
		CMatrix += other.CMatrix;
		D0Vector += other.D0Vector;
		Scalar += other.Scalar;
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
		VolGradConstraint(Other.VolGradConstraint)
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

	WedgeQuadric(const MeshVertType& TriVert0, const MeshVertType& TriVert1, const MeshVertType& TriVert2, const D1VectorType& BasicWeights);

	WedgeQuadric& operator+=(const WedgeQuadric& Other)
	{
		MyBase::operator+=(Other);
		return *this;
	}

	double Evaluate(const MeshVertType& Vert, const D1VectorType& BasicWeights) const;

	void CalcAttributes(MeshVertType& Vert, const D1VectorType& BasicWeights) const;

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

	bool    EncodeAttrGradient(const InverseGradientProjection& GradientTool,
		const DenseAttrAccessor& Vert0Attrs,
		const DenseAttrAccessor& Vert1Attrs,
		const DenseAttrAccessor& Vert2Attrs,
		const D1VectorType& Weights,
		B1MatrixType& GradientMatrix,
		D1VectorType& DistanceVector);


	static void ComputeAttrs(const double Area, const B1MatrixType& GradientMatrix, const D1VectorType& DistVector, const Vector3d& Pos, const D1VectorType& Weights, DenseAttrAccessor& Attrs);


	static void SumOuterProducts(const B1MatrixType& GradientArray, const D1VectorType& DistArray, SymmetricMatrix& OuterProductSum, Vector3d& DistGradientSum);

};



