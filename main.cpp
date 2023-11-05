
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <omp.h>

#include "Utils.h"
#include "Primitives.h"

struct Scene
{
    bool disableLighting;
    int lightingType;

    int maxBounces;
    int raySplits;

    Color skyCol;

    std::shared_ptr<Light>* lightPtrs;
    int lightCount;

    std::shared_ptr<Shape>* shapePtrs;
    int shapeCount;

    Scene(bool disableLighting, int lightingType, int maxBounces, int raySplits, Color skyCol) :
        disableLighting(disableLighting), lightingType(lightingType),
        maxBounces(maxBounces), raySplits(raySplits), 
        skyCol(skyCol),
        lightPtrs(nullptr), lightCount(0), 
        shapePtrs(nullptr), shapeCount(0)
    {}
};



SurfaceHitInfo CastRayInScene(Scene& scene, Ray ray, Hit& hit, int bounce = 0)
{
    SurfaceHitInfo surface = SurfaceHitInfo(
        Color(), 
        Color(),
        Color()
    );

    Hit bestHit = {};
    double len = 0.0;
    bool hasHitSomething = false;

    for (int j = 0; j < scene.shapeCount; j++)
    {
        Shape* currShape = scene.shapePtrs[j].get();

        if (hit.target != nullptr)
            if (((Shape*)hit.target) == currShape)
                continue;

        Hit tempHit = {};
        if (currShape->RayIntersect(ray, &tempHit))
        {
            if (hasHitSomething && tempHit.len >= len)
                continue;

            bestHit = tempHit;
            len = tempHit.len;
            hasHitSomething = true;
        }
    }

    if (hasHitSomething)
    {
        Shape* hitShape = (Shape*)bestHit.target;

        surface.surfaceColor = hitShape->mat.col;
        surface.surfaceEmission = hitShape->mat.emissionCol * hitShape->mat.emissionStr;

        if (scene.disableLighting)
        {
            surface.cumulativeLight = Color(1, 1, 1);
            goto endHit;
        }

        if (scene.lightingType != 2)
        {
            for (int j = 0; j < scene.lightCount; j++)
            {
                Light* currLight = scene.lightPtrs[j].get();
                Vec3 dirToLight = currLight->GetRelativePos(bestHit.origin);
                dirToLight.Normalize();

                if (bestHit.normal.Dot(dirToLight) <= 0)
                    continue;

                double distSqr = currLight->GetDistSqr(bestHit.origin);

                Ray lightRay(bestHit.origin, dirToLight);
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

                surface.cumulativeLight += currLight->col * 
                    currLight->GetIntensity(lightRay, bestHit.normal);
            }
        }

        if (scene.lightingType != 0 && bounce <= scene.maxBounces)
        {
            for (int i = 0; i <= scene.raySplits; i++)
            {
                Vec3 randDir = bestHit.normal + RandDir();
                randDir.Normalize();

                Vec3 reflectDir = ray.dir.Reflect(bestHit.normal);

                Vec3 bounceDir = randDir.VLerp(reflectDir, ((Shape*)bestHit.target)->mat.reflectivity);
                Ray bounceRay(bestHit.origin, bounceDir);
                Hit bounceHit = {0.0, Vec3(), Vec3(), bestHit.target};

                SurfaceHitInfo bounceSurface = CastRayInScene(scene, bounceRay, bounceHit, bounce + 1);
                Color bounceCol = (bounceSurface.surfaceColor * bounceSurface.cumulativeLight) + bounceSurface.surfaceEmission;

                bounceCol /= (double)(scene.raySplits + 1);
                surface.cumulativeLight += bounceCol;
            }
        }
    }
    else
    {
        if (bounce == 0)
            surface.surfaceEmission = scene.skyCol;
        else
            surface.surfaceEmission = scene.skyCol * hit.normal.Dot(ray.dir * -1.0);
    }

endHit:
    hit = bestHit;

    surface.surfaceColor.Max();
    surface.surfaceEmission.Max();
    surface.cumulativeLight.Max();
    return surface;
}



