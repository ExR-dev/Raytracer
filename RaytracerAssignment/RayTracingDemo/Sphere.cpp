#include "Sphere.h"

#include <iostream>


Sphere::Sphere(const Vector3D& colour, const Vector3D c, double r) : 
    Shape(colour), center(c), radius(r)
{}

bool Sphere::Intersection(const Ray& ray, double& t)
{
    Vector3D centerToRayOrigin = ray.origin - center; // Sphere center to ray origin
    double projDist = centerToRayOrigin * ray.direction; // Distance from sphere to ray origin along ray direction

    Vector3D centerToRayMin = centerToRayOrigin - ray.direction * projDist; // Sphere center to closest point of ray
    double hitOffset = (radius * radius) - (centerToRayMin * centerToRayMin); // Offset of closest point of ray to sphere bounds, squared

    if (hitOffset < -0.000000000001)
        return false; // Closest point of ray is past bounds of sphere

    hitOffset = sqrt(std::max(0.0, hitOffset));

    double // Distance from ray to entry & exit 
        t0 = -hitOffset - projDist,
        t1 =  hitOffset - projDist;

    if (t0 > t1) // Intersection must be closest distance
        std::swap(t0, t1);

    if (t0 < 0.0) // Intersection must be positive
    {
        if (t1 < 0.0) return false;
        t0 = t1;
    }

    t = t0;
    return true;
}