
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

    Color sunCol(255, 235, 200);
    float sunStr = 0.25f;

    std::shared_ptr<Light> lightPtrs[] = {
        /*std::make_shared<GlobalLight>(GlobalLight(
            {0.8f, -1.0f, 0.2f}, sunStr, sunCol
        )),*/

        std::make_shared<PointLight>(PointLight(
            {4.0f, 10.0f, -1.0f},
            35.0f,
            {255, 230, 195}
        )),
    };
    int lightCount = sizeof(lightPtrs) / sizeof(std::shared_ptr<Light>);


    std::shared_ptr<Shape> shapePtrs[] = {
        std::make_shared<Plane>(Plane(
            { 0.0f, -1.5f, 0.0f },
            {  0.0f, 1.0f,  0.0f },
            { 0.0f, {165, 215, 185} }
        )),
        
        std::make_shared<Cube>(Cube(
            { -0.5f, -0.5f, -0.5f },
            {  0.5f,  0.5f,  0.5f },
            { 0.0f, {230, 100, 50} }
        )),
        
        std::make_shared<Tri>(Tri(
            { 4.50f, 2.4f, -0.6f },
            { 5.5f, 1.8f, -0.7f },
            { 4.75f, 2.2f,  0.8f },
            { 0.0f, {100, 100, 255} }
        )),

        std::make_shared<Sphere>(Sphere(
            0.75f,
            { -3.0f, 0.0f, 0.0f },
            { 0.0f, {125, 110, 95} }
        )),
    };
    int shapeCount = sizeof(shapePtrs) / sizeof(std::shared_ptr<Shape>);


    // Render Scene
    const unsigned int 
        w = 320,
        h = 180,
        dim = w * h;

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

    bool disableScanSpeed = false;
    unsigned int scanSpeed = dim / 8;
    unsigned int currPix = 0;

    sf::Image img;
    sf::Texture tex;
    sf::Sprite sprite;

    img.create(w, h, sf::Color::Black);
    sprite.setScale((float)scaleW, (float)scaleH);


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

        if (!disableScanSpeed)
        {
            if (dT > 0.3f)
                scanSpeed -= w * 32;
            else if (dT > 0.2f)
                scanSpeed -= w * 24;
            else if (dT > 0.15f)
                scanSpeed -= w * 12;
            else if (dT > 0.125f)
                scanSpeed -= w * 4;
            else if (dT > 0.11f)
                scanSpeed -= w * 2;
            else if (dT > 0.09f)
                scanSpeed -= w;

            else if (dT < 0.07f)
                scanSpeed += w / 2;
            else if (dT < 0.06f)
                scanSpeed += w;
            else if (dT < 0.04f)
                scanSpeed += w * 2;
            else if (dT < 0.02f)
                scanSpeed += w * 4;

            scanSpeed = std::max(1u, std::min(scanSpeed, dim));
        }


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
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            cam.pos -= cam.dir * 4.0f * dT;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            cam.pos += camRight * 4.0f * dT;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            cam.pos -= camRight * 4.0f * dT;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            cam.pos += camUp * 4.0f * dT;
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
            cam.pos -= camUp * 4.0f * dT;


        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab))
        {
            scanSpeed = dim;
            disableScanSpeed = true;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
        {
            scanSpeed = dim / 2;
            disableScanSpeed = false;
        }

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

        camRight = cam.dir.Cross({0.0f, -1.0f, 0.0f});
        camRight.Normalize();

        camUp = cam.dir.Cross(camRight);
        camUp.Normalize();

        float
            pHeight = tan(cam.fov * PI / 180.0f) * 2.0f,
            pWidth = pHeight / ((float)h / (float)w);

        Vec3 blLocal(-pWidth / 2.0f, -pHeight / 2.0f, 1.0f);

        for (int i = 0; i < scanSpeed; i++)
        {
            unsigned int
                x = currPix % w,
                y = currPix / w;

            float 
                v = 1.0f - (float)y / (float)h,
                u = (float)x / (float)w;

            Vec3 pLocal = blLocal + Vec3(pWidth * u, pHeight * v, 0.0f);
            Vec3 p = camRight * pLocal.x + camUp * pLocal.y + cam.dir * pLocal.z;
            p.Normalize();

            Ray ray(cam.pos, p);

            Hit bestHit = {};
            Hit hit = {};
            float len = 0.0f;
            bool hasHitSomething = false;

            Vec3 hitCol = Vec3((float)sunCol.r, (float)sunCol.g, (float)sunCol.b) / 255.0f;
            for (int i = 0; i < shapeCount; i++)
            {
                Shape* currShape = shapePtrs[i].get();
                if (currShape->RayIntersect(ray, &hit))
                {
                    if (hasHitSomething && hit.len >= len)
                        continue;

                    bestHit = hit;
                    len = hit.len;
                    hasHitSomething = true;
                }
            }

            Vec3 lighting;
            if (hasHitSomething)
            {
                lighting = Vec3(0.015f, 0.025f, 0.045f);

                hitCol = Vec3(
                    (float)((Shape*)bestHit.target)->mat.col.r,
                    (float)((Shape*)bestHit.target)->mat.col.g,
                    (float)((Shape*)bestHit.target)->mat.col.b
                ) / 255.0f;

                Shape* hitShape = (Shape*)bestHit.target;
                Ray lightRay(bestHit.pos, hit.normal);

                for (int i = 0; i < lightCount; i++)
                {
                    Light* currLight = lightPtrs[i].get();
                    Vec3 toLight = currLight->GetRelativePos(bestHit.pos);
                    toLight.Normalize();

                    if (bestHit.normal.Dot(toLight) <= 0.0f)
                        continue;

                    float distSqr = currLight->GetDistSqr(bestHit.pos);

                    Ray lightRay(bestHit.pos, toLight);
                    Hit lightHit = {};

                    bool isBlocked = false;
                    for (int j = 0; j < shapeCount; j++)
                    {
                        Shape* currShape = shapePtrs[j].get();
                        if (currShape == hitShape)
                            continue;

                        if (currShape->RayIntersect(lightRay, &lightHit))
                            if (bestHit.len * bestHit.len < distSqr)
                                isBlocked = true;

                        if (isBlocked)
                            break;
                    }

                    if (isBlocked)
                        continue;

                    float lightStr = currLight->GetIntensity(lightRay, bestHit.normal);

                    lighting += Vec3(
                        (float)currLight->col.r,
                        (float)currLight->col.g,
                        (float)currLight->col.b
                    ) / 255.0f * lightStr;
                }
            }
            else
                lighting = Vec3(sunStr);

            hitCol = Vec3(
                std::max(0.0f, std::min(hitCol.x, 1.0f)),
                std::max(0.0f, std::min(hitCol.y, 1.0f)),
                std::max(0.0f, std::min(hitCol.z, 1.0f))
            );

            lighting = Vec3(
                std::max(0.0f, std::min(lighting.x, 1.0f)),
                std::max(0.0f, std::min(lighting.y, 1.0f)),
                std::max(0.0f, std::min(lighting.z, 1.0f))
            );
            
            img.setPixel(x, y, {
                (uint8_t)(hitCol.x * lighting.x * 255.0f), 
                (uint8_t)(hitCol.y * lighting.y * 255.0f), 
                (uint8_t)(hitCol.z * lighting.z * 255.0f)
            });

            ++currPix %= dim;
        }


        for (int y = 2; y < 8; y++)
        {
            for (int x = 2; x < 8; x++)
            {
                if (dT > 0.09f)
                    img.setPixel(x, y, {255, 0, 0});
                else if (dT < 0.07f)
                    img.setPixel(x, y, {0, 0, 255});
                else
                    img.setPixel(x, y, {0, 255, 0});
            }
        }

        tex.loadFromImage(img);
        sprite.setTexture(tex);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    shapePtrs->reset();
    return 0;
}
