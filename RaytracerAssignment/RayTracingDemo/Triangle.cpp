#include "Triangle.h"

#include <cmath>


Triangle::Triangle(const Vector3D& colour, const Vector3D& p0, const Vector3D& p1, const Vector3D& p2) : 
	Shape(colour), p0(p0), p1(p1), p2(p2)
{}

bool Triangle::Intersection(const Ray& ray, double& t)
{
    Vector3D bXAxis = p1 - p0; // Barycentric axis towards p1
    Vector3D bYAxis = p2 - p0; // Barycentric axis towards p2

    Vector3D idealBXAxis = ray.direction ^ bYAxis; // Direction of barycentric x-axis if it is orthogonal with y-axis and ray
    double bXAxisRatio = bXAxis * idealBXAxis; // Length of barycentric x-axis from perspective of ray

    if (abs(bXAxisRatio) < 0.000000000001)
        return false; // Ray is parallell with triangle or triangle is infinitely thin

    Vector3D dirToBOrigin = ray.origin - p0;
    double invBXAxisRatio = 1.0 / bXAxisRatio;
    double bU = invBXAxisRatio * (dirToBOrigin * idealBXAxis);

    if (bU < 0.0 || bU > 1.0)
        return false;

    Vector3D idealBYAxis = dirToBOrigin ^ bXAxis;
    double bV = invBXAxisRatio * (ray.direction * idealBYAxis);

    if ((bV < 0.0) || (bV > 1.0 - bU))
        return false;

    t = invBXAxisRatio * (bYAxis * idealBYAxis);
    return (t > 0.0);
}
