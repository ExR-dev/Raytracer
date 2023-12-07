#pragma once

#include "Utils.h"
#include "Vec3.h"

#include <cmath>


constexpr double riVacuum = 1.0;
constexpr double riAir = 1.000293;
constexpr double riWater = 1.333;
constexpr double riGlass = 1.52;
constexpr double riDiamond = 2.417;


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
    Color(const Vec3& v) :
        r((double)v.x), g((double)v.y), b((double)v.z)
    {}
    Color(const sf::Color& c) :
        r(((double)c.r) / 255.0), g(((double)c.g) / 255.0), b(((double)c.b) / 255.0)
    {}


    operator Vec3() const
    {
        return Vec3(r, g, b);
    }

    Color& operator=(const Color& c)
    {
        r = c.r;
        g = c.g;
        b = c.b;
        return *this;
    }
    bool operator==(const Color& c) const
    {
        return (
            r == c.r &&
            g == c.g &&
            b == c.b
            );
    }
    bool operator!=(const Color& c) const
    {
        return !(*this == c);
    }
    Color operator+(const Color& c) const
    {
        return {
            r + c.r,
            g + c.g,
            b + c.b
        };
    }
    void operator+=(const Color& c)
    {
        r += c.r;
        g += c.g;
        b += c.b;
    }
    Color operator+(const double& a) const
    {
        return {
            r + a,
            g + a,
            b + a
        };
    }
    void operator+=(const double& a)
    {
        r += a;
        g += a;
        b += a;
    }
    Color operator-(const Color& c) const
    {
        return {
            r - c.r,
            g - c.g,
            b - c.b
        };
    }
    void operator-=(const Color& c)
    {
        r -= c.r;
        g -= c.g;
        b -= c.b;
    }
    Color operator-(const double& a) const
    {
        return {
            r - a,
            g - a,
            b - a
        };
    }
    void operator-=(const double& a)
    {
        r -= a;
        g -= a;
        b -= a;
    }
    Color operator*(const Color& c) const
    {
        return {
            r * c.r,
            g * c.g,
            b * c.b
        };
    }
    Color operator*(const double& a) const
    {
        return {
            r * a,
            g * a,
            b * a
        };
    }
    void operator*=(const double& a)
    {
        r *= a;
        g *= a;
        b *= a;
    }
    Color operator/(const double& a) const
    {
        return {
            r / a,
            g / a,
            b / a
        };
    }
    void operator/=(const double& a)
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
    inline Color Lerp(const Color& c, const double& t) const
    {
        return {
            utils::Lerp(r, c.r, t),
            utils::Lerp(g, c.g, t),
            utils::Lerp(b, c.b, t)
        };
    }
    inline Color ApplyGamma(const double& str) const
    {
        return {
            pow(r, 1.0 / str),
            pow(g, 1.0 / str),
            pow(b, 1.0 / str)
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


    inline Color ACESFilm() const
    {
        return {
            std::max(0.0, std::min((r*(2.51*r + 0.03)) / (r*(2.43*r + 0.59) + 0.14), 1.0)),
            std::max(0.0, std::min((g*(2.51*g + 0.03)) / (g*(2.43*g + 0.59) + 0.14), 1.0)),
            std::max(0.0, std::min((b*(2.51*b + 0.03)) / (b*(2.43*b + 0.59) + 0.14), 1.0))
        };
    }
};
