#include "Sphere.h"

#include <iostream>


Sphere::Sphere(const Vector3D& colour, const Vector3D c, double r) : 
    Shape(colour), center(c), radius(r)
{}

bool Sphere::Intersection(const Ray& ray, double& t)
{
    Vector3D centerToOrigin = ray.origin - center;
    double projectedDist = centerToOrigin * ray.direction;

    Vector3D pc = centerToOrigin - ray.direction * projectedDist;
    double hitOffset = radius * radius - (pc * pc);

    if (hitOffset < -0.000000000001)
        return false;

    hitOffset = sqrt(std::max(0.0, hitOffset));

    double
        t0 = -hitOffset - projectedDist,
        t1 =  hitOffset - projectedDist;

    if (t0 > t1)
        std::swap(t0, t1);

    if (t0 < 0.0)
    {
        t0 = t1;
        if (t0 < 0.0)
            return false;
    }

    t = t0;
    return true;
}