#include "OBB.h"

OBB::OBB(
	const Vector3D& colour, 
	const Vector3D& centerPoint, 
	const Vector3D& u, const Vector3D& v, const Vector3D& w,
	double width, double height, double depth) : 
	
	Shape(colour), 
	center(centerPoint), 
	xAxis(u), yAxis(v), zAxis(w),
	xLen(width), yLen(height), zLen(depth)
{}

bool OBB::Intersection(const Ray& ray, double& t)
{
	// Implement intersection logic here

	return false;
}
