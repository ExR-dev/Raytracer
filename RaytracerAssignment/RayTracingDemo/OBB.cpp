#include "OBB.h"

#include <iostream>


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
}

/// <summary> Ray-OBB intersection check </summary>
/// <param name="ray"> The ray to check the OBB with </param>
/// <param name="t"> The distance to the first positive intersection </param>
/// <returns> The result of the intersection check </returns>
bool OBB::Intersection(const Ray& ray, double& t)
{
	double // Distances to entry & exit
		min = -std::numeric_limits<double>::max(),
		max = std::numeric_limits<double>::max();

	Vector3D rayToCenter = center - ray.origin;

	for (int i = 0; i < 3; i++)
	{ // Check each axis individually
		Vector3D axis = axes[i];
		double halfLength = halfLengths[i];

		double
			distAlongAxis = axis * rayToCenter, // Distance from ray to OBB center along axis
			f = axis * ray.direction; // Length of direction 

		if (abs(f) > 0.000000000001)
		{ // Ray is not orthogonal with axis
			double
				t0 = (distAlongAxis + halfLength) / f,
				t1 = (distAlongAxis - halfLength) / f;

			if (t0 > t1)	std::swap(t0, t1);

			if (t0 > min)	min = t0; // Keep the longer entry-point
			if (t1 < max)	max = t1; // Keep the shorter exit-point

			if (min > max)	return false; // Ray misses OBB
			if (max < 0.0)	return false; // OBB is behind ray
		}
		else if (distAlongAxis <= -halfLength || distAlongAxis >= halfLength)
			return false; // Ray is orthogonal with axis but not located between the axis-planes
	}

	t = (min > 0.0) ? min : max; // Return the closest positive intersection
	return true;
}
