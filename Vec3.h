#pragma once

#include "Utils.h"

#include <cmath>

#include "SFML/Graphics/Shader.hpp"


struct Vec3
{
    double x, y, z;

    Vec3() :
        x(0), y(0), z(0)
    {}
    Vec3(double x, double y, double z) :
        x(x), y(y), z(z)
    {}
    Vec3(float x, float y, float z) :
        x((double)x), y((double)y), z((double)z)
    {}
    Vec3(int x, int y, int z) :
        x((double)x), y((double)y), z((double)z)
    {}

    Vec3& operator=(const Vec3& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
    bool operator==(const Vec3& v) const
    {
        return (x == v.x && y == v.y && z == v.z);
    }
    bool operator!=(const Vec3& v) const
    {
        return !(*this == v);
    }

    double& operator[](int i)
    {
        return ((double*)this)[i];
    }

    Vec3 operator+(const Vec3& v) const
    {
        return {x + v.x, y + v.y, z + v.z};
    }
    void operator+=(const Vec3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
    }
    Vec3 operator+(const double& a) const
    {
        return {x + a, y + a, z + a};
    }
    void operator+=(const double& a)
    {
        x += a;
        y += a;
        z += a;
    }

    Vec3 operator-(const Vec3& v) const
    {
        return {x - v.x, y - v.y, z - v.z};
    }
    void operator-=(const Vec3& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }

    Vec3 operator-(const double& a) const
    {
        return {x - a, y - a, z - a};
    }
    Vec3 operator-() const
    {
        return {-x, -y, -z};
    }
    void operator-=(const double& a)
    {
        x -= a;
        y -= a;
        z -= a;
    }

    Vec3 operator*(const Vec3& v) const
    {
        return {x * v.x, y * v.y, z * v.z};
    }
    Vec3 operator*(const double& a) const
    {
        return {x * a, y * a, z * a};
    }
    void operator*=(const double& a)
    {
        x *= a;
        y *= a;
        z *= a;
    }

    Vec3 operator/(const double& a) const
    {
        return {x / a, y / a, z / a};
    }
    void operator/=(const double& a)
    {
        x /= a;
        y /= a;
        z /= a;
    }


    inline double MagSqr() const
    {
        return (x*x + y*y + z*z);
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
    Vec3& NormalizeApprox()
    {
        *(this) *= utils::isqrt(MagSqr());
        return *(this);
    }

    inline double Dot(const Vec3& v) const
    {
        return (x * v.x) + (y * v.y) + (z * v.z);
    }
    inline Vec3 Cross(const Vec3& v) const
    {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }
    inline Vec3 Invert() const
    {
        return {
           1.0 / x,
           1.0 / y,
           1.0 / z
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
    inline Vec3 Lerp(const Vec3& v, const double& t) const
    {
        return {
            utils::Lerp(x, v.x, t),
            utils::Lerp(y, v.y, t),
            utils::Lerp(z, v.z, t)
        };
    }

    inline Vec3 Reflect(const Vec3& normal) const
    {
        Vec3 r = (*this) - normal * (Dot(normal) * 2.0);
        r.Normalize();
        return r;
    }
    inline Vec3 Refract(const Vec3& normal, const double& n1, const double& n2) const
    {
        double 
            n = n1 / n2,
            dot = Dot(normal),
            c = sqrt(1.0 - n*n * (1.0 - dot*dot));

        double sign = 1.0;
        if (dot < 0.0)
            sign = -1.0;

        Vec3 refraction = ((*this) * n) + (normal * sign * (c - sign * n * dot));
        refraction.Normalize();
        return refraction;
    }

    sf::Glsl::Vec3 ToShader()
    {
        return sf::Glsl::Vec3(
            (float)x, 
            (float)y, 
            (float)z
        );
    }
};

inline Vec3 RandDir()
{
    Vec3 v;

    do     v = (Vec3(utils::RandNum(), utils::RandNum(), utils::RandNum()) * 2.0) - 1.0;
    while (v.MagSqr() > 1.0);

    double m = v.Mag();

    if (m > utils::MINVAL)
        return v / m;
    else
        return RandDir();
}


/*
sf::Glsl::Vec3 Normalize(sf::Glsl::Vec3 v)
{
    Vec3 t = Vec3(v.x, v.y, v.z);
    t.Normalize();
    return sf::Glsl::Vec3(t.x, t.y, t.z);
}
sf::Glsl::Vec3 Normalize(Vec3 v)
{
    v.Normalize();
    return sf::Glsl::Vec3(v.x, v.y, v.z);
}
*/