int main()
{
    // Build Scene
    Cam cam(
        80.0f,
        Vec3(2.5, 2.0, -1.0),
        { 0.0, 0.0, 1.0 }
    );

    Scene scene(true, true, 2, 0, Color());


    std::shared_ptr<Light> lightPtrs[] = {
        /*std::make_shared<GlobalLight>(GlobalLight(
            Vec3(-1, -1, -1), 1.0, scene.skyCol
        )),*/
        /*std::make_shared<PointLight>(PointLight(
            Vec3(2.5, 3.9, 1.75),
            0.0, 0.33, Color(1,1,1)
        )),*/
        0
    };
    int lightCount = 0; //sizeof(lightPtrs) / sizeof(std::shared_ptr<Light>);

    scene.lightPtrs = lightPtrs;
    scene.lightCount = lightCount;


    /*std::shared_ptr<Shape> shapePtrs[] = {
        std::make_shared<Plane>(Plane(
            Vec3(0.0, 0.0, 0.0),
            Vec3(0.0, 1.0, 0.0),
            Material(Color(1.0, 1.0, 1.0))
        )),


        std::make_shared<Sphere>(Sphere(
            6.0,
            Vec3(-3.875, 6.0, 24.0),
            Material(Color(1, 1, 1), 5.0)
        )),


        std::make_shared<Sphere>(Sphere(
            3.0,
            Vec3(-3.875, 2.0, -6.0),
            Material(Color(1, 1, 1), 1.0, Color(), 0.0)
        )),

        std::make_shared<Sphere>(Sphere(
            2.0,
            Vec3(0.0, 2.0, 0.0),
            Material(Color(1, 1, 1))
        )),
        std::make_shared<Sphere>(Sphere(
            1.5,
            Vec3(-3.5, 1.5, 0.0),
            Material(Color(1, 0, 0))
        )),
        std::make_shared<Sphere>(Sphere(
            1.0,
            Vec3(-6.0, 1.0, 0.0),
            Material(Color(0, 1, 0), 0.5, Color(), 0.0)
        )),
        std::make_shared<Sphere>(Sphere(
            0.75,
            Vec3(-7.75, 0.75, 0.0),
            Material(Color(0, 0, 1))
        )),
    };*/
    std::shared_ptr<Shape> shapePtrs[] = {
        std::make_shared<AABB>(AABB( // Light
            Vec3(1.2, 3.95, 1.0),
            Vec3(3.8, 4.00, 2.5),
            Material(Color(1,1,1), 0.0, Color(1,1,1), 2.2)
        )),


        /*std::make_shared<Sphere>(Sphere(
            0.75,
            Vec3(1.25, 2.0, 1.75),
            Material(Color(1.0, 1.0, 1.0), 1.0, Color(), 0.0)
        )),*/
        /*std::make_shared<Sphere>(Sphere(
            1.0,
            Vec3(2.5, 2.0, 1.75),
            Material(Color(0.9, 0.9, 0.9), 0.75, Color(1,1,1), 0.025)
        )),*/

        std::make_shared<OBB>(OBB(
            Vec3(2.5, 2.0, 1.75),
            Vec3(0.4, 0.2, -0.1),
            Vec3(-0.7, 0.4, 0.1),
            Vec3(1.0, 0.3, 0.25),
            Material(Color(1.0, 0.7, 0.5), 0.5, Color(1,1,0), 0.05)
        )),



        std::make_shared<Tri>(Tri( // Floor
            Vec3(0.0, 0.0, 0.0),
            Vec3(5.0, 0.0, 3.5),
            Vec3(5.0, 0.0, 0.0),
            Material(Color(0.2, 0.2, 1.0), 0.0, Color(), 0.0)
        )),
        std::make_shared<Tri>(Tri(
            Vec3(5.0, 0.0, 3.5),
            Vec3(0.0, 0.0, 0.0),
            Vec3(0.0, 0.0, 3.5),
            Material(Color(0.2, 0.2, 1.0), 0.0, Color(), 0.0)
        )),


        std::make_shared<Tri>(Tri( // Roof
            Vec3(0.0, 4.0, 0.0),
            Vec3(5.0, 4.0, 3.5),
            Vec3(0.0, 4.0, 3.5),
            Material(Color(0.25, 0.25, 0.4), 0.0, Color(), 0.0)
        )),
        std::make_shared<Tri>(Tri(
            Vec3(5.0, 4.0, 3.5),
            Vec3(0.0, 4.0, 0.0),
            Vec3(5.0, 4.0, 0.0),
            Material(Color(0.25, 0.25, 0.4), 0.0, Color(), 0.0)
        )),


        std::make_shared<Tri>(Tri( // Front
            Vec3(0.0, 0.0, 0.0),
            Vec3(5.0, 0.0, 0.0),
            Vec3(5.0, 4.0, 0.0),
            Material(Color(1.0, 0.2, 0.2), 0.0, Color(), 0.0)
        )),
        std::make_shared<Tri>(Tri(
            Vec3(5.0, 4.0, 0.0),
            Vec3(0.0, 4.0, 0.0),
            Vec3(0.0, 0.0, 0.0),
            Material(Color(1.0, 0.2, 0.2), 0.0, Color(), 0.0)
        )),


        std::make_shared<Tri>(Tri( // Left
            Vec3(0.0, 0.0, 0.0),
            Vec3(0.0, 4.0, 0.0),
            Vec3(0.0, 0.0, 3.5),
            Material(Color(0.2, 1.0, 0.2), 0.0, Color(), 0.0)
        )),
        std::make_shared<Tri>(Tri(
            Vec3(0.0, 4.0, 3.5),
            Vec3(0.0, 0.0, 3.5),
            Vec3(0.0, 4.0, 0.0),
            Material(Color(0.2, 1.0, 0.2), 0.0, Color(), 0.0)
        )),


        std::make_shared<Tri>(Tri( // Back
            Vec3(5.0, 0.0, 3.5),
            Vec3(0.0, 0.0, 3.5),
            Vec3(5.0, 4.0, 3.5),
            Material(Color(1.0, 0.2, 0.2), 0.0, Color(), 0.0)
        )),
        std::make_shared<Tri>(Tri(
            Vec3(0.0, 4.0, 3.5),
            Vec3(5.0, 4.0, 3.5),
            Vec3(0.0, 0.0, 3.5),
            Material(Color(1.0, 0.2, 0.2), 0.0, Color(), 0.0)
        )),


        std::make_shared<Tri>(Tri( // Right
            Vec3(5.0, 0.0, 0.0),
            Vec3(5.0, 0.0, 3.5),
            Vec3(5.0, 4.0, 0.0),
            Material(Color(0.2, 1.0, 0.2), 0.0, Color(), 0.0)
        )),
        std::make_shared<Tri>(Tri(
            Vec3(5.0, 4.0, 3.5),
            Vec3(5.0, 4.0, 0.0),
            Vec3(5.0, 0.0, 3.5),
            Material(Color(0.2, 1.0, 0.2), 0.0, Color(), 0.0)
        )),
    };
    int shapeCount = sizeof(shapePtrs) / sizeof(std::shared_ptr<Shape>);

    scene.shapePtrs = shapePtrs;
    scene.shapeCount = shapeCount;


    // Render Scene
    const unsigned int 
        w = 320,
        h = 180,
        dim = w * h;

    bool cumulativeLighting = true;
    int cumulativeFrameCount = 0;

    Color* frame = new Color[dim];
    for (int i = 0; i < dim; i++)
        frame[i] = Color();

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

    bool randomizeSampleDir = false;
    bool disableScanSpeed = false;
    unsigned int scanSpeed = dim / 8;
    unsigned int currPix = 0;

    sf::Image img;
    sf::Texture tex;
    sf::Sprite sprite;

    img.create(w, h, sf::Color::Black);
    sprite.setScale((float)scaleW, (float)scaleH);


    sf::Clock clock;
    double lT = 0.0, tT = 0.0, dT = 0.0;

    sf::Vector2i deltas;
    sf::Vector2i fixed(window.getSize());
    fixed.x /= 2;
    fixed.y /= 2;

    bool giveControl = true;
    bool pauseSampling = false;


    {
        scene.disableLighting = false;

        randomizeSampleDir = true;

        disableScanSpeed = true;
        scanSpeed = dim;

        giveControl = false;

        cumulativeLighting = true;
    }

    while (window.isOpen())
    {
        lT = tT;
        tT = clock.getElapsedTime().asSeconds();
        dT = tT - lT;

        if (!disableScanSpeed)
        {
            if (dT > 0.3)
                scanSpeed -= w * 32;
            else if (dT > 0.2)
                scanSpeed -= w * 24;
            else if (dT > 0.15)
                scanSpeed -= w * 12;
            else if (dT > 0.125)
                scanSpeed -= w * 4;
            else if (dT > 0.11)
                scanSpeed -= w * 2;
            else if (dT > 0.09)
                scanSpeed -= w;

            else if (dT < 0.07)
                scanSpeed += w / 2;
            else if (dT < 0.06)
                scanSpeed += w;
            else if (dT < 0.04)
                scanSpeed += w * 2;
            else if (dT < 0.02)
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
                {
                    giveControl = !giveControl;

                    currPix = 0;
                    cumulativeFrameCount = 0;
                    cumulativeLighting = !giveControl;
                }

                if (event.mouseWheelScroll.delta != 0.0f)
                    cam.fov -= event.mouseWheelScroll.delta;
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::L)
                {
                    scene.disableLighting = !scene.disableLighting;

                    if (scene.disableLighting)
                    {
                        currPix = 0;
                        cumulativeFrameCount = 0;
                        cumulativeLighting = false;
                    }
                }
                else if (event.key.code == sf::Keyboard::E)
                {
                    scene.lightingType = ++scene.lightingType % 3;

                }
                else if (event.key.code == sf::Keyboard::R)
                    randomizeSampleDir = !randomizeSampleDir;
                else if (event.key.code == sf::Keyboard::P)
                    pauseSampling = !pauseSampling;
                else if (event.key.code == sf::Keyboard::C)
                {
                    currPix = 0;
                    cumulativeFrameCount = 0;
                    for (int i = 0; i < dim; i++)
                        frame[i] = Color();
                }
                else if (event.key.code == sf::Keyboard::Tab && !event.key.alt)
                {
                    disableScanSpeed = !disableScanSpeed;
                    scanSpeed = (disableScanSpeed) ? dim : dim / 12;
                }
                else if (event.key.code == sf::Keyboard::Q)
                {

                    for (int y = 1; y < h - 1; y++)
                    {
                        for (int x = 1; x < w - 1; x++)
                        {
                            if (x == 0 || x == w - 1)
                                continue;
                            if (y == 0 || y == h - 1)
                                continue;

                            int
                                tl = (x - 1) + (y - 1) * w,
                                tm = (x + 0) + (y - 1) * w,
                                tr = (x + 1) + (y - 1) * w,
                                ml = (x - 1) + (y + 0) * w,
                                mm = (x + 0) + (y + 0) * w,
                                mr = (x + 1) + (y + 0) * w,
                                bl = (x - 1) + (y + 1) * w,
                                bm = (x + 0) + (y + 1) * w,
                                br = (x + 1) + (y + 1) * w;

                            Color avgCol =
                                frame[tl] + frame[tm] + frame[tr] +
                                frame[ml] + frame[mm] + frame[mr] +
                                frame[bl] + frame[bm] + frame[br];
                            avgCol /= 9.0;
                            avgCol *= 9.0;
                            avgCol /= cumulativeFrameCount;

                            frame[mm] = avgCol;

                        }
                    }

                    cumulativeFrameCount = 8;
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        if (giveControl)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                cam.origin += cam.fwd * 4.0f * dT;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                cam.origin -= cam.fwd * 4.0f * dT;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                cam.origin += cam.right * 4.0f * dT;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                cam.origin -= cam.right * 4.0f * dT;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                cam.origin += cam.up * 4.0f * dT;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
                cam.origin -= cam.up * 4.0f * dT;

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

        double
            viewHeight = tan(((double)cam.fov / 2.0) * PI / 180.0) * 2.0,
            viewWidth = viewHeight / ((double)h / (double)w);

        Vec3 botLeftLocal(-viewWidth / 2.0, -viewHeight / 2.0, 1.0);

        int drawStart = currPix;
        int drawEnd = std::min(dim, currPix + scanSpeed);

        #pragma omp parallel for num_threads(3)
        for (int i = drawStart; i < drawEnd; i++)
        {
            if (pauseSampling)
                break;

            unsigned int
                x = i % w,
                y = i / w;

            double u, v;
            if (randomizeSampleDir)
            {
                v = 1.0 - ((double)y + (RandNum() - 0.5) / 2.0) / (double)h;
                u = ((double)x + (RandNum() - 0.5) / 2.0) / (double)w;
            }
            else
            {
                v = 1.0 - (double)y / (double)h;
                u = (double)x / (double)w;
            }

            Vec3 dirLocal = botLeftLocal + Vec3(viewWidth * u, viewHeight * v, 0.0);
            Vec3 pixDir = cam.right * dirLocal.x + cam.up * dirLocal.y + cam.fwd * dirLocal.z;
            pixDir.Normalize();

            Ray ray(cam.origin, pixDir);
            Hit hit = {};

            SurfaceHitInfo hitSurface = CastRayInScene(scene, ray, hit);
            Color hitCol = (hitSurface.surfaceColor * hitSurface.cumulativeLight) + hitSurface.surfaceEmission;
            //hitCol.Clamp();
            Color toRender = Color();

            if (cumulativeLighting)
            {
                /*if (randomizeSampleDir)
                {
                    double avgWeight = 1.0 / (double)(cumulativeFrameCount + 1);
                    frame[i] = (frame[i] * (1.0 - avgWeight)) + (hitCol * avgWeight);
                    toRender = frame[i];
                }
                else
                {
                    frame[i] += hitCol;
                    cumulativeDivisor = (double)(cumulativeFrameCount + 1);
                }*/

                frame[i] += hitCol;
                toRender = frame[i] / (double)(cumulativeFrameCount + 1);
            }
            else
            {
                frame[i] = hitCol;
                toRender = frame[i];
            }

            toRender.Clamp();
            img.setPixel(x, y, {
                (uint8_t)(toRender.r * 255.0), 
                (uint8_t)(toRender.g * 255.0), 
                (uint8_t)(toRender.b * 255.0)
            });
        }

        if (!pauseSampling)
        {
            currPix = drawEnd % dim;
            if (currPix != drawEnd)
                cumulativeFrameCount++;
        }

        tex.loadFromImage(img);
        sprite.setTexture(tex);

        window.clear();
        window.draw(sprite);
        window.display();
    }

    for (int i = 0; i < shapeCount; i++)
        shapePtrs[i].reset();
    for (int i = 0; i < lightCount; i++)
        lightPtrs[i].reset();

    //shapePtrs->reset();
    //lightPtrs->reset();
    delete[] frame;

    return 0;
}
