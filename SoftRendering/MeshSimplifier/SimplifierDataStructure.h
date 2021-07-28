#pragma once
#include "Math/Vector3d.h"
#include "Math/Vector2.h"
#include "SimplifierMacros.h"
#include <memory>
#include <iostream>
#include <math.h>

//对称 3X3 矩阵
class SymmetricMatrix
{
public:

	double Data[6];

	SymmetricMatrix()
	{
		Reset();
	}

	SymmetricMatrix(double a11, double a12, double a13,
								double a22, double a23,
											double a33)
	{
		Data[0] = a11; Data[1] = a12; Data[2] = a13;
					   Data[3] = a22; Data[4] = a23;
									  Data[5] = a33;
	}

	SymmetricMatrix(const SymmetricMatrix& other)
	{
		memcpy(Data, other.Data, sizeof(Data));
	}

	void Reset()
	{
		memset(Data, 0, sizeof(Data));
	}

	double& operator[](const int idx)
	{
		check(idx < 6);
		return Data[idx];
	}

	const double& operator[](const int idx) const
	{
		check(idx < 6);
		return Data[idx];
	}

	Vector3d operator*(const Vector3d& Vect) const
	{
		Vector3d Result(
			Vect[0] * Data[0] + Vect[1] * Data[1] + Vect[2] * Data[2],
			Vect[0] * Data[1] + Vect[1] * Data[3] + Vect[2] * Data[4],
			Vect[0] * Data[2] + Vect[1] * Data[4] + Vect[2] * Data[5]
		);
		return Result;
	}

	SymmetricMatrix operator*(const SymmetricMatrix& Other) const
	{
		SymmetricMatrix Result(Data[0] * Other[0] + Data[1] * Other[1] + Data[2] * Other[2], Data[0] * Other[1] + Data[1] * Other[3] + Data[2] * Other[4], Data[0] * Other[2] + Data[1] * Other[4] + Data[2] * Other[5],
																							 Data[1] * Other[1] + Data[3] * Other[3] + Data[4] * Other[4], Data[1] * Other[2] + Data[3] * Other[4] + Data[4] * Other[5],
																																						   Data[2] * Other[2] + Data[4] * Other[4] + Data[5] * Other[5]);
		return Result;
	}

	SymmetricMatrix operator*(const double Scalar) const
	{
		SymmetricMatrix Result(*this);
		for (int i = 0; i < 6; ++i)
		{
			Result[i] *= Scalar;
		}
		return Result;
	}

	SymmetricMatrix operator-(const SymmetricMatrix& Other) const
	{
		SymmetricMatrix Result(*this);
		for (int i = 0; i < 6; ++i)
		{
			Result[i] -= Other[i];
		}
		return Result;
	}

	SymmetricMatrix& operator*=(const double Scalar)
	{
		for (int i = 0; i < 6; ++i)
		{
			Data[i] *= Scalar;
		}
		return *this;
	}

	SymmetricMatrix& operator+=(const SymmetricMatrix& Other)
	{
		for (int i = 0; i < 6; ++i)
		{
			Data[i] += Other[i];
		}
		return *this;
	}

	void Zero()
	{
		Reset();
	}

	void Identity()
	{
		Data[0] = 1.0; Data[1] = 0.0; Data[2] = 0.0;
					   Data[3] = 1.0; Data[4] = 0.0;
								      Data[5] = 1.0;
	}

	double Det() const
	{

		return	-Data[2] * Data[2] * Data[3] +
			2. * Data[1] * Data[2] * Data[4] +
				-Data[0] * Data[4] * Data[4] +
				-Data[1] * Data[1] * Data[5] +
				 Data[0] * Data[3] * Data[5];
	}

