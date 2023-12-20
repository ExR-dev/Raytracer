#include "Plane.h"


Plane::Plane(const Vector3D & colour, const Vector3D & p, const Vector3D & n) : 
	Shape(colour), origin(p), normal(n)
{
    normal.Normalize();
}

bool Plane::Intersection(const Ray & ray, double & t)
{
    double a = normal * ray.direction;

    // Backface culling
    if (a >= 0)
        return false; // Ray is orthogonal to or facing same direction as plane

    if (normal * (origin - ray.origin) >= 0)
        return false; // Ray is behind plane

    t = ((normal * origin) - normal * ray.origin) / a;
    return true;
}
