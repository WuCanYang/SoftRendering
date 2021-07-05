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

	/*std::cout << direction.x() << "   " << direction.y() << "   " << direction.z() << std::endl;
	std::cout << right.x() << "   " << right.y() << "   " << right.z() << std::endl;
	std::cout << up.x() << "   " << up.y() << "   " << up.z() << std::endl;*/

	Matrix4X4 viewTransform;
	viewTransform.m11 = right.x();
	viewTransform.m12 = right.y();
	viewTransform.m13 = right.z();
	viewTransform.m14 = -right.x() * Position.x() - right.y() * Position.y() - right.z() * Position.z();

	viewTransform.m21 = up.x();
	viewTransform.m22 = up.y();
	viewTransform.m23 = up.z();
	viewTransform.m24 = -up.x() * Position.x() - up.y() * Position.y() - up.z() * Position.z();

	viewTransform.m31 = direction.x();
	viewTransform.m32 = direction.y();
	viewTransform.m33 = direction.z();
	viewTransform.m34 = -direction.x() * Position.x() - direction.y() * Position.y() - direction.z() * Position.z();

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