	SymmetricMatrix Inverse(bool& Success, double Threshold = 1.e-8) const
	{
		SymmetricMatrix Result(-Data[4] * Data[4] + Data[3] * Data[5], Data[2] * Data[4] - Data[1] * Data[5], -Data[2] * Data[3] + Data[1] * Data[4],
																	  -Data[2] * Data[2] + Data[0] * Data[5],  Data[1] * Data[2] - Data[0] * Data[4],
																											  -Data[1] * Data[1] + Data[0] * Data[3]);

		double InvDet = Det();
		Success = (abs(InvDet) > Threshold);

		InvDet = 1. / InvDet;


		Result *= InvDet;

		return Result;
	}
};


static SymmetricMatrix ScaledProjectionOperator(const Vector3d& Vec)
{
	SymmetricMatrix Result(Vec[0] * Vec[0], Vec[0] * Vec[1], Vec[0] * Vec[2],
											Vec[1] * Vec[1], Vec[1] * Vec[2],
															 Vec[2] * Vec[2]);

	return Result;
}


//-------------------------------------------------------------------------------

template<int Size>
class DenseVecD
{
public:

	double Data[Size];

	DenseVecD()
	{
		Reset();
	}

	DenseVecD(const DenseVecD& Other)
	{
		memcpy(Data, Other.Data, sizeof(Data));
	}

	void Reset()
	{
		memset(Data, 0, sizeof(Data));
	}

	int size() const { return Size; }

	void SetElement(const int idx, const double val)
	{
		check(idx < Size);
		Data[idx] = val;
	}

	double GetElement(const int idx) const
	{
		check(idx < Size);
		return Data[idx];
	}

	double& operator[](const int idx)
	{
		check(idx < Size);
		return Data[idx];
	}

	const double& operator[](const int idx) const
	{
		check(idx < Size);
		return Data[idx];
	}

	DenseVecD& operator=(const DenseVecD& Other)
	{
		memcpy(Data, Other.Data, sizeof(Data));
		return *this;
	}

	DenseVecD& operator+=(const DenseVecD& Other)
	{
		for (int i = 0; i < Size; ++i)
		{
			Data[i] += Other.Data[i];
		}
		return *this;
	}

	DenseVecD& operator*=(const double scalar)
	{
		for (int i = 0; i < Size; ++i)
		{
			Data[i] *= scalar;
		}
		return *this;
	}

	double dot(const DenseVecD& Other) const
	{
		double Result;
		for (int i = 0; i < Size; ++i)
		{
			Result += Data[i] * Other.Data[i];
		}
		return Result;
	}

	double L2NormSqr() const
	{
		double Result;
		for (int i = 0; i < Size; ++i)
		{
			Result += Data[i] * Data[i];
		}
		return Result;
	}
};


template<int Cols>
class DenseBMatrix
{
public:
	
	Vector3d Data[Cols];



	DenseBMatrix()
	{
		Reset();
	}

	DenseBMatrix(const DenseBMatrix& Other)
	{
		memcpy(Data, Other.Data, sizeof(Data));
	}

	void Reset()
	{
		memset(Data, 0, sizeof(Data));
	}

	int NumCols() const { return Cols; }

	void SetColumn(int i, const Vector3d& val)
	{
		check(i < Cols);
		Data[i] = val;
	}

	Vector3d& GetColumn(int i)
	{
		check(i < Cols);
		return Data[i];
	}

	const Vector3d& GetColumn(int i) const
	{
		check(i < Cols);
		return Data[i];
	}

	DenseBMatrix& operator=(const DenseBMatrix& Other)
	{
		memcpy(Data, Other.Data, sizeof(Data));
		return *this;
	}

	DenseBMatrix& operator+=(const DenseBMatrix& Other)
	{
		for (int i = 0; i < Cols; ++i)
		{
			Data[i] += Other.Data[i];
		}
		return *this;
	}

	Vector3d operator*(const DenseVecD<Cols>& Vec) const
	{
		Vector3d Result;
		for (int i = 0; i < Cols; ++i)
		{
			Result += Vec.Data[i] * Data[i];
		}
		return Result;
	}

