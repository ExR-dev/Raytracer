#include "Plane.h"



Plane::Plane(const Vector3D & colour, const Vector3D & p, const Vector3D & n) : 
	Shape(colour), origin(p), normal(n)
{
    normal.Normalize();
}

bool Plane::Intersection(const Ray & ray, double & t)
{
    double a = normal * ray.direction;

    if (a >= 0)
        return false;

    if (normal * (origin - ray.origin) >= 0)
        return false;

    t = ((normal * origin) - normal * ray.origin) / (normal * ray.direction);
    return true;
}
