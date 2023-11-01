#pragma once

#include <iostream>
#include <cmath>


constexpr float PI = 3.141592653f;


inline float InverseSqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5f;

    x2 = number * 0.5f;
    y = number;
    i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y));
    //y = y * (threehalfs - (x2 * y * y));

    return y;
}

float Lerp(float p0, float p1, float t)
{
    return (1.0f - t) * p0 + t * p1;
}


struct Vec3
{
    float x, y, z;


    Vec3() :
        x(0.0f), y(0.0f), z(0.0f)
    {}

    Vec3(double x, double y, double z) :
        x((float)x), y((float)y), z((float)z)
    {}

    Vec3(float x, float y, float z) :
        x(x), y(y), z(z)
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

    Vec3 operator+(float a) const
    {
        return {x + a, y + a, z + a};
    }
    void operator+=(float a)
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

    Vec3 operator-(float a) const
    {
        return {x - a, y - a, z - a};
    }
    void operator-=(float a)
    {
        x -= a;
        y -= a;
        z -= a;
    }

    Vec3 operator*(const Vec3 v) const
    {
        return {x * v.x, y * v.y, z * v.z};
    }
    Vec3 operator*(float a) const
    {
        return {x * a, y * a, z * a};
    }
    void operator*=(float a)
    {
        x *= a;
        y *= a;
        z *= a;
    }

    Vec3 operator/(float a) const
    {
        return {x / a, y / a, z / a};
    }
    void operator/=(float a)
    {
        x /= a;
        y /= a;
        z /= a;
    }


    inline float MagSqr() const
    {
        return (x * x + y * y + z * z);
    }

    inline float Mag() const
    {
        return sqrt(MagSqr());
    }


    Vec3& Normalize()
    {
        *(this) *= InverseSqrt(MagSqr());
        return *(this);
    }

    inline float Dot(const Vec3 v) const
    {
        return x * v.x + y * v.y + z * v.z;
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

    inline Vec3 VLerp(const Vec3 v, float t) const
    {
        return {
            Lerp(x, v.x, t),
            Lerp(y, v.y, t),
            Lerp(z, v.z, t)
        };
    }
};

struct Color
{
    uint8_t r, g, b, a;

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) :
        r(r), g(g), b(b), a(a)
    {}
};

struct Material
{
    float emission;
    Color col;

    Material(float emission, Color col) :
        emission(emission), col(col)
    {}
};


struct Ray
{
    Vec3 pos, dir;

    Ray(Vec3 pos, Vec3 dir) :
        pos(pos), dir(dir)
    {}

    inline Vec3 InverseDir() const
    {
        return {
            1.0f / dir.x, 
            1.0f / dir.y, 
            1.0f / dir.z
        };
    }
};

struct Hit
{
    float len;
    Vec3 pos, normal;

    Hit() :
        len(-1.0f), pos({}), normal({})
    {}

    Hit(float len, Vec3 pos, Vec3 normal) :
        len(len), pos(pos), normal(normal)
    {}
};