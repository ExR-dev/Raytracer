
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
        70.0f,
        { 0.0, 1.0, -3.0 },
        { 0.0, 0.0, 1.0 }
    );
    cam.dir.Normalize();

    //Color sunCol(1.0, 0.92, 0.78);
    Color sunCol(1.0, 0.9, 0.7);
    double sunStr = 0.98;

    std::shared_ptr<Light> lightPtrs[] = {
        std::make_shared<GlobalLight>(GlobalLight(
            {0.5, -1.0, 0.28}, sunStr, sunCol
        )),

        std::make_shared<PointLight>(PointLight(
            {3.0, 4.5, -2.0},
            35.0,
            {0.0, 0.7, 1.0}
        )),
    };
    int lightCount = sizeof(lightPtrs) / sizeof(std::shared_ptr<Light>);


    std::shared_ptr<Shape> shapePtrs[] = {
        std::make_shared<Plane>(Plane(
            {  0.0, 0.0,  0.0 },
            {  0.0,  1.0,  0.0 },
            { 0.0f, {0.8, 0.8, 0.8} }
        )),

        
        std::make_shared<Cube>(Cube(
            { -0.25,  -1.0, -1.0 },
            {  0.25,  4.5,  5.0 },
            { 0.0f, {0.4, 0.25, 0.55} }
        )),


        std::make_shared<Sphere>(Sphere(
            1.0,
            { -2.5, 1.0, 0.0 },
            { 0.0f, {0.45, 0.8, 0.2} }
        )),

        /*std::make_shared<Sphere>(Sphere(
            0.8,
            { -4.0, 0.1, 0.0 },
            { 0.0f, {0.12, 0.2, 0.4} }
        )),
        

        std::make_shared<Tri>(Tri(
            {  4.50,  2.4, -0.6 },
            {  5.5,   1.8, -0.7 },
            {  4.75,  2.2,  0.8 },
            { 0.0f, {0.35, 0.35, 1.0} }
        )),
        
        std::make_shared<Tri>(Tri(
            { 4.5, 2.4, -0.6 },
            { 5.0, 0.8, -0.7 },
            { 4.75, 2.2,  0.8 },
            { 0.0f, {1.0, 1.0, 0.35} }
        )),
        
        std::make_shared<Tri>(Tri(
            { 4.50, 2.4, -0.6 },
            { 5.5, 1.8, -0.7 },
            { 5.0, 0.8, -0.7 },
            { 0.0f, {1.0, 0.35, 1.0} }
        )),*/
    };
    int shapeCount = sizeof(shapePtrs) / sizeof(std::shared_ptr<Shape>);


    // Render Scene
    const unsigned int 
        w = 640,
        h = 360,
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

    bool giveControl = true;
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
            else if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseWheelScrolled)
            {
                if (event.mouseButton.button == 1)
                    giveControl = !giveControl;
                if (event.mouseWheelScroll.delta != 0.0f)
                    cam.fov -= event.mouseWheelScroll.delta;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        if (giveControl)
        {
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
        }
        

        camRight = cam.dir.Cross({0.0f, -1.0f, 0.0f});
        camRight.Normalize();

        camUp = cam.dir.Cross(camRight);
        camUp.Normalize();

        float
            pHeight = tan((float)(cam.fov / 2.0f) * PI / 180.0f) * 2.0f,
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
            Hit 
                bestHit = {},
                hit = {};

            float len = 0.0f;
            bool hasHitSomething = false;

            Vec3 hitCol = Vec3(sunCol.r, sunCol.g, sunCol.b);
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
                    ((Shape*)bestHit.target)->mat.col.r,
                    ((Shape*)bestHit.target)->mat.col.g,
                    ((Shape*)bestHit.target)->mat.col.b
                );

                Shape* hitShape = (Shape*)bestHit.target;
                Ray lightRay(bestHit.pos, hit.normal);

                for (int i = 0; i < lightCount; i++)
                {
                    Light* currLight = lightPtrs[i].get();
                    Vec3 toLight = currLight->GetRelativePos(bestHit.pos);
                    toLight.Normalize();

                    if (bestHit.normal.Dot(toLight) <= 0)
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
                            if ((lightHit.len * lightHit.len) < distSqr)
                                isBlocked = true;

                        if (isBlocked)
                            break;
                    }

                    if (isBlocked)
                        continue;

                    float lightStr = currLight->GetIntensity(lightRay, bestHit.normal);

                    lighting += Vec3(
                        currLight->col.r,
                        currLight->col.g,
                        currLight->col.b
                    ) * lightStr;
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
                (uint8_t)(hitCol.x * lighting.x * 255.0), 
                (uint8_t)(hitCol.y * lighting.y * 255.0), 
                (uint8_t)(hitCol.z * lighting.z * 255.0)
            });

            ++currPix %= dim;
        }


        /*Vec3 spCPoint = Vec3(9.5, 9.5, 0.0);
        float axisLength = 8.0f;

        double
            camPitch = asin(-cam.dir.y),
            camYaw = atan2(cam.dir.x, cam.dir.z);

        double D2R = PI / 180.0;
        double yScale = 1.0 / tan(D2R * cam.fov / 2.0);
        double xScale = yScale / ((double)w / (double)w);
        Matrix4x4 camProjMat = Matrix4x4(
            xScale, 0.0,    0.0,    0.0,
            0.0,    yScale, 0.0,    0.0,
            0.0,    0.0,    -1.0,   -1.0,
            0.0,    0.0,    0.0,    0.0
        );

        Matrix4x4 rotationMat = identity.Rotate(Vec3(camPitch, camYaw, 0.0));
        Matrix4x4 rotationMat = identity.Rotate(Vec3(0.0, -camPitch, -camYaw));

        Matrix4x4 camBase = Matrix4x4(camRight, camUp, cam.dir);
        Matrix4x4 camBaseRot = camBase.Rotate(Vec3(0.0, -camPitch, -camYaw));

        Vec3 eAngles = camBase.GetEulerAngles(cam.dir);
        eAngles *= -1;
        std::cout << "(x: " << eAngles.x << ", y: " << eAngles.y << ", z: " << eAngles.z << ")\n";

        Matrix4x4 eRotMat = identity.Rotate(eAngles);


        for (int i = 0; i < 3; i++)
        {
            Vec3 axisNormal;
            sf::Color axisCol;
            if (i == 0)
            {
                axisCol = {255, 0, 0};
                //axisNormal = camBase.GetCol(0);
                //axisNormal = camBaseRot.GetCol(0);
                //axisNormal = rotationMat.GetCol(0);
                //axisNormal = eRotMat.GetCol(0);
            }
            else if (i == 1)
            {
                axisCol = {0, 255, 0};
                //axisNormal = camBase.GetCol(1);
                //axisNormal = camBaseRot.GetCol(1);
                //axisNormal = rotationMat.GetCol(1);
                //axisNormal = eRotMat.GetCol(1);
            }
            else if (i == 2)
            {
                axisCol = {0, 0, 255};
                //axisNormal = camBase.GetCol(2);
                //axisNormal = camBaseRot.GetCol(2);
                //axisNormal = rotationMat.GetCol(2);
                //axisNormal = eRotMat.GetCol(2);
            }
            axisNormal.Normalize();

            if (axisNormal.z < 0)
                axisCol = {
                    (uint8_t)((double)axisCol.r / (1.0 - axisNormal.z)),
                    (uint8_t)((double)axisCol.g / (1.0 - axisNormal.z)),
                    (uint8_t)((double)axisCol.b / (1.0 - axisNormal.z))
                };

            Vec3 transition = spCPoint;
            int ltX = 0, ltY = 0;

            axisNormal *= axisLength;
            for (double i = 0.01; i <= 1.0; i += 0.03)
            {
                transition = spCPoint.VLerp(spCPoint + axisNormal, i);
                
                int
                    tX = (int)transition.x,
                    tY = (int)transition.y;

                if ( (tX != ltX || tY != ltY) && (tX >= 0 && tY >= 0) )
                    img.setPixel(tX, tY, axisCol);

                ltX = tX,
                ltY = tY;
            }
        }*/

        /*for (int y = 1; y < 4; y++)
        {
            for (int x = 1; x < 4; x++)
            {
                if (dT > 0.09f)
                    img.setPixel(x, y, {255, 0, 0});
                else if (dT < 0.07f)
                    img.setPixel(x, y, {0, 0, 255});
                else
                    img.setPixel(x, y, {0, 255, 0});
            }
        }*/

        tex.loadFromImage(img);
        sprite.setTexture(tex);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    shapePtrs->reset();
    return 0;
}
