#pragma once

#include "Utils.h"

#include <cmath>
#include <random>


struct Vec3
{
    double x, y, z;

    Vec3() :
        x(0), y(0), z(0)
    {}
    Vec3(float x, float y, float z) :
        x((double)x), y((double)y), z((double)z)
    {}
    Vec3(double x, double y, double z) :
        x(x), y(y), z(z)
    {}
    Vec3(int x, int y, int z) :
        x((double)x), y((double)y), z((double)z)
    {}

    Vec3& operator=(const Vec3 v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
    bool operator==(const Vec3 v) const
    {
        return (x == v.x && y == v.y && z == v.z);
    }
    bool operator!=(const Vec3 v) const
    {
        return !(*this == v);
    }

    Vec3 operator+(const Vec3 v) const
    {
        return {x + v.x, y + v.y, z + v.z};
    }
    void operator+=(const Vec3 v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
    }
    Vec3 operator+(double a) const
    {
        return {x + a, y + a, z + a};
    }
    void operator+=(double a)
    {
        x += a;
        y += a;
        z += a;
    }

    Vec3 operator-(const Vec3 v) const
    {
        return {x - v.x, y - v.y, z - v.z};
    }
    void operator-=(const Vec3 v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }

    Vec3 operator-(double a) const
    {
        return {x - a, y - a, z - a};
    }
    void operator-=(double a)
    {
        x -= a;
        y -= a;
        z -= a;
    }

    Vec3 operator*(const Vec3 v) const
    {
        return {x * v.x, y * v.y, z * v.z};
    }
    Vec3 operator*(double a) const
    {
        return {x * a, y * a, z * a};
    }
    void operator*=(double a)
    {
        x *= a;
        y *= a;
        z *= a;
    }

    Vec3 operator/(double a) const
    {
        return {x / a, y / a, z / a};
    }
    void operator/=(double a)
    {
        x /= a;
        y /= a;
        z /= a;
    }


    inline double MagSqr() const
    {
        return (x * x + y * y + z * z);
    }
    inline double Mag() const
    {
        return sqrt(MagSqr());
    }
    Vec3& Normalize()
    {
        *(this) *= 1.0 / Mag();
        return *(this);
    }

    inline double Dot(const Vec3 v) const
    {
        return (x * v.x) + (y * v.y) + (z * v.z);
    }
    inline Vec3 Cross(const Vec3 v) const
    {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }
    inline Vec3 Abs() const
    {
        return {
           std::abs(x),
           std::abs(y),
           std::abs(z)
        };
    }
    inline Vec3 VLerp(const Vec3 v, double t) const
    {
        return {
            Lerp(x, v.x, t),
            Lerp(y, v.y, t),
            Lerp(z, v.z, t)
        };
    }

    inline Vec3 Reflect(const Vec3 normal) const
    {
        Vec3 r = (*this) - normal * (Dot(normal) * 2.0);

        return r;
    }

    inline Vec3 Refract(const Vec3 normal, double n1, double n2) const
    {
        double n = n1 / n2;
        double dot = Dot(normal);
        double c = sqrt(1 - n*n * (1 - dot*dot));

        double sign = 1;
        if (dot < 0.0)
            sign = -1;

        Vec3 refraction = ((*this) * n) + (normal * sign * (c - sign * n * dot));
        refraction.Normalize();
        return refraction;
    }


};

inline Vec3 RandDir()
{
    Vec3 v;

    do      v = Vec3(RandNum(), RandNum(), RandNum());
    while (v.MagSqr() > 1.0);

    v *= 2.0;
    v -= Vec3(1, 1, 1);

    double m = v.Mag();

    if (m > MINVAL)
        return v / m;
    else
        return RandDir();
}