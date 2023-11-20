#pragma once

#include "Vec3.h"

#include <cmath>
#include "Utils.h"


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


    Vec3 ToHSV() const
    {
        Vec3 out;
        double fCMax = std::max(std::max(r, g), b);
        double fCMin = std::min(std::min(r, g), b);
        double fDelta = fCMax - fCMin;

        if (fDelta > 0.0)
        {
            if (fCMax == r)
            {
                out.x = 60.0 * (fmod(((g - b) / fDelta), 6.0));
            }
            else if (fCMax == g)
            {
                out.x = 60.0 * (((b - r) / fDelta) + 2.0);
            }
            else if (fCMax == b)
            {
                out.x = 60.0 * (((r - g) / fDelta) + 4.0);
            }

            if (fCMax > 0.0)
            {
                out.y = fDelta / fCMax;
            }
            else
            {
                out.y = 0.0;
            }

            out.z = fCMax;
        }
        else
        {
            out.x = 0.0;
            out.y = 0.0;
            out.z = fCMax;
        }

        if (out.x < 0.0)
            out.x = 360.0 + out.x;
        return out;
    }

    Color& FromHSV(Vec3 in)
    {
        double fC = in.z * in.y;
        double fHPrime = fmod(in.x / 60.0, 6.0);
        double fX = fC * (1.0 - fabs(fmod(fHPrime, 2.0) - 1.0));
        double fM = in.z - fC;

        if (0.0 <= fHPrime && fHPrime < 1.0)
        {
            r = fC;
            g = fX;
            b = 0.0;
        }
        else if (1 <= fHPrime && fHPrime < 2.0)
        {
            r = fX;
            g = fC;
            b = 0.0;
        }
        else if (2.0 <= fHPrime && fHPrime < 3.0)
        {
            r = 0.0;
            g = fC;
            b = fX;
        }
        else if (3.0 <= fHPrime && fHPrime < 4.0)
        {
            r = 0.0;
            g = fX;
            b = fC;
        }
        else if (4.0 <= fHPrime && fHPrime < 5.0)
        {
            r = fX;
            g = 0.0;
            b = fC;
        }
        else if (5.0 <= fHPrime && fHPrime < 6.0)
        {
            r = fC;
            g = 0.0;
            b = fX;
        }
        else
        {
            r = 0.0;
            g = 0.0;
            b = 0.0;
        }

        r += fM;
        g += fM;
        b += fM;

        return *this;
    }
};


constexpr double riVacuum = 1.0;
constexpr double riAir = 1.000293;
constexpr double riWater = 1.333;
constexpr double riGlass = 1.52;
constexpr double riDiamond = 2.417;

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
