#pragma once

#include "Utils.h"

#include <iostream>


struct Cam
{
    float fov;
    Vec3 pos, dir;

    Cam(float fov, Vec3 pos, Vec3 dir) :
        fov(fov), pos(pos), dir(dir)
    {}
};


struct Light
{
    Vec3 pos;
    float intensity;
    Color col;

    Light(Vec3 pos, float intensity, Color col) :
        pos(pos), intensity(intensity), col(col)
    {}
};

struct PointLight : Light
{
    PointLight(Vec3 pos, float intensity, Color col) :
        Light(pos, intensity, col)
    {}
};


struct Shape
{
    Material mat;

    Shape(Material mat) :
        mat(mat)
    {}

    virtual bool RayIntersect(const Ray& ray, Hit* hit) const
    {
        return false;
    }
};

struct Cube : Shape
{
    Vec3 min, max;

    Cube(Vec3 min, Vec3 max, Material mat) :
        Shape(mat), min(min), max(max)
    {}


    /*bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        Vec3 
            m = ray.Invert(), // can precompute if traversing a set of aligned boxes
            n = m * ray.pos,   // can precompute if traversing a set of aligned boxes
            k = (max - min) * m.Abs(),
            t1 = (n * -1.0f) - k,
            t2 = k - n;

        float tN = std::max(std::max(t1.x, t1.y), t1.z);
        float tF = std::min(std::min(t2.x, t2.y), t2.z);

        if (tN > tF || tF < 0.0f) 
            return false;

        Vec3 normal = (tN > 0.0f) ?
            Vec3(std::nextafter(tN, t1.x), std::nextafter(tN, t1.y), std::nextafter(tN, t1.z)) :
            Vec3(std::nextafter(t2.x, tF), std::nextafter(t2.y, tF), std::nextafter(t2.z, tF));

        normal = normal * Vec3(
            (ray.dir.x > 0.0f) ? -1.0f : 1.0f, 
            (ray.dir.y > 0.0f) ? -1.0f : 1.0f, 
            (ray.dir.z > 0.0f) ? -1.0f : 1.0f
        );

        if (hit != nullptr)
        {
            hit->len = tN;
            hit->pos = ray.pos + ray.dir * hit->len;
            hit->normal = normal;
            hit->normal.Normalize();
        }

        return true;
    }*/
        
    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        Vec3 inv_dir = ray.InverseDir();

        float tx1 = (min.x - ray.pos.x) * inv_dir.x;
        float tx2 = (max.x - ray.pos.x) * inv_dir.x;

        float tmin = std::min(tx1, tx2);
        float tmax = std::max(tx1, tx2);

        float ty1 = (min.y - ray.pos.y) * inv_dir.y;
        float ty2 = (max.y - ray.pos.y) * inv_dir.y;

        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        float tz1 = (min.z - ray.pos.z) * inv_dir.z;
        float tz2 = (max.z - ray.pos.z) * inv_dir.z;

        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));

        bool result = tmax >= std::max(0.0f, tmin) && tmin < 10000000.0f;

        if (hit != nullptr)
        {
            hit->len = tmin;
            hit->pos = ray.pos + (ray.dir * tmin);
            hit->normal = ray.dir * (-1.0f);
        }

        return result;
    }
};

struct Sphere : Shape
{
    float rad;
    Vec3 pos;

    Sphere(float rad, Vec3 pos, Material mat) :
        Shape(mat), rad(rad), pos(pos)
    {}


    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        Vec3 oc = ray.pos - pos;
        float b = oc.Dot(ray.dir);

        Vec3 qc = oc - ray.dir * b;
        float h = rad * rad - qc.Dot(qc);

        if (h < 0.0f) 
            return false;

        //h = sqrt(h);
        h = 1.0f / InverseSqrt(h);

        float 
            t0 = -b - h,
            t1 = -b + h;

        if (t0 > t1)
            std::swap(t0, t1);

        if (t0 < 0.0f)
        {
            t0 = t1;
            if (t0 < 0.0f)
                return false;
        }

        if (hit != nullptr)
        {
            hit->len = t0;
            hit->pos = ray.pos + ray.dir * hit->len;
            hit->normal = hit->pos - pos;
            hit->normal.Normalize();
        }

        return true;
        //return vec2(-b - h, -b + h);
    }

    /*bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        Vec3 L = pos - ray.pos;
        float tca = L.Dot(ray.dir);

        if (tca < 0)
            return false;

        float
            radSqr = rad * rad,
            dSqr = L.Dot(L) - tca * tca;

        if (dSqr > radSqr)
            return false;

        float thc = sqrt(radSqr - dSqr);

        float 
            t0 = tca - thc, 
            t1 = tca + thc;

        if (t0 > t1)
            std::swap(t0, t1);

        if (t0 < 0)
        {
            t0 = t1;
            if (t0 < 0)
                return false;
        }

        if (hit != nullptr)
        {
            hit->len = t0;
            hit->pos = ray.pos + ray.dir * t0;
            hit->normal = hit->pos - pos;
            hit->normal.Normalize();
        }
        return true;
    }*/
};

struct Tri : Shape
{
    Vec3 v0, v1, v2;

    Tri(Vec3 v0, Vec3 v1, Vec3 v2, Material mat) :
        Shape(mat), v0(v0), v1(v1), v2(v2)
    {}

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        const float EPSILON = 0.0000001f;
        Vec3 edge1, edge2, h, s, q;
        float a, f, u, v;
        edge1 = v1 - v0;
        edge2 = v2 - v0;
        h = ray.dir.Cross(edge2);
        a = edge1.Dot(h);

        if (a > -EPSILON && a < EPSILON)
            return false;

        f = 1.0f / a;
        s = ray.pos - v0;
        u = f * s.Dot(h);

        if (u < 0.0f || u > 1.0f)
            return false;

        q = s.Cross(edge1);
        v = f * ray.dir.Dot(q);

        if (v < 0.0f || u + v > 1.0f)
            return false;

        float t = f * edge2.Dot(q);

        if (t > EPSILON)
        {
            if (hit != nullptr)
            {
                hit->len = t;
                hit->pos = ray.pos + ray.dir * t;
                hit->normal = edge1.Cross(edge2);
                hit->normal.Normalize();
            }
            return true;
        }
        
        return false;
    }
};

struct Plane : Shape
{
    Vec3 pos, normal;

    Plane(Vec3 pos, Vec3 normal, Material mat) :
        Shape(mat), pos(pos), normal(normal)
    {}

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        float a = normal.Dot(ray.dir);

        if (a >= 0.0f)
            return false;

        if (normal.Dot(pos - ray.pos) >= 0.0f)
            return false;

        float 
            b = normal.Dot(ray.dir),
            d = normal.Dot(pos),
            t = (d - normal.Dot(ray.pos)) / b;

        if (hit != nullptr)
        {
            hit->len = t;
            hit->pos = ray.pos + ray.dir * t;
            hit->normal = normal;
        }

        return true;
    }
};