	DenseBMatrix& operator*=(const double& scalar)
	{
		for (int i = 0; i < Cols; ++i)
		{
			Data[i] *= scalar;
		}
		return *this;
	}
};

/**
* Produce a new SymetricMatrix that is the result of SymetricMatrix times scalar
* NB: ThisMatrix * Scalar
*
* @param Scalar  - single scale.
*
* @return  ThisMatrix * Scalar
*/
static SymmetricMatrix operator*(double Scalar, SymmetricMatrix Mat)
{
	return Mat * Scalar;
}

/**
* vector * Matrix.
*  NB: This is really vector^Transpose * Matrix
*      and results in a vector^Transpose.   But since we don't distinguish
*      the transpose space, we treat both as vectors.
* @param LhsVector - vector
* @param SymMatrix - SymmetricMatrix,
*/
static Vector3d operator*(const Vector3d& LhsVector, const SymmetricMatrix& SymMatrix)
{
	// Vector^Transpose * Matrix = (Matrix^Transpose * Vector )^Transpose.
	// but our matrix is symmetric (Matrix^Transpose = Matrix)
	return SymMatrix * LhsVector;
}


template<int SIZE>
SymmetricMatrix OuterProductOperator(const DenseBMatrix<SIZE>& DenseB)
{
	// counting on return value optimization
	SymmetricMatrix Result;  // default zero initialization

	for (int i = 0; i < SIZE; ++i)  Result += ScaledProjectionOperator(DenseB.GetColumn(i));

	return Result;
}

//-------------------------------------------------------------------------------

class DMatrix
{
public:

	double Data[9];

	DMatrix()
	{
		Reset();
	}
	/**
	* Constructor: Element by element.
	*/
	DMatrix(double a11, double a12, double a13,
		double a21, double a22, double a23,
		double a31, double a32, double a33)
	{
		Data[0] = a11; Data[1] = a12; Data[2] = a13;
		Data[3] = a21; Data[4] = a22; Data[5] = a23;
		Data[6] = a31; Data[7] = a32; Data[8] = a33;
	}

	/**
	* Constructor: Row based.
	*/
	DMatrix(const Vector3d& Row0,
			const Vector3d& Row1,
			const Vector3d& Row2)
	{
		Data[0] = Row0[0];  Data[1] = Row0[1];  Data[2] = Row0[2];
		Data[3] = Row1[0];  Data[4] = Row1[1];  Data[5] = Row1[2];
		Data[6] = Row2[0];  Data[7] = Row2[1];  Data[8] = Row2[2];
	}

	DMatrix(const DMatrix& Other)
	{
		memcpy(Data, Other.Data, sizeof(Data));
	}

	void Reset()
	{
		memset(Data, 0, sizeof(Data));
	}

	/**
	* Accesses elements in the matrix using standard M(i,j) notation

	* @param i - the row  in [0,3)
	* @param j - the column in [0, 3)
	*/
	double operator()(int i, int j) const
	{
		check(-1 < i && i < 3 && -1 < j && j < 3);
		return Data[j + i * 3];
	}

	double& operator()(int i, int j)
	{
		check(-1 < i && i < 3 && -1 < j && j < 3);
		return Data[j + i * 3];
	}

	double& operator[](const int idx)
	{
		check(idx < 9);
		return Data[idx];
	}

	const double& operator[](const int idx) const
	{
		check(idx < 9);
		return Data[idx];
	}

	/**
	* Produce a new Vec3d that is the result of Matrix vector multiplication.
	* NB: this does M*v  not v * M
	*
	* @param Vect  - three dimensional double precision vector
	*
	* @return  Matrix * Vect.
	*/
	Vector3d operator* (const Vector3d& Vect) const
	{
		Vector3d Result(
			Vect[0] * Data[0] + Vect[1] * Data[1] + Vect[2] * Data[2],
			Vect[0] * Data[3] + Vect[1] * Data[4] + Vect[2] * Data[5],
			Vect[0] * Data[6] + Vect[1] * Data[7] + Vect[2] * Data[8]
		);

		return Result;
	}

