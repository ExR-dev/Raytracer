
#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <cmath>
//#include <omp.h>

constexpr auto PI = 3.1415926535897932384;


double Lerp(double p0, double p1, double t)
{
    return (1.0 - t) * p0 + t * p1;
}


struct Vec3
{
    double x, y, z;

    Vec3()
        : x(0.0), y(0.0), z(0.0)
    {

    }

    Vec3(double x, double y, double z)
        : x(x), y(y), z(z)
    {

    }


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


    Vec3 operator+(const Vec3& v) const
    {
        return {x + v.x, y + v.y, z + v.z};
    }

    void operator+=(const Vec3 &v)
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



    double Magnitude()
    {
        return sqrt(x*x + y*y + z*z);
    }

    Vec3& Normalize()
    {
        double invMag = 1.0 / Magnitude();
        *(this) *= invMag;
        return *(this);
    }


    double Dot(Vec3 v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    Vec3 Cross(Vec3 v) const
    {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }

    Vec3 VLerp(Vec3 v, double t) const
    {
        return {
            Lerp(x, v.x, t),
            Lerp(y, v.y, t),
            Lerp(z, v.z, t)
        };
    }
};

struct Ray
{
    Vec3 pos, dir;

    Ray(Vec3 pos, Vec3 dir) 
        : pos(pos), dir(dir)
    {

    }

    Ray Inverse() const
    {
        return {pos, {1.0 / dir.x, 1.0 / dir.y, 1.0 / dir.z}};
    }

};

struct Hit
{
    double len;
    Vec3 pos, normal;

    Hit() 
        : len(-1.0), pos({}), normal({})
    {

    }

    Hit(double len, Vec3 pos, Vec3 normal) :
        len(len), pos(pos), normal(normal)
    {

    }
};


struct Color
{
    unsigned char r, g, b, a;

    Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255) 
        : r(r), g(g), b(b), a(a)
    {

    }
};

struct Material
{
    double emission;
    Color col;

    Material(double emission, Color col)
        : emission(emission), col(col)
    {

    }
};


struct Cam
{
    double fov;
    Vec3 pos, dir;

    Cam(double fov, Vec3 pos, Vec3 dir)
        : fov(fov), pos(pos), dir(dir)
    {
    
    }
};


struct PointLight
{
    Vec3 pos;
    Material mat;

    PointLight(Vec3 pos, Material mat) 
        : pos(pos), mat(mat)
    {

    }
};


struct Cube
{
    Vec3 min, max;
    Material mat;

    Cube(Vec3 min, Vec3 max, Material mat) 
        : min(min), max(max), mat(mat)
    {

    }

    bool RayCubeIntersect(const Ray& r, Hit& h)
    {
        Ray inv_r = r.Inverse();

        double tx1 = (min.x - inv_r.pos.x) * inv_r.dir.x;
        double tx2 = (max.x - inv_r.pos.x) * inv_r.dir.x;
        
        double tmin = std::min(tx1, tx2);
        double tmax = std::max(tx1, tx2);

        double ty1 = (min.y - inv_r.pos.y) * inv_r.dir.y;
        double ty2 = (max.y - inv_r.pos.y) * inv_r.dir.y;

        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        double tz1 = (min.z - inv_r.pos.z) * inv_r.dir.z;
        double tz2 = (max.z - inv_r.pos.z) * inv_r.dir.z;

        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));

        bool result = tmax >= std::max(0.0, tmin) && tmin < 10000000.0;

        if (&h != nullptr)
        {
            h.len = tmin;
            h.pos = r.pos + (r.dir * tmin);
            h.normal = {0.0, 0.0, 0.0};
        }

        return result;
    }
};

struct Sphere
{
    double rad;
    Vec3 pos;
    Material mat;

    Sphere(double rad, Vec3 pos, Material mat)
        : rad(rad), pos(pos), mat(mat)
    {

    }
};

struct Plane
{
    Vec3 pos, normal;
    Material mat;

    Plane(Vec3 pos, Vec3 normal, Material mat) 
        : pos(pos), normal(normal), mat(mat)
    {

    }
};



