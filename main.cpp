
#include "Utils.h"
#include "Vec3.h"
#include "Phys.h"
#include "Graphics.h"
#include "Shapes.h"
#include "Lights.h"
#include "Scene.h"
#include "SFML/Graphics/Shader.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <omp.h>
#include <new>
#include <math.h>




int main()
{
    if (!sf::Shader::isAvailable())
        return 1;

    // Build Scene
    Cam cam(
        75.0f, true, 3.5f,
        Vec3(0.0, 5.0, -5.0),
        { 0.0, 0.0, 1.0 }
    );

    Scene scene(nullptr, false, 2, 1, 0);


    Vec3 sunPos(0.8, 0.7, -0.3);
    sunPos.Normalize();
    double sunLen = 100000.0;
    sunPos *= sunLen;

    Shape* shapePtrs[] = {
        //0
        new Sphere(
            sunLen, Vec3(0, 0, 0),
            Material(Color(0.5, 0.65, 1.0), 1, 1, 0, Color(0.1, 0.25, 0.85), 0.01)
        ),
        new Sphere(
            7500.0, sunPos,
            Material(Color(1.0, 0.85, 0.4), 1, 1, 0, Color(1.0, 0.75, 0.45), 50)
        ),

        new Plane(
            Vec3(0,0,0),
            Vec3(0,1,0),
            Material(Color(0.75, 0.95, 0.5), 1, 1, 0, Color(0,0,0), 0)
        ),


        new Sphere(
            1.5, Vec3(3.5, 1.5, -3.0),
            Material(Color(1,1,1), 0.0, riGlass, 0, Color(0,0,0), 0)
        ),

        new Sphere(
            1.0, Vec3(3.0, 0.5, 0.0),
            Material(Color(0.25, 0.25, 0.25), 1, 1, 0, Color(0,0,0), 0)
        ),

        new AABB(
            Vec3(1.75, 0.0, 0.2),
            Vec3(2.5, 0.75, 1.8),
            Material(Color(1.0, 0.25, 0.66), 1, 1, 0, Color(0,0,0), 0)
        ),
    };
    int shapeCount = sizeof(shapePtrs) / sizeof(Shape*);
    if (shapePtrs[0] == nullptr)
        shapeCount = 0;

    scene.shapePtrs = shapePtrs;
    scene.shapeCount = shapeCount;


    // Render Scene
    const unsigned int 
        w = 1280,
        h = 720,
        dim = w * h;

    bool cumulativeLighting = true;
    unsigned int cumulativeFrameCount = 0;

    Color* frame = new Color[dim];
    for (int i = 0; i < dim; i++)
        frame[i] = Color();

    sf::RenderWindow window(
        sf::VideoMode(w, h),
        "SFML Window" 
        //, sf::Style::Fullscreen
    );

    sf::RenderTexture renderTex;
    renderTex.create(w, h);

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
    sf::Sprite drawSprite;
    sf::Shader shader;

    img.create(w, h, sf::Color::Black);
    drawSprite.setScale((float)scaleW, (float)scaleH);
    shader.loadFromFile("RaytracerShader.frag", sf::Shader::Type::Fragment);


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
        //cam.perspective = false;
        //cam.fov = 1.0f;
        scene.disableLighting = true;
        scene.lightingType = 2;
        scene.maxBounces = 3;
        cumulativeLighting = false; 
        randomizeSampleDir = false;
        disableScanSpeed = true;
        scanSpeed = dim;
        giveControl = true;
        perPixelSamples = 3;
    }


    while (window.isOpen())
    {
        lT = tT;
        tT = clock.getElapsedTime().asSeconds();
        dT = tT - lT;

        /*if (!disableScanSpeed)
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
        }*/


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
            double speedMult = (double)cam.speed * dT * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 3.0 : 1.0);
            speedMult /= (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ? 7.0 : 1.0);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                cam.origin += cam.fwd * speedMult;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                cam.origin -= cam.fwd * speedMult;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                cam.origin += cam.right * speedMult;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                cam.origin -= cam.right * speedMult;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                cam.origin += cam.up * speedMult;
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
                cam.origin -= cam.up * speedMult;

            deltas = fixed - sf::Mouse::getPosition();
            if (deltas != sf::Vector2i(0, 0))
                sf::Mouse::setPosition(fixed);

            if (deltas.y != 0)
            {
                float sign = (float)deltas.y * -0.001f;
                int verticality = (sign > 0) ? -1 : 1;

                Vec3 offAngle = cam.fwd - Vec3(0, verticality, 0);
                offAngle.NormalizeApprox();

                cam.fwd = (
                    cam.fwd * cos(sign) +
                    cam.right.Cross(cam.fwd) * sin(sign) +
                    cam.right * cam.right.Dot(cam.fwd) * (1.0f - cos(sign))
                    );

                if (offAngle.Dot(cam.fwd) <= 0)
                    cam.fwd = Vec3(0, verticality, 0) + offAngle * utils::MINVAL * 100.0;
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
            viewHeight = tan(((double)cam.fov / 2.0) * utils::PI / 180.0) * 2.0,
            viewWidth = viewHeight / ((double)h / (double)w);

        Vec3 botLeftLocal(-viewWidth / 2.0, -viewHeight / 2.0, 1.0);

        int drawStart = currPix;
        int drawEnd = std::min(dim, currPix + scanSpeed);

        std::vector<Refraction> riQueueShared(1, {riAir, nullptr});

        for (int i = 0; i < scene.shapeCount; i++)
        {
            Shape* shape = scene.shapePtrs[i];
            if (shape->PointIntersect(cam.origin))
                riQueueShared.push_back({shape->mat.refractIndex, shape});
        }

        goto skipHehe;
        #pragma omp parallel for num_threads(6)
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
                    v = 1.0 - ((double)y + (utils::RandNum() - 0.5) / 2.0) / (double)h;
                    u = ((double)x + (utils::RandNum() - 0.5) / 2.0) / (double)w;
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
                Hit hit = {0.0, pixPos, pixDir, nullptr};

                std::vector<Refraction> riQueue;
                riQueue = riQueueShared;

                SurfaceHitInfo hitSurface = scene.CastRay(ray, hit, riQueue);
                hitCol += (hitSurface.surfaceColor * hitSurface.cumulativeLight.ApplyGamma(1.5)) + hitSurface.surfaceEmission;
            }
            hitCol /= perPixelSamples;

            frame[i] = hitCol;
            Color toRender = frame[i];
            toRender.Clamp();

            img.setPixel(x, y, {
                (uint8_t)(toRender.r * 255.0), 
                (uint8_t)(toRender.g * 255.0), 
                (uint8_t)(toRender.b * 255.0)
            });
        }
    skipHehe:

        {
            double
                viewHeight = tan(((double)cam.fov / 2.0) * utils::PI / 180.0) * 2.0,
                viewWidth = viewHeight / ((double)h / (double)w);

            shader.setUniform("viewHeight", (float)viewHeight);
            shader.setUniform("viewWidth", (float)viewWidth);

            shader.setUniform("camPos", cam.origin.ToShader());
            shader.setUniform("camFwd", cam.fwd.ToShader());
            shader.setUniform("camUp", cam.up.ToShader());
            shader.setUniform("camRight", cam.right.ToShader());
        }



        tex.loadFromImage(img);
        sprite.setTexture(tex);


        if (!cumulativeLighting)
        {
            shader.setUniform("frameCount", 0);
            renderTex.clear();
        }
        else
            shader.setUniform("frameCount", (int)cumulativeFrameCount);

        shader.setUniform("texture", sf::Shader::CurrentTexture);
        shader.setUniform("lastFrame", renderTex.getTexture());

        renderTex.draw(sprite, &shader);
        renderTex.display();
        drawSprite.setTexture(renderTex.getTexture());

        window.clear();
        window.draw(drawSprite);
        window.display();


        if (!pauseSampling)
        {
            currPix = drawEnd % dim;
            if (currPix != drawEnd)
                cumulativeFrameCount++;
        }
    }

    for (int i = 0; i < shapeCount; i++)
        delete shapePtrs[i];
    delete[] frame;
    return 0;
}