	/**
	* Produce a new 3x3 matrix that is the result of 3x3matrix  times 3x3matridx
	* NB: ThisMatrix * Other
	*
	* @param Other  - 3x3 full matrix
	*
	* @return  ThisMatrix * Other
	*/
	DMatrix operator*(const DMatrix& B) const
	{

		DMatrix Result(
			Data[0] * B[0] + Data[1] * B[3] + Data[2] * B[6], Data[0] * B[1] + Data[1] * B[4] + Data[2] * B[7], Data[0] * B[2] + Data[1] * B[5] + Data[2] * B[8],
			Data[3] * B[0] + Data[4] * B[3] + Data[5] * B[6], Data[3] * B[1] + Data[4] * B[4] + Data[5] * B[7], Data[3] * B[2] + Data[4] * B[5] + Data[5] * B[8],
			Data[6] * B[0] + Data[7] * B[3] + Data[8] * B[6], Data[6] * B[1] + Data[7] * B[4] + Data[8] * B[7], Data[6] * B[2] + Data[7] * B[5] + Data[8] * B[8]
		);

		return Result;
	}

	DMatrix& operator*=(const double scalar)
	{
		for (int i = 0; i < 9; ++i)
		{
			Data[i] *= scalar;
		}
		return *this;
	}


	/**
	* Update this Matrix to an identity matrix (1 on diagonal 0 off diagonal).
	*/
	void Identity()
	{
		Data[0] = 1.;  Data[1] = 0.;  Data[2] = 0.;
		Data[3] = 0.;  Data[4] = 1.;  Data[5] = 0.;
		Data[6] = 0.;  Data[7] = 0.;  Data[8] = 1.;
	}

	/**
	* Determinant of the matrix.   The matrix may be inverted if this is non-zero.
	*/
	double Det() const
	{

		return -Data[2] * Data[4] * Data[6] +
				Data[1] * Data[5] * Data[6] +
				Data[2] * Data[3] * Data[7] +
			   -Data[0] * Data[5] * Data[7] +
			   -Data[1] * Data[3] * Data[8] +
				Data[0] * Data[4] * Data[8];

	}

	/**
	* Construct the inverse of this matrix.
	* @param  Success   - On return this will be true if the inverse is valid
	* @param  Threshold - Compared against the determinant of the matrix.  If abs(Det()) < Threshold the inverse is said to have failed
	*
	* @return The inverse of 'this' matrix.
	*/
	DMatrix Inverse(bool& sucess, double threshold = 1.e-8) const
	{
		DMatrix Result(
			-Data[5] * Data[7] + Data[4] * Data[8], Data[2] * Data[7] - Data[1] * Data[8], -Data[2] * Data[4] + Data[1] * Data[5],
			Data[5] * Data[6] - Data[3] * Data[8], -Data[2] * Data[6] + Data[0] * Data[8], Data[2] * Data[3] - Data[0] * Data[5],
			-Data[4] * Data[6] + Data[3] * Data[7], Data[1] * Data[6] - Data[0] * Data[7], -Data[1] * Data[3] + Data[0] * Data[4]
		);

		double InvDet = Det();
		sucess = (abs(InvDet) > threshold);

		InvDet = 1. / InvDet;

		Result *= InvDet;

		return Result;
	}

	/**
	* Construct the inverse of this matrix.
	* NB: If abs( Det() ) of the matrix is less than 1.e-8 the result will be invalid.
	*
	* @return The inverse of 'this' matrix
	*/
	DMatrix Inverse() const
	{
		bool tmp;
		return Inverse(tmp);
	}

	/**
	* Sum of the rows - return as a vector
	*/
	Vector3d RowSum() const
	{
		Vector3d Result(Data[0] + Data[1] + Data[2],
						Data[3] + Data[4] + Data[5],
						Data[6] + Data[7] + Data[8]);

		return Result;
	}

