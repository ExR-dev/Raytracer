#pragma once

#include <iostream>
#include <cmath>
#include <random>

namespace utils
{
    constexpr double PI = 3.14159265358979323;
    constexpr double MINVAL = 0.000000001;
    constexpr double MAXVAL = 1000000000.0;


    inline double isqrt(double number)
    {
        double y = number;
        double x2 = y * 0.5;
        std::int64_t i = *(std::int64_t*)&y;
        i = 0x5fe6eb50c7b537a9 - (i >> 1);
        y = *(double*)&i;
        y = y * (1.5 - (x2 * y * y));
        return y;
    }

    inline double Lerp(double p0, double p1, double t)
    {
        return (1.0 - t) * p0 + t * p1;
    }

    std::default_random_engine generator((unsigned int)time(0));
    std::mt19937 gen(generator());
    unsigned int VeryRand(int s, int e)
    {
        std::uniform_int_distribution<std::mt19937::result_type> distr(s, e);
        return distr(gen);

        /*std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<std::mt19937::result_type> distr(s, e);
        return distr(gen);*/
    }

    double RandNum()
    {
        return (double)VeryRand(0, RAND_MAX) / (double)(RAND_MAX + 1);
    }


    /*
    struct Vec4
    {
        double x, y, z, w;

        Vec4() :
            x(0), y(0), z(0), w(0)
        {}
        Vec4(double a) :
            x(a), y(a), z(a), w(a)
        {}
        Vec4(double x, double y, double z, double w) :
            x(x), y(y), z(z), w(w)
        {}

        operator Vec3() const
        {
            return Vec3(x, y, z);
        }

        Vec4& operator=(const Vec4 v)
        {
            x = v.x;
            y = v.y;
            z = v.z;
            w = v.w;
            return *this;
        }
        bool operator==(const Vec4 v) const
        {
            return (x == v.x && y == v.y && z == v.z && w == v.w);
        }
        bool operator!=(const Vec4 v) const
        {
            return !(*this == v);
        }
        Vec4 operator+(const Vec4 v) const
        {
            return {x + v.x, y + v.y, z + v.z, w + v.w};
        }
        void operator+=(const Vec4 v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            w += v.w;
        }
        Vec4 operator+(double a) const
        {
            return {x + a, y + a, z + a, w + a};
        }
        void operator+=(double a)
        {
            x += a;
            y += a;
            z += a;
            w += a;
        }
        Vec4 operator-(const Vec4 v) const
        {
            return {x - v.x, y - v.y, z - v.z, w - v.w};
        }
        void operator-=(const Vec4 v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            w -= v.w;
        }
        Vec4 operator-(double a) const
        {
            return {x - a, y - a, z - a, w - a};
        }
        void operator-=(double a)
        {
            x -= a;
            y -= a;
            z -= a;
            w -= a;
        }
        Vec4 operator*(const Vec4 v) const
        {
            return {x * v.x, y * v.y, z * v.z, w * v.w};
        }
        Vec4 operator*(double a) const
        {
            return {x * a, y * a, z * a, w * a};
        }
        void operator*=(double a)
        {
            x *= a;
            y *= a;
            z *= a;
            w *= a;
        }
        Vec4 operator/(double a) const
        {
            return {x / a, y / a, z / a, w / a};
        }
        void operator/=(double a)
        {
            x /= a;
            y /= a;
            z /= a;
            w /= a;
        }
    };

    struct Matrix4x4
    {
        double mat[4][4];


        Matrix4x4()
        {
            mat[0][0] = mat[0][1] = mat[0][2] = mat[0][3] =
            mat[1][0] = mat[1][1] = mat[1][2] = mat[1][3] =
            mat[2][0] = mat[2][1] = mat[2][2] = mat[2][3] =
            mat[3][0] = mat[3][1] = mat[3][2] = mat[3][3] = 0.0;
        }
        Matrix4x4(Vec3 v0, Vec3 v1, Vec3 v2)
        {
            mat[0][0] = v0.x; mat[0][1] = v1.x; mat[0][2] = v2.x; mat[0][3] = 0.0;
            mat[1][0] = v0.y; mat[1][1] = v1.y; mat[1][2] = v2.y; mat[1][3] = 0.0;
            mat[2][0] = v0.z; mat[2][1] = v1.z; mat[2][2] = v2.z; mat[2][3] = 0.0;
            mat[3][0] = 0.0;  mat[3][1] = 0.0;  mat[3][2] = 0.0;  mat[3][3] = 1.0;
        }
        Matrix4x4(Vec4 v0, Vec4 v1, Vec4 v2, Vec4 v3)
        {
            mat[0][0] = v0.x; mat[0][1] = v1.x; mat[0][2] = v2.x; mat[0][3] = v3.x;
            mat[1][0] = v0.y; mat[1][1] = v1.y; mat[1][2] = v2.y; mat[1][3] = v3.y;
            mat[2][0] = v0.z; mat[2][1] = v1.z; mat[2][2] = v2.z; mat[2][3] = v3.z;
            mat[3][0] = v0.w; mat[3][1] = v1.w; mat[3][2] = v2.w; mat[3][3] = v3.w;
        }
        Matrix4x4(double m00, double m11, double m22, double m33)
        {
            mat[0][0] = m00; mat[0][1] = 0.0; mat[0][2] = 0.0; mat[0][3] = 0.0;
            mat[1][0] = 0.0; mat[1][1] = m11; mat[1][2] = 0.0; mat[1][3] = 0.0;
            mat[2][0] = 0.0; mat[2][1] = 0.0; mat[2][2] = m22; mat[2][3] = 0.0;
            mat[3][0] = 0.0; mat[3][1] = 0.0; mat[3][2] = 0.0; mat[3][3] = m33;
        }
        Matrix4x4(
            double m00, double m01, double m02, double m03,
            double m10, double m11, double m12, double m13,
            double m20, double m21, double m22, double m23,
            double m30, double m31, double m32, double m33)
        {
            mat[0][0] = m00; mat[0][1] = m01; mat[0][2] = m02; mat[0][3] = m03;
            mat[1][0] = m10; mat[1][1] = m11; mat[1][2] = m12; mat[1][3] = m13;
            mat[2][0] = m20; mat[2][1] = m21; mat[2][2] = m22; mat[2][3] = m23;
            mat[3][0] = m30; mat[3][1] = m31; mat[3][2] = m32; mat[3][3] = m33;
        }
        Matrix4x4(double m[4][4])
        {
            mat[0][0] = m[0][0]; mat[0][1] = m[0][1]; mat[0][2] = m[0][2]; mat[0][3] = m[0][3];
            mat[1][0] = m[1][0]; mat[1][1] = m[1][1]; mat[1][2] = m[1][2]; mat[1][3] = m[1][3];
            mat[2][0] = m[2][0]; mat[2][1] = m[2][1]; mat[2][2] = m[2][2]; mat[2][3] = m[2][3];
            mat[3][0] = m[3][0]; mat[3][1] = m[3][1]; mat[3][2] = m[3][2]; mat[3][3] = m[3][3];
        }
        Matrix4x4(const Matrix4x4& m)
        {
            (*this) = m;
        }


        Matrix4x4& operator=(const Matrix4x4& m)
        {
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                    mat[y][x] = m.mat[y][x];
            return *this;
        }
        bool operator==(const Matrix4x4& m) const
        {
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                    if (mat[y][x] != m.mat[y][x])
                        return false;
            return true;
        }
        bool operator!=(const Matrix4x4& m) const
        {
            return !(*this == m);
        }

        Matrix4x4& operator*=(const Matrix4x4& m)
        {
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                {
                    double sum = 0.0;
                    for (int z = 0; z < 4; z++)
                        sum += mat[y][z] * m.mat[z][x];
                    mat[y][x] = sum;
                }
            return *this;
        }
        Matrix4x4 operator*(const Matrix4x4& m) const
        {
            Matrix4x4 M(*this);
            M *= m;
            return M;
        }

        Matrix4x4& operator*=(double d)
        {
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                    mat[y][x] *= d;
            return *this;
        }
        Matrix4x4 operator*(double d) const
        {
            Matrix4x4 M(*this);
            M *= d;
            return M;
        }

        Matrix4x4& operator+=(const Matrix4x4& m)
        {
            for (int y = 0; y < 4; y++)
                for (int x = 0; x < 4; x++)
                    mat[y][x] += m.mat[y][x];
            return *this;
        }
        Matrix4x4 operator+(const Matrix4x4& m) const
        {
            Matrix4x4 M(*this);
            M += m;
            return M;
        }


        Vec4 GetRow(int y) const
        {
            return Vec4(mat[y][0], mat[y][1], mat[y][2], mat[y][3]);
        }
        Vec4 GetCol(int x) const
        {
            return Vec4(mat[0][x], mat[1][x], mat[2][x], mat[3][x]);
        }

        double Determinant(Matrix4x4 m, int n = 4)
        {
            double d = 0;

            if (n == 1)
                return m.mat[0][0];

            Matrix4x4 temp = Matrix4x4(0);
            int sign = 1;

            for (int f = 0; f < n; f++)
            {
                getCofactor(m, temp, 0, f, n);
                d += sign * m.mat[0][f] * Determinant(temp, n - 1);

                sign = -sign;
            }

            return d;
        }

        Vec3 TransformPoint(const Vec3& p) const
        {
            double d[4], c[4] = {p.x, p.y, p.z, 1.0};

            for (int y = 0; y < 4; y++)
            {
                double sum = 0.0;
                for (int x = 0; x < 4; x++)
                    sum += (mat[y][x] * c[x]);
                d[y] = sum;
            }

            if (abs(d[3]) > 0.00001)
                return Vec3(d[0] / d[3], d[1] / d[3], d[2] / d[3]);

            return Vec3(d[0], d[1], d[2]);
        }
        Vec3 TransformVector(const Vec3& v) const
        {
            double d[3], c[3] = {v.x, v.y, v.z};

            for (int i = 0; i < 3; i++)
            {
                double sum = 0.0;
                for (int j = 0; j < 3; j++)
                    sum += (mat[i][j] * c[j]);
                d[i] = sum;
            }
            return Vec3(d[0], d[1], d[2]);
        }

        Vec3 GetEulerAngles(const Vec3& v) const
        {
            Vec3 result = Vec3();

            double sy = mat[0][2];
            if (sy < 1.0)
            {
                if (sy > -1.0)
                {
                    std::cout << "1\n";
                    result.x = -atan2(mat[1][2], mat[2][2]);
                    result.y = asin(sy);
                    result.z = -atan2(mat[0][1], mat[0][0]);
                }
                else
                {
                    std::cout << "2\n";
                    result.x = 0.0;
                    result.y = -1.570796327;
                    result.z = atan2(mat[1][0], mat[1][1]);
                }
            }
            else
            {
                std::cout << "3\n";
                result.x = 0.0;
                result.y = 1.570796327;
                result.z = atan2(mat[1][0], mat[1][1]);
            }

            return result;
        }
        Matrix4x4 Rotate(Vec3 angles) const
        {
            double
                cX = cos(angles.x),
                sX = sin(angles.x),
                cY = cos(angles.y),
                sY = sin(angles.y),
                cZ = cos(angles.z),
                sZ = sin(angles.z);

            return Matrix4x4(
                cZ*cY,  cZ*sY*sX - sZ*cX,   cZ*sY*cX + sZ*sX,   0.0,
                sZ*cY,  sZ*sY*sX + cZ*cY,   sZ*sY*cX - cZ*sX,   0.0,
                -sY,    cY*sX,              cY*cX,              0.0,
                0.0,    0.0,                0.0,                1.0
            );
        }

        Matrix4x4 XRot(double angle) const
        {
            double
                radians = angle * PI / 180.0,
                c = cos(radians),
                s = sin(radians);

            return Matrix4x4(
                1.0,    0.0,    0.0,    0.0,
                0.0,    c,      -s,     0.0,
                0.0,    s,      c,      0.0,
                0.0,    0.0,    0.0,    1.0
            );
        }
        Matrix4x4 YRot(double angle) const
        {
            double
                radians = angle * PI / 180.0,
                c = cos(radians),
                s = sin(radians);

            return Matrix4x4(
                c,      0.0,    s,      0.0,
                0.0,    1.0,    0.0,    0.0,
                -s,     0.0,    c,      0.0,
                0.0,    0.0,    0.0,    1.0
            );
        }
        Matrix4x4 ZRot(double angle) const
        {
            double
                radians = angle * PI / 180.0,
                c = cos(radians),
                s = sin(radians);

            return Matrix4x4(
                c,      -s,     0.0,    0.0,
                s,      c,      0.0,    0.0,
                0.0,    0.0,    1.0,    0.0,
                0.0,    0.0,    0.0,    1.0
            );
        }

    };

    const Matrix4x4 identity = Matrix4x4(1.0, 1.0, 1.0, 1.0);
    */

}