int main()
{
    // Build Scene
    Cam cam(
        35.0,
        { 0.0, 0.0, -2.5 },
        { 0.0, 0.0, 1.0 }
    );
    cam.dir.Normalize();

    PointLight sun(
        {25.0, 100.0, 15.0},
        {1.0, {255, 255, 255}}
    );

    Cube cubes[] = {
        {
            { -4.0, -2.5, -4.0 },
            { 4.0, -3.0, 4.0 },
            { 0.0, {255, 255, 255} }
        },

        {
            { -0.50000000, -0.49999999, -0.49999999 },
            {  0.50000000,  0.49999999,  0.49999999 },
            { 0.0, {255, 0, 0} }
        },

        {
            { -0.49999999, -0.50000000, -0.49999999 },
            {  0.49999999,  0.50000000,  0.49999999 },
            { 0.0, {0, 255, 0} }
        },

        {
            { -0.49999999, -0.49999999, -0.50000000 },
            {  0.49999999,  0.49999999,  0.50000000 },
            { 0.0, {0, 0, 255} }
        },
    };
    int cubeCount = sizeof(cubes) / sizeof(Cube);


    // Render Scene
    const unsigned int 
        w = 320, 
        h = 200;

    sf::RenderWindow window(
        sf::VideoMode(w, h),
        "SFML Window", 
        sf::Style::Fullscreen
    );

    unsigned int
        sW = window.getSize().x,
        sH = window.getSize().y;

    double
        scaleW = (double)sW / (double)w,
        scaleH = (double)sH / (double)h;

    Vec3 camRight = cam.dir.Cross({0.0, -1.0, 0.0});
    camRight.Normalize();

    Vec3 camUp = cam.dir.Cross(camRight);
    camUp.Normalize();


    sf::Clock clock;
    double lT = 0.0, tT = 0.0, dT = 0.0;

    sf::Vector2i deltas;
    sf::Vector2i fixed(window.getSize());
    fixed.x /= 2;
    fixed.y /= 2;

    std::thread threads[h];

    while (window.isOpen())
    {
        lT = tT;
        tT = clock.getElapsedTime().asSeconds();
        dT = tT - lT;


        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseWheelScrolled)
                if (event.mouseWheelScroll.delta != 0)
                    cam.fov += event.mouseWheelScroll.delta;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            cam.pos += cam.dir * 4.0 * dT;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            cam.pos -= cam.dir * 4.0 * dT;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            cam.pos += camRight * 4.0 * dT;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            cam.pos -= camRight * 4.0 * dT;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            cam.pos += camUp * 4.0 * dT;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
            cam.pos -= camUp * 4.0 * dT;

        deltas = fixed - sf::Mouse::getPosition();
        if (deltas != sf::Vector2i(0, 0))
            sf::Mouse::setPosition(fixed);

        if (deltas.y != 0) // X
        {
            double sign = (double)deltas.y * -0.001;

            cam.dir = (
                cam.dir * cos(sign) +
                camRight.Cross(cam.dir) * sin(sign) +
                camRight * camRight.Dot(cam.dir) * (1.0 - cos(sign))
            );
        }
        if (deltas.x != 0) // Y
        {
            double sign = (double)deltas.x * -0.001;

            cam.dir = {
                cam.dir.x * cos(sign) + cam.dir.z * sin(sign),
                cam.dir.y,
                -cam.dir.x * sin(sign) + cam.dir.z * cos(sign)
            };
        }


        sf::Image img;
        img.create(w, h, sf::Color::Black);

        camRight = cam.dir.Cross({0.0, -1.0, 0.0});
        camRight.Normalize();

        camUp = cam.dir.Cross(camRight);
        camUp.Normalize();

        double
            pHeight = tan(cam.fov * PI / 180.0) * 2.0,
            pWidth = pHeight / ((double)h / (double)w);

        Vec3 blLocal(-pWidth / 2.0, -pHeight / 2.0, 1.0);


        for (unsigned int y = 0; y < h; y++)
        {
            double v = 1.0 - (double)y / (double)h;

            for (unsigned int x = 0; x < w; x++)
            {
                double u = (double)x / (double)w;

                Vec3 pLocal = blLocal + Vec3(pWidth * u, pHeight * v, 0.0);
                Vec3 p = camRight * pLocal.x + camUp * pLocal.y + cam.dir * pLocal.z;
                p.Normalize();

                Color hitCol(
                    (unsigned int)std::max(p.x * 255.0, 0.0),
                    (unsigned int)std::max(p.y * 255.0, 0.0),
                    (unsigned int)std::max(p.z * 255.0, 0.0)
                );

                Ray r(cam.pos, p);

                Hit hit = {};
                double len = 0;
                bool hasHitSomething = false;

                for (int i = 0; i < cubeCount; i++)
                {
                    if (cubes[i].RayCubeIntersect(r, hit))
                    {
                        if (hasHitSomething && hit.len >= len)
                            continue;

                        hitCol = cubes[i].mat.col;
                        len = hit.len;
                        hasHitSomething = true;
                    }
                }

                img.setPixel(
                    x, y,
                    {hitCol.r, hitCol.g, hitCol.b}
                );
            }
        }

        sf::Texture tex;
        tex.loadFromImage(img);

        sf::Sprite sprite;
        sprite.setTexture(tex);
        sprite.setScale((double)scaleW, (double)scaleH);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}
