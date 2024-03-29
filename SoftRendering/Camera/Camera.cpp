#include "Camera.h"
#include "Model/Constant.h"
#include <math.h>

Matrix4X4 Camera::GetViewMatrix()
{
	Vector3 up(0.0f, 1.0f, 0.0f);
	Vector3 direction = Position - Target;
	direction.Normalize();

	Vector3 right = up.cross(direction);
	right.Normalize();
	up = direction.cross(right);
	up.Normalize();

	Matrix4X4 viewTransform;
	viewTransform.m11 = right.x();
	viewTransform.m12 = right.y();
	viewTransform.m13 = right.z();
	viewTransform.m14 = -right.dot(Position);

	viewTransform.m21 = up.x();
	viewTransform.m22 = up.y();
	viewTransform.m23 = up.z();
	viewTransform.m24 = -up.dot(Position);

	viewTransform.m31 = direction.x();
	viewTransform.m32 = direction.y();
	viewTransform.m33 = direction.z();
	viewTransform.m34 = -direction.dot(Position);

	viewTransform.m44 = 1.0f;
	return viewTransform;
}

Matrix4X4 Camera::GetPerspectiveMatrix()
{
	Matrix4X4 m;

	float tanHalfFov = tanf(fovy / 2.0f / 180.0f * PI);
	float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	m.m11 = 1.0f / (aspect * tanHalfFov);
	m.m22 = 1.0f / (tanHalfFov);
	m.m33 = (_near + _far) / (_near - _far);
	m.m34 = 2.0f * _near * _far / (_near - _far);
	m.m43 = -1.0f;

	return m;
}