	/**
	* Sum of the column - return as a vector
	*/
	Vector3d ColSum() const
	{
		Vector3d Result(Data[0] + Data[3] + Data[6],
						Data[1] + Data[4] + Data[7],
						Data[2] + Data[5] + Data[8]);
		return Result;
	}
private:

};

/**
		 * left multiply matrix by vector.    This is really Transpose(vector) * Matrix
		 */
static Vector3d operator*(const Vector3d& VecTranspose, const DMatrix& M)
{
	Vector3d Result(M[0] * VecTranspose[0] + M[3] * VecTranspose[1] + M[6] * VecTranspose[2],
					M[1] * VecTranspose[0] + M[4] * VecTranspose[1] + M[7] * VecTranspose[2],
					M[2] * VecTranspose[0] + M[5] * VecTranspose[1] + M[8] * VecTranspose[2]);

	return Result;
}

/**
* Dense3x3 matrix time SymmetricMatrix = results in dense3x3 matrix
*/
static DMatrix operator*(const DMatrix& Dm, const SymmetricMatrix& Sm)
{
	DMatrix Result(Sm[0] * Dm[0] + Sm[1] * Dm[1] + Sm[2] * Dm[2], Sm[1] * Dm[0] + Sm[3] * Dm[1] + Sm[4] * Dm[2], Sm[2] * Dm[0] + Sm[4] * Dm[1] + Sm[5] * Dm[2],
				   Sm[0] * Dm[3] + Sm[1] * Dm[4] + Sm[2] * Dm[5], Sm[1] * Dm[3] + Sm[3] * Dm[4] + Sm[4] * Dm[5], Sm[2] * Dm[3] + Sm[4] * Dm[4] + Sm[5] * Dm[5],
				   Sm[0] * Dm[6] + Sm[1] * Dm[7] + Sm[2] * Dm[8], Sm[1] * Dm[6] + Sm[3] * Dm[7] + Sm[4] * Dm[8], Sm[2] * Dm[6] + Sm[4] * Dm[7] + Sm[5] * Dm[8]);


	return Result;
}

/**
* SymmetricMatrix X Dense3x3 matrix  = results in dense3x3 matrix
*/
static DMatrix operator*(const SymmetricMatrix& Sm, const DMatrix& Dm)
{
	DMatrix Result(Sm[0] * Dm[0] + Sm[1] * Dm[3] + Sm[2] * Dm[6], Sm[0] * Dm[1] + Sm[1] * Dm[4] + Sm[2] * Dm[7], Sm[0] * Dm[2] + Sm[1] * Dm[5] + Sm[2] * Dm[8],
				   Sm[1] * Dm[0] + Sm[3] * Dm[3] + Sm[4] * Dm[6], Sm[1] * Dm[1] + Sm[3] * Dm[4] + Sm[4] * Dm[7], Sm[1] * Dm[2] + Sm[3] * Dm[5] + Sm[4] * Dm[8],
				   Sm[2] * Dm[0] + Sm[4] * Dm[3] + Sm[5] * Dm[6], Sm[2] * Dm[1] + Sm[4] * Dm[4] + Sm[5] * Dm[7], Sm[2] * Dm[2] + Sm[4] * Dm[5] + Sm[5] * Dm[8]);

	return Result;
}


//-------------------------------------------------------------------------------


