#include "Triangle.h"

#include <cmath>


Triangle::Triangle(const Vector3D& colour, const Vector3D& p0, const Vector3D& p1, const Vector3D& p2) : 
	Shape(colour), p0(p0), p1(p1), p2(p2)
{}

bool Triangle::Intersection(const Ray& ray, double& t)
{
    Vector3D baU = p1 - p0; // Barycentric u-axis
    Vector3D baV = p2 - p0; // Barycentric v-axis

    Vector3D orthoBaU = ray.direction ^ baV; // Direction of u-axis if it is orthogonal to v-axis and ray
    double baUScale = baU * orthoBaU; // Length of u-axis from ray's perspective

    if (abs(baUScale) < 0.000000000001)
        return false; // Ray is parallel to triangle or triangle is infinitely thin

    Vector3D rayToBOrig = ray.origin - p0;
    double baUScaleInv = 1.0 / baUScale;
    double bU = baUScaleInv * (rayToBOrig * orthoBaU); // Ray's barycentric u-coordinate

    if (bU < 0.0 || bU > 1.0)
        return false; // Ray misses triangle in u-axis

    Vector3D orthoBaV = rayToBOrig ^ baU; // Direction of v-axis if it is orthogonal with u-axis
    double bV = baUScaleInv * (ray.direction * orthoBaV); // Ray's barycentric v-coordinate

    if (bV < 0.0 || bV > (1.0 - bU))
        return false; // Ray misses triangle in v-axis

    t = baUScaleInv * (baV * orthoBaV);
    return (t > 0.0);
}
