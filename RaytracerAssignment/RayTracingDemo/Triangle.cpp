#include "Triangle.h"

Triangle::Triangle(const Vector3D& colour, const Vector3D& p0, const Vector3D& p1, const Vector3D& p2) : 
	Shape(colour), p0(p0), p1(p1), p2(p2)
{}

bool Triangle::Intersection(const Ray& ray, double& t)
{
    Vector3D edge1 = p1 - p0;
    Vector3D edge2 = p2 - p0;

    /*Vector3D kk = edge1 ^ edge2;
    if (kk * ray.dir >= 0)
        return false;*/

    Vector3D h = ray.direction ^ edge2;
    double a = edge1 * h;

    /*if (a > -0.00001 && a < 0.00001)
        return false;*/
    if (a == 0.0)
        return false;

    double f = 1.0 / a;
    Vector3D s = ray.origin - p0;
    double u = f * (s * h);

    if (u < 0 || u > 1)
        return false;

    Vector3D q = s ^ edge1;
    double v = f * (ray.direction * q);

    if (v < 0 || u + v > 1)
        return false;

    t = f * (edge2 * q);
    return (t > 0);
}
