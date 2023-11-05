#include "OBB.h"

#include <iostream>
#include <cmath>


OBB::OBB(
	const Vector3D& colour,
	const Vector3D& centerPoint,
	const Vector3D& u, const Vector3D& v, const Vector3D& w,
	double width, double height, double depth) :
	Shape(colour),
	center(centerPoint),
	axes{u, v, w},
	halfLengths{width, height, depth}
{
	axes[0].Normalize();
	axes[1].Normalize();
	axes[2].Normalize();

	if (abs(abs(axes[0] * (axes[1] ^ axes[2])) - 1.0) > 0.000000000001)
	{
		std::cout << "Warning: Provided base vectors in OBB constructor are not orthogonal. Substituting with cross of x-axis.\n";

		Vector3D xyCross = axes[0] ^ axes[1];
		double zSign = (axes[2] * xyCross > 0.0) ? 1.0 : -1.0;
		axes[2] = xyCross * zSign;
		axes[2].Normalize();

		Vector3D xzCross = axes[0] ^ axes[2];
		double ySign = (axes[1] * xzCross > 0.0) ? 1.0 : -1.0;
		axes[1] = xzCross * ySign;
		axes[1].Normalize();
	}
}

bool OBB::Intersection(const Ray& ray, double& t)
{
	double 
		min = std::numeric_limits<double>::min(),
		max = std::numeric_limits<double>::max();

	Vector3D p = center - ray.origin;

	for (int i = 0; i < 3; i++)
	{
		Vector3D axis = axes[i];
		double halfLength = halfLengths[i];

		double e = axis * p;
		double f = axis * ray.direction;

		if (abs(f) > 0.000000000001)
		{
			double
				t0 = (e + halfLength) / f,
				t1 = (e - halfLength) / f;

			if (t0 > t1)	std::swap(t0, t1);

			if (t0 > min)	min = t0;
			if (t1 < max)	max = t1;

			if (min > max)	return false;
			if (max < 0.0)	return false;
		}
		else if (-e - halfLength > 0.0 || -e + halfLength < 0.0)
			return false;
	}

	if (min > 0.0)	t = min;
	else			t = max;

	return true;
}