/**
		* This class generates the interpolation coefficients vector (Vec3d) g  and distance (double) d
		* defined over the face of a triangle.
		*
		* The Position Matrix is defined in terms of the three corners of triangle
		*  {pa, pb, pc} with corresponding normal 'FaceNormal'
		*
		* Position Matrix =
		*   ( pa_0, pa_1, pa_2 )
		*   ( pb_0, pb_1, pb_2 )
		*   ( pc_0, pc_1, pc_2 )
		*
		* The actual system solved is:
		*  <pa | g> + d = s0
		*  <pb | g> + d = s1
		*  <pc | g> + d = s2
		*  <FaceNormal | g> = 0
		*
		*
		* In matrix form   ( PositionMatrix   Vec3(1) )   ( g )  = ( s )		通过这个关系式计算出属性插值的梯度和距离（g 和 d）
		*                  ( FaceNormal^T ,     0     )   ( d )    ( 0 )
		*
		* where the vector (Vec3d) 's' represents the per-vertex data that forms boundary conditions
		* for the interpolation.
		*
		*  The actual solution is given by:
		*  -- Distance
		*  double d = < FaceNormal | InvsPositionMatrix * s> / <FaceNormal | InversePositionMatrix * Vec3d(1) >;
		*  -- Gradient
		*  Vec3d  g = InversePositionMatrix * s - d * InversePositionMatrix * Vec3d(1);
		*
		* The computation is broken up do allow for reuse with multiple sets of per-vertex data.
		*/
class InverseGradientProjection
{
public:
	InverseGradientProjection(const DMatrix& PositionMatrix, const Vector3d& FaceNormal)
	{
		// Threshold for computing the matrix inverse.
		const double DetThreshold = 1.e-8;

		// Compute the inverse of the position matrix
		PosInv = PositionMatrix.Inverse(bIsValid, DetThreshold);
		if (bIsValid)
		{
			// InversePositionMatrix * Vec3(1)
			MInv1 = PosInv.RowSum();

			// <FaceNormal | InversePositionMatrix 
			Dhat = FaceNormal * PosInv;

			// <FaceNormal | InvesePositionMatrix Vec3d(1)> 
			double ReScale = Dhat[0] + Dhat[1] + Dhat[2];

			bIsValid = bIsValid && (abs(ReScale) > 1.e-8);

			// divide by <FaceNormal | InvesePositionMatrix Vec3d(1)> 
			Dhat *= (1. / ReScale);
			//now:  Dhat =  <FaceNormal | InvPos  / <FaceNormal | InvPos. Vec3d(1) >
		}
	}

	bool IsValid() const
	{
		return bIsValid;
	}

	// @param  PerVertexData - Vertex Data at vertex {0, 1, 2} stored as a Vec3d
	// @param  OutGradient   - on return, the gradient terms
	// @return  Distance
	double ComputeGradient(const Vector3d& PerVertexData, Vector3d& OutGradient) const
	{
		// PosInv . s
		Vector3d MInvS = PosInv * PerVertexData;

		// Dist = <dhat | s>
		double Distance = Dhat.dot(PerVertexData);

		// Grad =  PosInv . s - Dist * PosInv . (1, 1, 1} 
		OutGradient = MInvS - Distance * MInv1;

		return Distance;
	}

private:

	bool		bIsValid;  // Indicates if the inversions incurred a divide by very-small-number.
	DMatrix		PosInv;    // Inverse(PositionMatrix)
	Vector3d    Dhat;      // n * Inverse(PositinoMatrix) / < n | Inverse(PositionMatrix) Vec3(1) >
	Vector3d    MInv1;     // Inverse(PositionMatrix) * Vec3(1)
};


//-------------------------------------------------------------------------------


template <typename DataType>
class DenseArrayWrapper			//基本属性的封装，通过地址操作让类中的属性能够通过数组的形式来进行访问
{
public:
	typedef DataType    Type;

	DenseArrayWrapper(DataType* data, int size)
	{
		Data = data;
		Size = size;
	}

	int Num() const { return Size; }

	void SetElement(const int j, const DataType Value) { check(j < Size);  Data[j] = Value; }
	DataType GetElement(const int j) const { check(j < Size);  return Data[j]; }

	DataType& operator[] (const int j) { check(j < Size);  return Data[j]; }
	DataType operator[] (const int j) const { check(j < Size); return Data[j]; }
	DataType& operator() (const int j) { return operator[](j); }
	DataType operator() (const int j) const { return operator[](j); }

