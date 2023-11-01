
#include <SFML/Graphics.hpp>
#include <iostream>
//#include <thread>
#include <cmath>
#include <omp.h>

#include "Utils.h"
#include "Primitives.h"



int main()
{
    // Build Scene
    Cam cam(
        35.0f,
        { 0.0f, 0.0f, -2.5f },
        { 0.0f, 0.0f, 1.0f }
    );
    cam.dir.Normalize();

    PointLight sun(
        {25.0f, 100.0f, 15.0f},
        1.0f,
        {255, 255, 255}
    );


    std::shared_ptr<Shape> shapePtrs[] = {
        std::make_shared<Cube>(Cube(
            { -4.0f, -2.5f, -4.0f },
            {  4.0f, -3.0f,  4.0f },
            { 0.0f, {255, 255, 255} }
        )),
        
        std::make_shared<Cube>(Cube(
            { -0.5f, -0.5f, -0.5f },
            {  0.5f,  0.5f,  0.5f },
            { 0.0f, {255, 0, 0} }
        )),

        /*std::make_shared<Cube>(Cube(
            { -0.50000f, -0.49999f, -0.49999f },
            {  0.50000f,  0.49999f,  0.49999f },
            { 0.0f, {255, 0, 0} }
        )),

        std::make_shared<Cube>(Cube(
            { -0.49999f, -0.50000f, -0.49999f },
            {  0.49999f,  0.50000f,  0.49999f },
            { 0.0f, {0, 255, 0} }
        )),

        std::make_shared<Cube>(Cube(
            { -0.49999f, -0.49999f, -0.50000f },
            {  0.49999f,  0.49999f,  0.50000f },
            { 0.0f, {0, 0, 255} }
        )),*/


        std::make_shared<Sphere>(Sphere(
            0.66f,
            { -3.0f, 0.0f, 0.0f },
            { 0.0f, {255, 255, 255} }
        )),
    };
    int shapeCount = sizeof(shapePtrs) / sizeof(std::shared_ptr<Shape>);


    // Render Scene
    const unsigned int 
        w = 320, 
        h = 180;

    sf::RenderWindow window(
        sf::VideoMode(w, h),
        "SFML Window", 
        sf::Style::Fullscreen
    );

    unsigned int
        sW = window.getSize().x,
        sH = window.getSize().y;

    float
        scaleW = (float)sW / (float)w,
        scaleH = (float)sH / (float)h;

    Vec3 camRight = cam.dir.Cross({0.0f, -1.0f, 0.0f});
    camRight.Normalize();

    Vec3 camUp = cam.dir.Cross(camRight);
    camUp.Normalize();


    sf::Clock clock;
    float lT = 0.0f, tT = 0.0f, dT = 0.0f;

    sf::Vector2i deltas;
    sf::Vector2i fixed(window.getSize());
    fixed.x /= 2;
    fixed.y /= 2;

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
                if (event.mouseWheelScroll.delta != 0.0f)
                    cam.fov += event.mouseWheelScroll.delta;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            cam.pos += cam.dir * 4.0f * dT;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            cam.pos -= cam.dir * 4.0f * dT;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            cam.pos += camRight * 4.0f * dT;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            cam.pos -= camRight * 4.0f * dT;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            cam.pos += camUp * 4.0f * dT;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
            cam.pos -= camUp * 4.0f * dT;

        deltas = fixed - sf::Mouse::getPosition();
        if (deltas != sf::Vector2i(0, 0))
            sf::Mouse::setPosition(fixed);

        if (deltas.y != 0)
        {
            float sign = (float)deltas.y * -0.001f;

            cam.dir = (
                cam.dir * cos(sign) +
                camRight.Cross(cam.dir) * sin(sign) +
                camRight * camRight.Dot(cam.dir) * (1.0f - cos(sign))
            );
        }
        if (deltas.x != 0)
        {
            float sign = (float)deltas.x * -0.001f;

            cam.dir = {
                cam.dir.x * cos(sign) + cam.dir.z * sin(sign),
                cam.dir.y,
                -cam.dir.x * sin(sign) + cam.dir.z * cos(sign)
            };
        }


        sf::Image img;
        img.create(w, h, sf::Color::Black);

        camRight = cam.dir.Cross({0.0f, -1.0f, 0.0f});
        camRight.Normalize();

        camUp = cam.dir.Cross(camRight);
        camUp.Normalize();

        float
            pHeight = tan(cam.fov * PI / 180.0f) * 2.0f,
            pWidth = pHeight / ((float)h / (float)w);

        Vec3 blLocal(-pWidth / 2.0f, -pHeight / 2.0f, 1.0f);

        for (unsigned int y = 0; y < h; y++)
        {
            float v = 1.0f - (float)y / (float)h;

            for (unsigned int x = 0; x < w; x++)
            {
                float u = (float)x / (float)w;

                Vec3 pLocal = blLocal + Vec3(pWidth * u, pHeight * v, 0.0f);
                Vec3 p = camRight * pLocal.x + camUp * pLocal.y + cam.dir * pLocal.z;
                p.Normalize();

                /*Color hitCol(
                    (unsigned int)std::max(p.x * 255.0f, 0.0f),
                    (unsigned int)std::max(p.y * 255.0f, 0.0f),
                    (unsigned int)std::max(p.z * 255.0f, 0.0f)
                );*/
                Color hitCol(10, 10, 10, 0);

                Ray ray(cam.pos, p);

                Hit hit = {};
                float len = 0.0f;
                bool hasHitSomething = false;

                /*for (int i = 0; i < cubeCount; i++)
                {
                    if (cubes[i].RayIntersect(ray, &hit))
                    {
                        if (hasHitSomething && hit.len >= len)
                            continue;

                        hitCol = cubes[i].mat.col;
                        len = hit.len;
                        hasHitSomething = true;
                    }
                }*/

                for (int i = 0; i < shapeCount; i++)
                {
                    Shape *currShape = shapePtrs[i].get();
                    if (currShape->RayIntersect(ray, &hit))
                    {
                        if (hasHitSomething && hit.len >= len)
                            continue;

                        //hitCol = currShape->mat.col;
                        hitCol = Color(
                            (uint8_t)(std::max(hit.normal.x, 0.0f) * 255),
                            (uint8_t)(std::max(hit.normal.y, 0.0f) * 255), 
                            (uint8_t)(std::max(hit.normal.z, 0.0f) * 255)
                        );

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
        sprite.setScale((float)scaleW, (float)scaleH);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    shapePtrs->reset();
    return 0;
}
