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
            hit->len = tmin;
            hit->origin = ray.origin + (ray.dir * tmin);
            hit->target = (void*)this;

            if (abs(hit->origin.x - min.x) < MINVAL)
                hit->normal = Vec3(-1, 0, 0);
            else if (abs(hit->origin.x - max.x) < MINVAL)
                hit->normal = Vec3(1, 0, 0);
            else if (abs(hit->origin.y - min.y) < MINVAL)
                hit->normal = Vec3(0, -1, 0);
            else if (abs(hit->origin.y - max.y) < MINVAL)
                hit->normal = Vec3(0, 1, 0);
            else if (abs(hit->origin.z - min.z) < MINVAL)
                hit->normal = Vec3(0, 0, -1);
            else if (abs(hit->origin.z - max.z) < MINVAL)
                hit->normal = Vec3(0, 0, 1);
        }

        return result;
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

        if (abs(abs(axes[0].Dot(axes[1].Cross(axes[2]))) - 1.0) > MINVAL)
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

            if (abs(f) > MINVAL)
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
};

struct Sphere : Shape
{
    double rad;
    Vec3 origin;

    Sphere(double rad, Vec3 origin, Material mat) :
        Shape(mat), rad(rad), origin(origin)
    {}

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        Vec3 oc = ray.origin - origin;
        double b = oc.Dot(ray.dir);

        Vec3 qc = oc - ray.dir * b;
        double h = rad * rad - qc.Dot(qc);

        if (h < -MINVAL)
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
            hit->normal = (hit->origin - origin) / rad;
            hit->target = (void*)this;
        }
        return true;
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

        Vec3 kk = edge1.Cross(edge2);
        if (kk.Dot(ray.dir) >= 0)
            return false;

        Vec3 h = ray.dir.Cross(edge2);
        double a = edge1.Dot(h);

        if (a > -MINVAL && a < MINVAL)
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
};

struct Plane : Shape
{
    Vec3 origin, normal;

    Plane(Vec3 origin, Vec3 normal, Material mat) :
        Shape(mat), origin(origin), normal(normal)
    {
        this->normal.Normalize();
    }

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        double a = normal.Dot(ray.dir);

        if (a >= 0)
            return false;

        if (normal.Dot(origin - ray.origin) >= 0)
            return false;

        double
            b = normal.Dot(ray.dir),
            d = normal.Dot(origin),
            t = (d - normal.Dot(ray.origin)) / b;

        if (hit != nullptr)
        {
            hit->len = t;
            hit->origin = ray.origin + ray.dir * t;
            hit->normal = normal;
            hit->target = (void*)this;
        }
        return true;
    }
};
