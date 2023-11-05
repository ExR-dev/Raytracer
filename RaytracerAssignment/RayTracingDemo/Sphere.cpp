#include "Sphere.h"

#include <cmath>
#include <algorithm>


Sphere::Sphere(const Vector3D& colour, const Vector3D c, double r) : 
    Shape(colour), center(c), radius(r)
{}

bool Sphere::Intersection(const Ray& ray, double& t)
{
    Vector3D sphereToRay = ray.origin - center;
    double rayToSphereOffset = sphereToRay * ray.direction;

    Vector3D qc = sphereToRay - ray.direction * rayToSphereOffset;
    double h = radius * radius - qc * qc;

    if (h < -0.000000001)
        return false;

    h = sqrt(std::max(0.0, h));

    double
        t0 = -h - rayToSphereOffset,
        t1 = h - rayToSphereOffset;

    if (t0 > t1)
    {
        double temp = t0;
        t0 = t1;
        t1 = temp;
    }

    if (t0 < 0)
    {
        t0 = t1;
        if (t0 < 0)
            return false;
    }

    t = t0;
    return true;
}