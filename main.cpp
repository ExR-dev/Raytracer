
#include "Utils.h"
#include "Vec3.h"
#include "Phys.h"
#include "Graphics.h"
#include "Shapes.h"
#include "Lights.h"
#include "Scene.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <omp.h>



int main()
{
    // Build Scene
    Cam cam(
        80.0f, true,
        Vec3(2.5, 2.0, -1.0),
        { 0.0, 0.0, 1.0 }
    );

    Scene scene(true, true, 8, 0, Color());


    Light* lightPtrs[] = {
        new GlobalLight(
            Vec3(0, -1, 0), 0.0, scene.skyCol
        ),
    };
    int lightCount = sizeof(lightPtrs) / sizeof(Light*);

    scene.lightPtrs = lightPtrs;
    scene.lightCount = lightCount;


    Shape* shapePtrs[] = {
        new AABB( // Light
            Vec3(0.2, 3.98, 0.2),
            Vec3(4.8, 4.00, 3.3),
            Material(Color(0,0,0), 1.0, 1.0, 0.0, Color(1,1,1), 0.9)
        ),


        new Sphere(
            1.0, Vec3(1.5, 2.0, 1.75),
            Material(Color(1.0, 1.0, 1.0), 1.0, 1.0, 1.0, Color(1.0,1.0,1.0), 0.00)
        ),
        new Sphere(
            1.0, Vec3(3.5, 2.0, 1.75),
            Material(Color(1.0, 1.0, 1.0), 1.0, 1.0, 1.0, Color(1.0,1.0,1.0), 0.00)
        ),


        new Tri( // Floor
            Vec3(0.0, 0.0, 0.0),
            Vec3(5.0, 0.0, 3.5),
            Vec3(5.0, 0.0, 0.0),
            Material(Color(0.7, 0.7, 0.8), 1.0, 1.0, 0.15, Color(), 0.0)
        ),
        new Tri(
            Vec3(5.0, 0.0, 3.5),
            Vec3(0.0, 0.0, 0.0),
            Vec3(0.0, 0.0, 3.5),
            Material(Color(0.7, 0.7, 0.8), 1.0, 1.0, 0.15, Color(), 0.0)
        ),

        new Tri( // Roof
            Vec3(0.0, 4.0, 0.0),
            Vec3(5.0, 4.0, 3.5),
            Vec3(0.0, 4.0, 3.5),
            Material(Color(0.075, 0.075, 0.075), 1.0, 1.0, 1.0, Color(), 0.0)
        ),
        new Tri(
            Vec3(5.0, 4.0, 3.5),
            Vec3(0.0, 4.0, 0.0),
            Vec3(5.0, 4.0, 0.0),
            Material(Color(0.075, 0.075, 0.075), 1.0, 1.0, 1.0, Color(), 0.0)
        ),

        new Tri( // Front
            Vec3(0.0, 0.0, 0.0),
            Vec3(5.0, 0.0, 0.0),
            Vec3(5.0, 4.0, 0.0),
            Material(Color(1.0, 0.7, 0.4), 1.0, 1.0, 0.0, Color(), 0.0)
        ),
        new Tri(
            Vec3(5.0, 4.0, 0.0),
            Vec3(0.0, 4.0, 0.0),
            Vec3(0.0, 0.0, 0.0),
            Material(Color(1.0, 0.7, 0.4), 1.0, 1.0, 0.0, Color(), 0.0)
        ),

        new Tri( // Left
            Vec3(0.0, 0.0, 0.0),
            Vec3(0.0, 4.0, 0.0),
            Vec3(0.0, 0.0, 3.5),
            Material(Color(0.9, 1.0, 0.95), 1.0, 1.0, 0.99, Color(), 0.0)
        ),
        new Tri(
            Vec3(0.0, 4.0, 3.5),
            Vec3(0.0, 0.0, 3.5),
            Vec3(0.0, 4.0, 0.0),
            Material(Color(0.9, 1.0, 0.95), 1.0, 1.0, 0.99, Color(), 0.0)
        ),

        new Tri( // Back
            Vec3(5.0, 0.0, 3.5),
            Vec3(0.0, 0.0, 3.5),
            Vec3(5.0, 4.0, 3.5),
            Material(Color(0.4, 0.7, 1.0), 1.0, 1.0, 0.0, Color(), 0.0)
        ),
        new Tri(
            Vec3(0.0, 4.0, 3.5),
            Vec3(5.0, 4.0, 3.5),
            Vec3(0.0, 0.0, 3.5),
            Material(Color(0.4, 0.7, 1.0), 1.0, 1.0, 0.0, Color(), 0.0)
        ),

        new Tri( // Right
            Vec3(5.0, 0.0, 0.0),
            Vec3(5.0, 0.0, 3.5),
            Vec3(5.0, 4.0, 0.0),
            Material(Color(0.9, 1.0, 0.95), 1.0, 1.0, 0.99, Color(), 0.0)
        ),
        new Tri(
            Vec3(5.0, 4.0, 3.5),
            Vec3(5.0, 4.0, 0.0),
            Vec3(5.0, 0.0, 3.5),
            Material(Color(0.9, 1.0, 0.95), 1.0, 1.0, 0.99, Color(), 0.0)
        ),
    };
    int shapeCount = sizeof(shapePtrs) / sizeof(Shape*);

    scene.shapePtrs = shapePtrs;
    scene.shapeCount = shapeCount;


    // Render Scene
    const unsigned int 
        w = 320,
        h = 180,
        dim = w * h;

    bool cumulativeLighting = true;
    unsigned int cumulativeFrameCount = 0;

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
    unsigned int perPixelSamples = 1;


    {
        scene.disableLighting = false;
        cumulativeLighting = true; 
        randomizeSampleDir = true;
        disableScanSpeed = true;
        scanSpeed = dim;
        giveControl = false;
        perPixelSamples = 2;
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
                    scene.disableLighting = !scene.disableLighting;
                else if (event.key.code == sf::Keyboard::E)
                    scene.lightingType = (scene.lightingType + 1) % 3;
                else if (event.key.code == sf::Keyboard::R)
                    randomizeSampleDir = !randomizeSampleDir;
                else if (event.key.code == sf::Keyboard::P)
                    pauseSampling = !pauseSampling;
                else if (event.key.code == sf::Keyboard::O)
                    cam.perspective = !cam.perspective;
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
                    Color *tempFrame = new Color[dim];

                    for (int y = 0; y < h; y++)
                    {
                        for (int x = 0; x < w; x++)
                        {
                            Color avgCol = frame[x + y * w];
                            int samples = 1;

                            if (y != 0)
                            {
                                avgCol += frame[x + (y - 1) * w];
                                samples++;
                            }

                            if (y != h - 1)
                            {
                                avgCol += frame[x + (y + 1) * w];
                                samples++;
                            }

                            avgCol /= (double)samples;
                            tempFrame[x + y * w] = avgCol;
                        }
                    }

                    for (int y = 0; y < h; y++)
                    {
                        for (int x = 0; x < w; x++)
                        {
                            Color avgCol = tempFrame[x + y * w];
                            int samples = 1;

                            if (x != 0)
                            {
                                avgCol += frame[(x - 1) + y * w];
                                samples++;
                            }

                            if (x != w - 1)
                            {
                                avgCol += frame[(x + 1) + y * w];
                                samples++;
                            }

                            avgCol /= (double)samples;
                            frame[x + y * w] = avgCol;
                        }
                    }

                    delete[] tempFrame;
                    cumulativeFrameCount = 600;
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        if (giveControl)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                cam.origin += cam.fwd * 4.0 * dT;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                cam.origin -= cam.fwd * 4.0 * dT;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                cam.origin += cam.right * 4.0 * dT;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                cam.origin -= cam.right * 4.0 * dT;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                cam.origin += cam.up * 4.0 * dT;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
                cam.origin -= cam.up * 4.0 * dT;

            deltas = fixed - sf::Mouse::getPosition();
            if (deltas != sf::Vector2i(0, 0))
                sf::Mouse::setPosition(fixed);

            if (deltas.y != 0)
            {
                float sign = (float)deltas.y * -0.001f;
                int verticality = (sign > 0) ? -1 : 1;

                Vec3 offAngle = cam.fwd - Vec3(0, verticality, 0);
                offAngle.Normalize();

                cam.fwd = (
                    cam.fwd * cos(sign) +
                    cam.right.Cross(cam.fwd) * sin(sign) +
                    cam.right * cam.right.Dot(cam.fwd) * (1.0f - cos(sign))
                    );

                if (offAngle.Dot(cam.fwd) <= 0)
                    cam.fwd = Vec3(0, verticality, 0) + offAngle * MINVAL * 100.0;
            }
            if (deltas.x != 0 && abs(cam.fwd.y) != 1.0)
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

        #pragma omp parallel for num_threads(2)
        for (int i = drawStart; i < drawEnd; i++)
        {
            if (pauseSampling)
                break;

            unsigned int
                x = i % w,
                y = i / w;

            Color hitCol = Color();
            for (unsigned int j = 0; j < perPixelSamples; j++)
            {
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

                Vec3
                    pixPos = cam.origin,
                    pixDir = cam.fwd;

                if (cam.perspective)
                {
                    Vec3 dirLocal = botLeftLocal + Vec3(viewWidth * u, viewHeight * v, 0.0);
                    pixDir = cam.right * dirLocal.x + cam.up * dirLocal.y + cam.fwd * dirLocal.z;
                    pixDir.Normalize();
                }
                else
                    pixPos += cam.right * (viewWidth * (u - 0.5)) + cam.up * (viewHeight * (v - 0.5));

                Ray ray(pixPos, pixDir);
                Hit hit = {};

                SurfaceHitInfo hitSurface = CastRayInScene(scene, ray, hit);
                hitCol += (hitSurface.surfaceColor * hitSurface.cumulativeLight) + hitSurface.surfaceEmission;
            }
            hitCol /= perPixelSamples;

            Color toRender = Color();
            if (cumulativeLighting)
            {
                double avgWeight = 1.0 / (double)(cumulativeFrameCount + 1);
                frame[i] = (frame[i] * (1.0 - avgWeight)) + (hitCol * avgWeight);
                toRender = frame[i];
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
        delete shapePtrs[i];
    for (int i = 0; i < lightCount; i++)
        delete lightPtrs[i];
    delete[] frame;
    return 0;
}