	DenseArrayWrapper& operator*= (const double& Scalar)
	{
		int Num = Size;
		for (int i = 0; i < Num; ++i) Data[i] *= Scalar;

		return  *this;
	}
	DenseArrayWrapper& operator+= (const DenseArrayWrapper& Other)
	{
		int Num = Size;
		check(Num == Other.Num());

		for (int i = 0; i < Num; ++i) Data[i] += Other.Data[i];

		return *this;
	}


	DataType DotProduct(const DenseArrayWrapper& Other) const
	{
		int Num = std::min(Size, Other.Num());

		double Result = 0.;
		for (int i = 0; i < Num; ++i)
		{
			Result += Data[i] * Other[i];
		}

		return Result;
	}

	double L2NormSqr() const
	{
		return DotProduct(*this);
	}

	bool operator==(const DenseArrayWrapper& Other) const
	{
		int Num = Size;
		bool Result = (Num == Other.Size);

		for (int i = 0; i < Num && Result; ++i)
		{
			Result = Result && (Data[i] == Other.Data[i]);
		}

		return Result;
	}

	bool operator!=(const DenseArrayWrapper& Other) const
	{
		return !operator==(Other);
	}

private:

	DataType* Data; // Note: this object doesn't own the data!
	int       Size;
};


//-------------------------------------------------------------------------------

class AABB2d
{
	float Data[4];

	float Clamp(float val, float mmin, float mmax) const
	{
		return val < mmin ? mmin : val > mmax ? mmax : val;
	}

public:
	AABB2d()
	{
		Reset();
	}

	AABB2d(const AABB2d& Other)
	{
		Data[0] = Other.Data[0];
		Data[1] = Other.Data[1];
		Data[2] = Other.Data[2];
		Data[3] = Other.Data[3];
	}


	void Reset()
	{
		Data[0] = FLT_MAX;
		Data[1] = FLT_MAX;
		Data[2] = -FLT_MAX;
		Data[3] = -FLT_MAX;
	}

	void Union(const AABB2d& Other)
	{
		Data[0] = std::min(Data[0], Other.Data[0]);
		Data[1] = std::min(Data[1], Other.Data[1]);

		Data[2] = std::max(Data[2], Other.Data[2]);
		Data[3] = std::max(Data[3], Other.Data[3]);
	}

	AABB2d& operator+=(const AABB2d& Other)
	{
		Union(Other);
		return *this;
	}

	AABB2d& operator=(const AABB2d& Other)
	{
		for (int i = 0; i < 4; ++i)
		{
			Data[i] = Other.Data[i];
		}
		return *this;
	}

	bool isValid() const
	{
		return (Data[0] <= Data[2]) && (Data[1] <= Data[3]);
	}
	
	void ExpandToInclude(const Vector2& Point)
	{
		Data[0] = std::min(Data[0], Point._x);
		Data[1] = std::min(Data[1], Point._y);

		Data[2] = std::max(Data[2], Point._x);
		Data[3] = std::max(Data[3], Point._y);
	}

	void ClampPoint(Vector2& Point) const
	{
		Point._x = Clamp(Point._x, Data[0], Data[2]);
		Point._y = Clamp(Point._y, Data[1], Data[3]);
	}

	void ClampPoint(Vector2& Point, const float Fraction) const
	{
		const float halfFrac = Fraction * 0.5f;
		const float padX = halfFrac * (Data[2] - Data[0]);
		const float padY = halfFrac * (Data[3] - Data[1]);

		Point._x = Clamp(Point._x, Data[0] - padX, Data[2] + padX);
		Point._y = Clamp(Point._y, Data[1] - padY, Data[3] + padY);
	}

	Vector2 Min() const
	{
		return Vector2(Data[0], Data[1]);
	}

	Vector2 Max() const
	{
		return Vector2(Data[2], Data[3]);
	}
};