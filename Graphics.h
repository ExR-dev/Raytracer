#pragma once

#include "Utils.h"
#include "Vec3.h"

#include <cmath>



struct Color
{
    double r, g, b;

    Color() :
        r(0), g(0), b(0)
    {}
    Color(float r, float g, float b) :
        r((double)r), g((double)g), b((double)b)
    {}
    Color(double r, double g, double b) :
        r((double)r), g((double)g), b((double)b)
    {}
    Color(int r, int g, int b) :
        r((double)r), g((double)g), b((double)b)
    {}


    operator Vec3() const
    {
        return Vec3(r, g, b);
    }

    Color& operator=(const Color c)
    {
        r = c.r;
        g = c.g;
        b = c.b;
        return *this;
    }
    bool operator==(const Color c) const
    {
        return (
            r == c.r &&
            g == c.g &&
            b == c.b
            );
    }
    bool operator!=(const Color c) const
    {
        return !(*this == c);
    }
    Color operator+(const Color c) const
    {
        return {
            r + c.r,
            g + c.g,
            b + c.b
        };
    }
    void operator+=(const Color c)
    {
        r += c.r;
        g += c.g;
        b += c.b;
    }
    Color operator+(double a) const
    {
        return {
            r + a,
            g + a,
            b + a
        };
    }
    void operator+=(double a)
    {
        r += a;
        g += a;
        b += a;
    }
    Color operator-(const Color c) const
    {
        return {
            r - c.r,
            g - c.g,
            b - c.b
        };
    }
    void operator-=(const Color c)
    {
        r -= c.r;
        g -= c.g;
        b -= c.b;
    }
    Color operator-(double a) const
    {
        return {
            r - a,
            g - a,
            b - a
        };
    }
    void operator-=(double a)
    {
        r -= a;
        g -= a;
        b -= a;
    }
    Color operator*(const Color c) const
    {
        return {
            r * c.r,
            g * c.g,
            b * c.b
        };
    }
    Color operator*(double a) const
    {
        return {
            r * a,
            g * a,
            b * a
        };
    }
    void operator*=(double a)
    {
        r *= a;
        g *= a;
        b *= a;
    }
    Color operator/(double a) const
    {
        return {
            r / a,
            g / a,
            b / a
        };
    }
    void operator/=(double a)
    {
        r /= a;
        g /= a;
        b /= a;
    }

    Color& Max()
    {
        r = std::max(0.0, r);
        g = std::max(0.0, g);
        b = std::max(0.0, b);
        return *this;
    }
    Color& Min()
    {
        r = std::min(1.0, r);
        g = std::min(1.0, g);
        b = std::min(1.0, b);
        return *this;
    }
    Color& Clamp()
    {
        r = std::max(0.0, std::min(r, 1.0));
        g = std::max(0.0, std::min(g, 1.0));
        b = std::max(0.0, std::min(b, 1.0));
        return *this;
    }
    inline Color Clamped() const
    {
        return {
            std::max(0.0, std::min(r, 1.0)),
            std::max(0.0, std::min(g, 1.0)),
            std::max(0.0, std::min(b, 1.0))
        };
    }

    inline Color GetMax(const Color& c) const
    {
        return {
            std::max(r, c.r),
            std::max(g, c.g),
            std::max(b, c.b)
        };
    }
};


constexpr double riVacuum = 1.0;
constexpr double riAir = 1.000293;
constexpr double riWater = 1.333;
constexpr double riGlass = 1.52;

struct Material
{

    Color col;
    double opacity;
    double refractIndex;
    double reflectivity;

    Color emissionCol;
    double emissionStr;


    Material() :
        col(Color()), opacity(1), refractIndex(1), reflectivity(0),
        emissionCol(Color()), emissionStr(0)
    {}

    Material(Color col = Color(), double opacity = 1.0, double refractIndex = 1.0, double reflectivity = 0.0, Color emissionCol = Color(), double emissionStr = 0.0) :
        col(col), opacity(opacity), refractIndex(refractIndex), reflectivity(reflectivity),
        emissionCol(emissionCol), emissionStr(emissionStr)
    {}
};
