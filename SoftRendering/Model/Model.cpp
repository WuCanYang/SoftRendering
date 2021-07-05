#include "Model.h"
#include <math.h>
#include "Constant.h"

Model::Model() : Position(0.0f, 0.0f, -0.5f), Rotation(0.0f), Scale(0.5f), modelTexture(nullptr)
{

}

Model::~Model()
{
	if (modelTexture)
		delete modelTexture;
}

void Model::update()
{
	//if(modelTexture != nullptr) Rotation._y += 10.0f;
	//if (modelTexture == nullptr) Position._x += 0.3f;
}

Matrix4X4 Model::ScaleMatrix()
{
	Matrix4X4 m;
	m.m11 = Scale.x();
	m.m22 = Scale.y();
	m.m33 = Scale.z();
	m.m44 = 1.0f;
	return m;
}

Matrix4X4 Model::RotateMatrix()
{
	Matrix4X4 mx;
	float theta = Rotation.x() / 180.0f * PI;
	mx.m11 = 1.0f;
	mx.m22 = cosf(theta);
	mx.m23 = -sinf(theta);
	mx.m32 = sinf(theta);		// rotate x axis
	mx.m33 = cosf(theta);
	mx.m44 = 1.0f;

	Matrix4X4 my;
	theta = Rotation.y() / 180.0f * PI;
	my.m11 = cosf(theta);
	my.m13 = sinf(theta);
	my.m22 = 1.0f;				// rotate y axis
	my.m31 = -sinf(theta);
	my.m33 = cosf(theta);
	my.m44 = 1.0f;

	Matrix4X4 mz;
	theta = Rotation.z() / 180.0f * PI;
	mz.m11 = cosf(theta);
	mz.m12 = -sinf(theta);
	mz.m21 = sinf(theta);		// rotate z axis
	mz.m22 = cosf(theta);
	mz.m33 = 1.0f;
	mz.m44 = 1.0f;

	return mz * my * mx;
}

Matrix4X4 Model::TranslateMatrix()
{
	Matrix4X4 m;
	m.m11 = m.m22 = m.m33 = m.m44 = 1.0f;
	m.m14 = Position.x();
	m.m24 = Position.y();
	m.m34 = Position.z();
	return m;
}

Matrix4X4 Model::GetModelMatrix()
{
	Matrix4X4 scaleMatrix = ScaleMatrix();
	Matrix4X4 rotateMatrix = RotateMatrix();
	Matrix4X4 translateMatrix = TranslateMatrix();

	return translateMatrix * rotateMatrix * scaleMatrix;
}
