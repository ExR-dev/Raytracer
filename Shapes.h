#pragma once

#include "Utils.h"
#include "Vec3.h"
#include "Phys.h"
#include "Graphics.h"

#include <iostream>



struct Shape
{
    Material mat;

    Shape(const Material& mat) :
        mat(mat)
    {}

    virtual bool RayIntersect(const Ray& ray, Hit* hit) const
    {
        return false;
    }

    virtual bool PointIntersect(const Vec3& point) const
    {
        return false;
    }
};


struct AABB : Shape
{
    Vec3 min, max;

    AABB(const Vec3& min, const Vec3& max, const Material& mat) :
        Shape(mat), min(min), max(max)
    {}

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        Vec3 inv_dir = ray.InverseDir();

        double tx1 = (min.x - ray.origin.x) * inv_dir.x;
        double tx2 = (max.x - ray.origin.x) * inv_dir.x;

        double tmin = std::min(tx1, tx2);
        double tmax = std::max(tx1, tx2);

        double ty1 = (min.y - ray.origin.y) * inv_dir.y;
        double ty2 = (max.y - ray.origin.y) * inv_dir.y;

        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        double tz1 = (min.z - ray.origin.z) * inv_dir.z;
        double tz2 = (max.z - ray.origin.z) * inv_dir.z;

        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));

        bool result = tmax >= std::max(0.0, tmin) && tmin < std::numeric_limits<float>::max();

        if (hit != nullptr)
        {
            hit->len = (tmin > 0.0) ? tmin : tmax;
            hit->origin = ray.origin + (ray.dir * hit->len);
            hit->target = (void*)this;

            if (abs(hit->origin.x - min.x) < utils::MINVAL)
                hit->normal = Vec3(-1, 0, 0);
            else if (abs(hit->origin.x - max.x) < utils::MINVAL)
                hit->normal = Vec3(1, 0, 0);
            else if (abs(hit->origin.y - min.y) < utils::MINVAL)
                hit->normal = Vec3(0, -1, 0);
            else if (abs(hit->origin.y - max.y) < utils::MINVAL)
                hit->normal = Vec3(0, 1, 0);
            else if (abs(hit->origin.z - min.z) < utils::MINVAL)
                hit->normal = Vec3(0, 0, -1);
            else if (abs(hit->origin.z - max.z) < utils::MINVAL)
                hit->normal = Vec3(0, 0, 1);
        }

        return result;
    }

    bool PointIntersect(const Vec3& point) const override
    {
        if (point.x < min.x) return false;
        if (point.x > max.x) return false;
        if (point.y < min.y) return false;
        if (point.y > max.y) return false;
        if (point.z < min.z) return false;
        if (point.z > max.z) return false;
        return true;
    }
};

struct OBB : Shape
{
    Vec3 center;
    Vec3 axes[3];
    double halfLengths[3];

    OBB(const Vec3& c, const Vec3& xA, const Vec3& yA, const Vec3& zA, const Material& mat) :
        Shape(mat),
        center(c),
        axes{xA, yA, zA},
        halfLengths{xA.Mag(), 0.0, 0.0}
    {
        axes[0].Normalize();

        if (abs(abs(axes[0].Dot(axes[1].Cross(axes[2]))) - 1.0) > utils::MINVAL)
        {
            std::cout << "Warning: Provided base vectors in OBB constructor are not orthogonal. Substituting with normals of x-axis.\n";

            axes[1] = axes[1] - (axes[0] * axes[1].Dot(axes[0]));
            halfLengths[1] = axes[1].Mag();
            axes[1].Normalize();

            axes[2] = axes[2] - (axes[0] * axes[2].Dot(axes[0]));
            axes[2] = axes[2] - (axes[1] * axes[2].Dot(axes[1]));
            halfLengths[2] = axes[2].Mag();
            axes[2].Normalize();
        }
        else
        {
            halfLengths[1] = axes[1].Mag();
            axes[1].Normalize();

            halfLengths[2] = axes[2].Mag();
            axes[2].Normalize();

        }
    }

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        double
            min = std::numeric_limits<double>::min(),
            max = std::numeric_limits<double>::max();

        Vec3
            p = center - ray.origin,
            nMin = Vec3(),
            nMax = Vec3();

        for (int i = 0; i < 3; i++)
        {
            Vec3 axis = axes[i];
            double halfLength = halfLengths[i];

            double e = axis.Dot(p);
            double f = axis.Dot(ray.dir);

            if (abs(f) > utils::MINVAL)
            {
                Vec3 tnMin = axis;
                Vec3 tnMax = axis * -1.0;

                double
                    t0 = (e + halfLength) / f,
                    t1 = (e - halfLength) / f;

                if (t0 > t1)
                {
                    std::swap(t0, t1);
                    tnMin = tnMax;
                    tnMax = axis;
                }

                if (t0 > min)
                {
                    min = t0;
                    nMin = tnMin;
                }
                if (t1 < max)
                {
                    max = t1;
                    nMax = tnMax;
                }

                if (min > max)	return false;
                if (max < 0.0)	return false;
            }
            else if (-e - halfLength > 0.0 || -e + halfLength < 0.0)
                return false;
        }

        if (hit != nullptr)
        {
            if (min > 0.0)
            {
                hit->len = min;
                hit->normal = nMin;
            }
            else
            {
                hit->len = max;
                hit->normal = nMax;
            }

            hit->origin = ray.origin + ray.dir * hit->len;
            hit->normal.Normalize();
            hit->target = (void*)this;
        }

