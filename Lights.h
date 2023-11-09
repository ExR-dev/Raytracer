#pragma once

#include "Utils.h"
#include "Vec3.h"
#include "Phys.h"
#include "Graphics.h"



struct Light
{
    double intensity;
    Color col;

    Light(double intensity, const Color& col) :
        intensity(intensity), col(col)
    {}

    virtual Vec3 GetRelativePos(const Vec3& objPos) const
    {
        return Vec3();
    }

    virtual double GetDistSqr(const Vec3& objPos) const
    {
        return std::numeric_limits<double>::max();
    }

    virtual double GetIntensity(const Ray& lightray, const Vec3& surfaceNormal) const
    {
        return intensity * surfaceNormal.Dot(lightray.dir) * 2.0;
    }
};


struct GlobalLight : Light
{
    Vec3 normal;

    GlobalLight(const Vec3& normal, double intensity, const Color& col) :
        Light(intensity, col), normal(normal)
    {
        this->normal.Normalize();
    }

    Vec3 GetRelativePos(const Vec3& objPos) const override
    {
        return normal * (-1.0);
    }

    double GetDistSqr(const Vec3& objPos) const override
    {
        return std::numeric_limits<double>::max();
    }

    double GetIntensity(const Ray& lightray, const Vec3& surfaceNormal) const override
    {
        return intensity * surfaceNormal.Dot(lightray.dir) * 2.0;
    }
};

struct PointLight : Light
{
    Vec3 origin;
    double falloff;

    PointLight(const Vec3& origin, double falloff, double intensity, const Color& col) :
        Light(intensity, col), origin(origin), falloff(falloff)
    {}

    Vec3 GetRelativePos(const Vec3& objPos) const override
    {
        return origin - objPos;
    }

    double GetDistSqr(const Vec3& objPos) const override
    {
        return (origin - objPos).MagSqr();
    }

    double GetIntensity(const Ray& lightray, const Vec3& surfaceNormal) const override
    {
        return (intensity / (origin - lightray.origin).MagSqr()) * surfaceNormal.Dot(lightray.dir) * 2.0;
    }
};
