#pragma once

#include "Utils.h"

#include <iostream>



struct Ray
{
    Vec3 pos, dir;

    Ray(Vec3 pos, Vec3 dir) :
        pos(pos), dir(dir)
    {
        this->dir.Normalize();
    }

    inline Vec3 InverseDir() const
    {
        return {
            1.0 / dir.x,
            1.0 / dir.y,
            1.0 / dir.z
        };
    }
};

struct Hit
{
    float len;
    Vec3 pos, normal;
    void* target;

    Hit() :
        len(0.0f), pos({}), normal({}), target(nullptr)
    {}

    Hit(float len, Vec3 pos, Vec3 normal, void* target) :
        len(len), pos(pos), normal(normal), target(target)
    {
        this->normal.Normalize();
    }
};


struct Cam
{
    float fov;
    Vec3 pos, dir;

    Cam(float fov, Vec3 pos, Vec3 dir) :
        fov(fov), pos(pos), dir(dir)
    {
        this->dir.Normalize();
    }
};


struct Light
{
    float intensity;
    Color col;

    Light(float intensity, Color col) :
        intensity(intensity), col(col)
    {}

    virtual Vec3 GetRelativePos(const Vec3& objPos) const
    {
        return Vec3();
    }

    virtual float GetDistSqr(const Vec3& objPos) const
    {
        return std::numeric_limits<float>::max();
    }

    virtual float GetIntensity(const Ray& lightray, Vec3 surfaceNormal) const
    {
        return intensity * surfaceNormal.Dot(lightray.dir);
    }
};

struct GlobalLight : Light
{
    Vec3 normal;

    GlobalLight(Vec3 normal, float intensity, Color col) :
        Light(intensity, col), normal(normal)
    {
        this->normal.Normalize();
    }

    Vec3 GetRelativePos(const Vec3& objPos) const override
    {
        return normal * (-1.0f);
    }

    float GetDistSqr(const Vec3& objPos) const override
    {
        return std::numeric_limits<float>::max();
    }

    float GetIntensity(const Ray& lightray, Vec3 surfaceNormal) const override
    {
        return intensity * surfaceNormal.Dot(lightray.dir);
    }
};

struct PointLight : Light
{
    Vec3 pos;

    PointLight(Vec3 pos, float intensity, Color col) :
        Light(intensity, col), pos(pos)
    {}

    Vec3 GetRelativePos(const Vec3& objPos) const override
    {
        return pos - objPos;
    }

    float GetDistSqr(const Vec3& objPos) const override
    {
        return (pos - objPos).MagSqr();
    }

    float GetIntensity(const Ray& lightray, Vec3 surfaceNormal) const override
    {
        return (intensity / (pos - lightray.pos).MagSqr()) * surfaceNormal.Dot(lightray.dir);
    }
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


    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        Vec3 inv_dir = ray.InverseDir();

        double tx1 = (min.x - ray.pos.x) * inv_dir.x;
        double tx2 = (max.x - ray.pos.x) * inv_dir.x;

        double tmin = std::min(tx1, tx2);
        double tmax = std::max(tx1, tx2);

        double ty1 = (min.y - ray.pos.y) * inv_dir.y;
        double ty2 = (max.y - ray.pos.y) * inv_dir.y;

        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        double tz1 = (min.z - ray.pos.z) * inv_dir.z;
        double tz2 = (max.z - ray.pos.z) * inv_dir.z;

        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));

        bool result = tmax >= std::max(0.0, tmin) && tmin < std::numeric_limits<float>::max();

        if (hit != nullptr)
        {
            hit->len = tmin;
            hit->pos = ray.pos + (ray.dir * tmin);
            hit->target = (void*)this;

            if (abs(hit->pos.x - min.x) < MINVAL)
                hit->normal = Vec3(-1, 0, 0);
            else if (abs(hit->pos.x - max.x) < MINVAL)
                hit->normal = Vec3(1, 0, 0);
            else if (abs(hit->pos.y - min.y) < MINVAL)
                hit->normal = Vec3(0, -1, 0);
            else if (abs(hit->pos.y - max.y) < MINVAL)
                hit->normal = Vec3(0, 1, 0);
            else if (abs(hit->pos.z - min.z) < MINVAL)
                hit->normal = Vec3(0, 0, -1);
            else if (abs(hit->pos.z - max.z) < MINVAL)
                hit->normal = Vec3(0, 0, 1);
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

        if (h < 0)
            return false;

        h = 1.0f / InverseSqrt(h);

        float
            t0 = -b - h,
            t1 = -b + h;

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
            hit->pos = ray.pos + ray.dir * hit->len;
            hit->normal = hit->pos - pos;
            hit->normal.Normalize();
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
        Vec3 edge1, edge2, h, s, q;
        float a, f, u, v;

        edge1 = v1 - v0;
        edge2 = v2 - v0;
        h = ray.dir.Cross(edge2);
        a = edge1.Dot(h);

        if (a > -MINVAL && a < MINVAL)
            return false;

        f = 1.0 / a;
        s = ray.pos - v0;
        u = f * s.Dot(h);

        if (u < 0 || u > 1)
            return false;

        q = s.Cross(edge1);
        v = f * ray.dir.Dot(q);

        if (v < 0 || u + v > 1)
            return false;

        float t = f * edge2.Dot(q);

        if (t > 0)
        {
            if (hit != nullptr)
            {
                hit->len = t;
                hit->pos = ray.pos + ray.dir * t;
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
    Vec3 pos, normal;

    Plane(Vec3 pos, Vec3 normal, Material mat) :
        Shape(mat), pos(pos), normal(normal)
    {
        this->normal.Normalize();
    }

    bool RayIntersect(const Ray& ray, Hit* hit) const override
    {
        float a = normal.Dot(ray.dir);

        if (a >= 0)
            return false;

        if (normal.Dot(pos - ray.pos) >= 0)
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
            hit->target = (void*)this;
        }
        return true;
    }
};
