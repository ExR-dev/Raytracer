
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <omp.h>

#include "Utils.h"
#include "Primitives.h"

struct Scene
{
    bool disableLighting;

    int maxBounces;
    int raySplits;

    Color sunCol;
    float sunStr;

    std::shared_ptr<Light>* lightPtrs;
    int lightCount;

    std::shared_ptr<Shape>* shapePtrs;
    int shapeCount;

    Scene(int maxBounces, int raySplits, Color sunCol, float sunStr) :
        maxBounces(maxBounces), raySplits(raySplits), sunCol(sunCol), sunStr(sunStr),
        lightPtrs(nullptr), lightCount(0), 
        shapePtrs(nullptr), shapeCount(0)
    {
        disableLighting = true;
    }
};



Vec3 CastRayInScene(Scene& scene, Ray ray, Hit& hit, int bounce)
{
    Hit bestHit = {};

    float len = 0.0f;
    bool hasHitSomething = false;

    Vec3 hitCol = Vec3(scene.sunCol.r, scene.sunCol.g, scene.sunCol.b);
    for (int j = 0; j < scene.shapeCount; j++)
    {
        Shape* currShape = scene.shapePtrs[j].get();

        if (hit.target != nullptr)
            if (((Shape*)hit.target) == currShape)
                continue;

        if (currShape->RayIntersect(ray, &hit))
        {
            if (hasHitSomething && hit.len >= len)
                continue;

            bestHit = hit;
            len = hit.len;
            hasHitSomething = true;
        }
    }

    Vec3 lighting = Vec3(0);
    if (hasHitSomething)
    {
        hitCol = Vec3(
            ((Shape*)bestHit.target)->mat.col.r,
            ((Shape*)bestHit.target)->mat.col.g,
            ((Shape*)bestHit.target)->mat.col.b
        );

        if (scene.disableLighting)
        {
            lighting = Vec3(1.0, 1.0, 1.0);
            goto endHit;
        }

        lighting = hitCol * ((Shape*)bestHit.target)->mat.emission;

        Shape* hitShape = (Shape*)bestHit.target;
        Ray lightRay(bestHit.pos, hit.normal);

        for (int j = 0; j < scene.lightCount; j++)
        {
            Light* currLight = scene.lightPtrs[j].get();
            Vec3 toLight = currLight->GetRelativePos(bestHit.pos);
            toLight.Normalize();

            if (bestHit.normal.Dot(toLight) <= 0)
                continue;

            float distSqr = currLight->GetDistSqr(bestHit.pos);

            Ray lightRay(bestHit.pos, toLight);
            Hit lightHit = {};

            bool isBlocked = false;
            for (int k = 0; k < scene.shapeCount; k++)
            {
                Shape* currShape = scene.shapePtrs[k].get();
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

        if (bounce < scene.maxBounces)
        {
            for (int i = 0; i < scene.raySplits; i++)
            {
                Vec3 randDir = RandDir();

                if (bestHit.normal.Dot(randDir) <= 0)
                    randDir *= -1.0f;

                Ray randRay(bestHit.pos, randDir);
                Hit randHit = {};

                Vec3 outCol = CastRayInScene(scene, randRay, randHit, bounce + 1);

                if (randHit.target != nullptr)
                {
                    lighting += (outCol * 1.00f / (scene.raySplits)) * (1.0f / ((bestHit.pos - randHit.pos).MagSqr() + 1.0f)) * bestHit.normal.Dot(randDir)
                        * randHit.normal.Dot((randDir * -1.0f)); // This bit is experimental.

                    //lighting += (outCol * 0.1f / (scene.raySplits * scene.maxBounces)) * bestHit.normal.Dot(randDir) * randHit.normal.Dot((randDir * -1.0f)); // This bit is experimental.
                }
            }
        }
    }
    else
        lighting = (Vec3)scene.sunCol * scene.sunStr;

endHit:
    hit = bestHit;
    return hitCol = Vec3(
        std::max(0.0f, std::min(hitCol.x * lighting.x, 1.0f)),
        std::max(0.0f, std::min(hitCol.y * lighting.y, 1.0f)),
        std::max(0.0f, std::min(hitCol.z * lighting.z, 1.0f))
    );
}



int main()
{
    // Build Scene
    Cam cam(
        70.0f,
        { 0.0, 1.0, -3.0 },
        { 0.0, 0.0, 1.0 }
    );

    //Color sunCol(0.01, 0.01, 0.01);
    Color sunCol(0, 0, 0);
    float sunStr = 0.0;

    std::shared_ptr<Light> lightPtrs[] = {
        /*std::make_shared<GlobalLight>(GlobalLight(
            Vec3(0), 0, Color()
        )),*/


        /*std::make_shared<GlobalLight>(GlobalLight(
            {0.5, -1.0, 0.28}, sunStr, sunCol
        )),*/

        std::make_shared<PointLight>(PointLight(
            {3.3, 1.5, -4.5},
            0.0f, 5.0f, {1.0, 0.0, 0.0}
        )),
        std::make_shared<PointLight>(PointLight(
            {-1.5, 4.0, 4.0},
            0.0f, 5.0f, {0.0, 1.0, 0.0}
        )),
        std::make_shared<PointLight>(PointLight(
            {-3.0, 3.0, -2.0},
            0.0f, 5.0f, {0.0, 0.0, 1.0}
        )),
    };
    int lightCount = sizeof(lightPtrs) / sizeof(std::shared_ptr<Light>);

    std::shared_ptr<Shape> shapePtrs[] = {
        std::make_shared<Plane>(Plane(
            { 0.0, 0.0, 0.0 },
            { 0.0, 1.0, 0.0 },
            { 0.0f, {1, 1, 1} }
        )),
        std::make_shared<Plane>(Plane(
            { 0.0, 6.0, 0.0 },
            { 0.0, -1.0, 0.0 },
            { 0.0f, {.25, .25, .25} }
        )),

        std::make_shared<Cube>(Cube(
            { -0.25,  0.0,  0.5 },
            { 1.75,  3.0,  3.5 },
            { 0.0f, {1, 0, 0} }
        )),

        std::make_shared<Sphere>(Sphere(
            1.75,
            { -3.0, 0.75, -0.25 },
            { 0.0f, {0, 1, 0} }
        )),

        std::make_shared<Tri>(Tri(
            {  2.9, 0.0, -4.9 },
            {  3.0, 2.7, -3.7 },
            {  2.0, 0.0, -2.0 },
            { 0.0f, {0, 1, 1} }
        )),
        std::make_shared<Tri>(Tri(
            {  3.0, 2.7, -3.7 },
            { -1.2, 0.0, -4.7 },
            {  2.0, 0.0, -2.0 },
            { 0.0f, {1, 1, 0} }
        )),
        std::make_shared<Tri>(Tri(
            { -1.2, 0.0, -4.7 },
            {  3.0, 2.7, -3.7 },
            {  2.9, 0.0, -4.9 },
            { 0.0f, {1, 0, 1} }
        )),
    };
    int shapeCount = sizeof(shapePtrs) / sizeof(std::shared_ptr<Shape>);


    Scene scene(2, 2, sunCol, sunStr);

    scene.lightPtrs = lightPtrs;
    scene.lightCount = lightCount;

    scene.shapePtrs = shapePtrs;
    scene.shapeCount = shapeCount;


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
            scanSpeed = w / 2;
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

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::L)
                    scene.disableLighting = !scene.disableLighting;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        if (giveControl)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                cam.pos += cam.fwd * 4.0f * dT;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                cam.pos -= cam.fwd * 4.0f * dT;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                cam.pos += cam.right * 4.0f * dT;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                cam.pos -= cam.right * 4.0f * dT;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                cam.pos += cam.up * 4.0f * dT;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
                cam.pos -= cam.up * 4.0f * dT;


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

                cam.fwd = (
                    cam.fwd * cos(sign) +
                    cam.right.Cross(cam.fwd) * sin(sign) +
                    cam.right * cam.right.Dot(cam.fwd) * (1.0f - cos(sign))
                    );
            }
            if (deltas.x != 0)
            {
                float sign = (float)deltas.x * -0.001f;

                cam.fwd = {
                    cam.fwd.x * cos(sign) + cam.fwd.z * sin(sign),
                    cam.fwd.y,
                    -cam.fwd.x * sin(sign) + cam.fwd.z * cos(sign)
                };
            }

            cam.UpdateRotation();
        }

        float
            pHeight = tan((float)(cam.fov / 2.0f) * PI / 180.0f) * 2.0f,
            pWidth = pHeight / ((float)h / (float)w);

        Vec3 blLocal(-pWidth / 2.0f, -pHeight / 2.0f, 1.0f);

        int drawStart = currPix;
        int drawEnd = std::min(dim,currPix + scanSpeed);

        #pragma omp parallel for num_threads(4)
        for (int i = drawStart; i < drawEnd; i++)
        {
            unsigned int
                x = i % w,
                y = i / w;

            float
                v = 1.0f - (float)y / (float)h,
                u = (float)x / (float)w;

            Vec3 pLocal = blLocal + Vec3(pWidth * u, pHeight * v, 0.0f);
            Vec3 p = cam.right * pLocal.x + cam.up * pLocal.y + cam.fwd * pLocal.z;
            p.Normalize();

            Ray ray(cam.pos, p);
            Hit hit = {};

            Vec3 hitCol = CastRayInScene(scene, ray, hit, 0);
            
            img.setPixel(x, y, {
                (uint8_t)(hitCol.x * 255.0), 
                (uint8_t)(hitCol.y * 255.0), 
                (uint8_t)(hitCol.z * 255.0)
            });
        }
        currPix = drawEnd % dim;


        /*Vec3 spCPoint = Vec3(9.5, 9.5, 0.0);
        float axisLength = 8.0f;

        double
            camPitch = asin(-cam.fwd.y),
            camYaw = atan2(cam.fwd.x, cam.fwd.z);

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

        Matrix4x4 camBase = Matrix4x4(camRight, camUp, cam.fwd);
        Matrix4x4 camBaseRot = camBase.Rotate(Vec3(0.0, -camPitch, -camYaw));

        Vec3 eAngles = camBase.GetEulerAngles(cam.fwd);
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
