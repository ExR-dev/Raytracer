#pragma once

#include "Utils.h"
#include "Vec3.h"



struct Ray
{
    Vec3 origin, dir;

    Ray(const Vec3& origin, const Vec3& dir) :
        origin(origin), dir(dir)
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
    double len;
    Vec3 origin, normal;
    void* target;

    Hit() :
        len(0.0), origin({}), normal({}), target(nullptr)
    {}

    Hit(double len, const Vec3& origin, const Vec3& normal, void* target) :
        len(len), origin(origin), normal(normal), target(target)
    {
        this->normal.Normalize();
    }
};