        return true;
    }

    bool PointIntersect(const Vec3& point) const override
    {
        Vec3 p = point - center;
        return 
            abs(p.Dot(axes[0])) <= halfLengths[0] &&
            abs(p.Dot(axes[1])) <= halfLengths[1] &&
            abs(p.Dot(axes[2])) <= halfLengths[2];
    }
};

struct Sphere : Shape
{
    double rad;
    Vec3 center;

    Sphere(double rad, Vec3 center, Material mat) :
        Shape(mat), rad(rad), center(center)
    {}

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        Vec3 oc = ray.origin - center;
        double b = oc.Dot(ray.dir);

        Vec3 qc = oc - ray.dir * b;
        double h = rad * rad - qc.Dot(qc);

        if (h < -utils::MINVAL)
            return false;

        h = sqrt(std::max(0.0, h));

        double
            t0 = -b - h,
            t1 = -b + h;

        if (t0 > t1)
            std::swap(t0, t1);

        if (t0 < 0.0)
        {
            t0 = t1;
            if (t0 < 0.0)
                return false;
        }

        if (hit != nullptr)
        {
            hit->len = t0;
            hit->origin = ray.origin + ray.dir * hit->len;
            hit->normal = (hit->origin - center) / rad;

            if (hit->normal.Dot(ray.dir) > 0.0)
                hit->normal *= -1.0;

            hit->target = (void*)this;
        }
        return true;
    }

    bool PointIntersect(const Vec3& point) const override
    {
        Vec3 p = point - center;
        return p.MagSqr() <= rad * rad;
    }
};

struct Hemisphere : Shape
{
    double rad;
    Vec3 center, normal;

    Hemisphere(double rad, Vec3 center, Vec3 normal, Material mat) :
        Shape(mat), rad(rad), center(center), normal(normal)
    {}

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        Vec3 oc = ray.origin - center;
        double b = oc.Dot(ray.dir);

        Vec3 qc = oc - ray.dir * b;
        double h = rad * rad - qc.Dot(qc);

        if (h < -utils::MINVAL)
            return false;

        h = sqrt(std::max(0.0, h));

        double
            t0 = -b - h,
            t1 = -b + h;

        if (t0 > t1)
            std::swap(t0, t1);

        if (t0 < 0.0)
        {
            t0 = t1;
            if (t0 < 0.0)
                return false;
        }

        if (normal.Dot((ray.origin + ray.dir * t0) - center) < 0.0)
        {
            if (hit != nullptr)
            {
                hit->len = t0;
                hit->origin = ray.origin + ray.dir * hit->len;
                hit->normal = (hit->origin - center) / rad;
                hit->target = (void*)this;
            }
            return true;
        }

        double a = normal.Dot(ray.dir);

        if ((a >= 0) != (normal.Dot(center - ray.origin) >= 0))
            return false;

        /*if (normal.Dot(center - ray.origin) >= 0)
            return false;*/

        t0 = (normal.Dot(center) - normal.Dot(ray.origin)) / a;

        Vec3 p = (ray.origin + ray.dir * t0) - center;

        if (p.MagSqr() > rad*rad)
            return false;

        if (hit != nullptr)
        {
            hit->len = t0;
            hit->origin = p + center;
            hit->normal = normal;
            hit->target = (void*)this;
        }
        return true;
    }

    bool PointIntersect(const Vec3& point) const override
    {
        Vec3 p = point - center;
        double a = normal.Dot(p - center);
        return (p.MagSqr() <= rad*rad) && (a <= 0.0);
    }
};

struct Tri : Shape
{
    Vec3 v0, v1, v2;

    Tri(Vec3 v0, Vec3 v1, Vec3 v2, Material mat) :
        Shape(mat), v0(v0), v1(v1), v2(v2)
    {}

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;

        // Backface-culling
        Vec3 kk = edge1.Cross(edge2);
        if (kk.Dot(ray.dir) >= 0)
            return false;

        Vec3 h = ray.dir.Cross(edge2);
        double a = edge1.Dot(h);

        if (a > -utils::MINVAL && a < utils::MINVAL)
            return false;

        Vec3 s = ray.origin - v0;
        double f = 1.0 / a;
        double u = f * s.Dot(h);

        if (u < 0 || u > 1)
            return false;

        Vec3 q = s.Cross(edge1);
        double v = f * ray.dir.Dot(q);

        if (v < 0 || u + v > 1)
            return false;

        double t = f * edge2.Dot(q);

        if (t > 0)
        {
            if (hit != nullptr)
            {
                hit->len = t;
                hit->origin = ray.origin + ray.dir * t;
                hit->normal = edge1.Cross(edge2);
                hit->normal.Normalize();
                hit->target = (void*)this;
            }
            return true;
        }
        return false;
    }

    bool PointIntersect(const Vec3& point) const override
    {
        return false;
    }
};

struct Plane : Shape
{
    Vec3 center, normal;

    Plane(Vec3 center, Vec3 normal, Material mat) :
        Shape(mat), center(center), normal(normal)
    {
        this->normal.Normalize();
    }

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        double a = normal.Dot(ray.dir);

        // Backface-culling
        /*if (a >= 0)
            return false;*/

        if ((a >= 0) != (normal.Dot(center - ray.origin) >= 0))
            return false;

        if (normal.Dot(center - ray.origin) >= 0)
            return false;

        double t = (normal.Dot(center) - normal.Dot(ray.origin)) / a;

        if (hit != nullptr)
        {
            hit->len = t;
            hit->origin = ray.origin + ray.dir * t;
            hit->normal = normal;
            hit->target = (void*)this;
        }
        return true;
    }

    bool PointIntersect(const Vec3& point) const override
    {
        return normal.Dot(center - point) < 0.0;
    }
};
