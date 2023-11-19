
#include "Vector3D.h"

#include <cmath>


Vector3D::Vector3D(double xValue, double yValue, double zValue) :
	x(xValue), y(yValue), z(zValue)
{}

Vector3D Vector3D::operator+(const Vector3D& right) const
{
	return Vector3D(x + right.x, y + right.y, z + right.z);
}

Vector3D Vector3D::operator-(const Vector3D& right) const
{
	return Vector3D(x - right.x, y - right.y, z - right.z);
}

/// <summary>Dot product</summary>
double Vector3D::operator*(const Vector3D& right) const
{
	return (x * right.x) + (y * right.y) + (z * right.z);
}

/// <summary>Cross product</summary>
Vector3D Vector3D::operator^(const Vector3D& right) const
{
	return Vector3D(
		y * right.z - z * right.y,
		z * right.x - x * right.z,
		x * right.y - y * right.x
	);
}

Vector3D Vector3D::operator*(double scalar) const
{
	return Vector3D(x * scalar, y * scalar, z * scalar);
}

double Vector3D::GetX() const
{
	return x;
}

double Vector3D::GetY() const
{
	return y;
}

double Vector3D::GetZ() const
{
	return z;
}

double Vector3D::Length() const
{
	return sqrt(x*x + y*y + z*z);
}

void Vector3D::Normalize()
{
	(*this) = (*this) * (1.0 / Length());
}
