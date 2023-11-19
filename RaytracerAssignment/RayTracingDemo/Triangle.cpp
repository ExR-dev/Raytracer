#include "Triangle.h"

#include <cmath>


Triangle::Triangle(const Vector3D& colour, const Vector3D& p0, const Vector3D& p1, const Vector3D& p2) : 
	Shape(colour), p0(p0), p1(p1), p2(p2)
{}

bool Triangle::Intersection(const Ray& ray, double& t)
{
    Vector3D p0ToP1 = p1 - p0;
    Vector3D p0ToP2 = p2 - p0;

    // Backface culling
    //if ((p0ToP1 ^ p0ToP2) * ray.direction >= 0.0)
    //    return false;

    Vector3D h = ray.direction ^ p0ToP2;
    double a = p0ToP1 * h;

    if (abs(a) < 0.000000000001)
        return false;

    double f = 1.0 / a;
    Vector3D s = ray.origin - p0;
    double u = f * (s * h);

    if (u < 0.0 || u > 1.0)
        return false;

    Vector3D q = s ^ p0ToP1;
    double v = f * (ray.direction * q);

    if ((v < 0.0) || (v > 1.0 - u))
        return false;

    t = f * (p0ToP2 * q);
    return (t > 0.0);
}
