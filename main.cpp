
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
#include <new>
#include <math.h>
#include <format>




int main()
{
    if (!sf::Shader::isAvailable())
        return 1;

    // Build Scene
    Cam cam(
        75.0f, true, 5.0f,
        Vec3(0.0, 5.0, -7.0),
        Vec3(0.0, 0.0, 1.0)
    );


    // Render Scene
    const unsigned int 
        w = /*80,*/ /*160,*/ /*320,*/ /*640,*/ 960, /*1280,*/ /*1920,*/
        h = /*45,*/ /*90, */ /*180,*/ /*360,*/ 540, /*720, */ /*1080,*/
        dim = w * h;

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


    sf::Image img;
    sf::Texture tex;
    sf::RenderTexture renderTex;
    sf::Sprite sprite, drawSprite;
    sf::Shader shader;

    img.create(w, h, sf::Color::Black);
    renderTex.create(w, h);
    drawSprite.setScale((float)scaleW, (float)scaleH);
    shader.loadFromFile("RaytracerShader.frag", sf::Shader::Type::Fragment);


    sf::Clock clock;
    double lT = 0.0, tT = 0.0, dT = 0.0;

    sf::Vector2i deltas;
    sf::Vector2i fixed(window.getSize());
    fixed.x /= 2;
    fixed.y /= 2;


    bool cumulativeLighting, randomizeSampleDir, giveControl, disableLighting;
    unsigned int perPixelSamples, maxBounces;

    {
        giveControl = true;
        cam.fov = 70.0f;

        cumulativeLighting = false;
        randomizeSampleDir = true;
        disableLighting = false;
        perPixelSamples = 12;
        maxBounces = 8;
    }

    {
        shader.setUniform("imgW", (int)w);
        shader.setUniform("imgH", (int)h);
        shader.setUniform("maxBounces", (int)maxBounces);

        // AABBs (MAX 16)
        { 
            // Shape:   vec3(min x3), vec3(max x3)
            // Mat:     vec4(color x3, opacity x1), vec4(emission x3, strength x1), vec4(reflectivity x1, rI x1, unused x2)

            const std::string shapeName = "aabb";

            int iShape = 0, iMat = 0, iBounds = 0;
            /*shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-1.0, 2.0, -1.0));
            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(1.0, 4.0, 1.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, 1.0, 1.0, 1.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, 0.0, 0.0, 0.0));

            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(1.1, 3.2, -2.3));
            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(2.4, 3.6, -2.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, 0.0, 0.0, 1.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, 0.0, 0.0, 0.666));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));


            shader.setUniform(std::format("{}Bounds[{}]", shapeName, iBounds), sf::Glsl::Vec4(0.7, 3.0, -0.65, 2.75));
            shader.setUniform(std::format("{}BoundCoverage[{}]", shapeName, iBounds++), iMat / 3);*/

            shader.setUniform(std::format("{}Count", shapeName), iMat / 3);
        }
        
        // OBBs (MAX 16)
        { 
            // Shape:   vec3(center x3), vec3(halfLength x3), vec3(x-axis x3), vec3(y-axis x3), vec3(z-axis x3)
            // Mat:     vec4(color x3, opacity x1), vec4(emission x3, strength x1), vec4(reflectivity x1, rI x1, unused x2)

            const std::string shapeName = "obb";

            int iShape = 0, iMat = 0, iBounds = 0;
            /*shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(1.0, 4.5, -2.0));
            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(0.75, 0.5, 1.0));
            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), Normalize(Vec3(6.0, 4.0, -2.0)));
            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), Normalize(Vec3(-0.01965655, -0.384051845, -0.92310227)));
            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), Normalize(Vec3(-0.5970381, 0.73607948, -0.3189553)));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.1, 1.0, 0.5, 1.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.5, 1.0, 0.7, 0.1));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.2, 0.0, 0.0, 0.0));


            shader.setUniform(std::format("{}Bounds[{}]", shapeName, iBounds), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            shader.setUniform(std::format("{}BoundCoverage[{}]", shapeName, iBounds++), iMat / 3);*/

            shader.setUniform(std::format("{}Count", shapeName), iMat / 3);
        }

        // Spheres (MAX 16)
        {
            // Shape:   vec4(pos x3, rad x1)
            // Mat:     vec4(color x3, opacity x1), vec4(emission x3, strength x1), vec4(reflectivity x1, rI x1, unused x2)

            const std::string shapeName = "sphere";

            int iShape = 0, iMat = 0, iBounds = 0;
            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec4(1.5, 3.0, 0.0, 3.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, 1.0, 1.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, riGlass, 0.0, 0.0));

            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec4(5.0, 1.0, 0.0, 1.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, 1.0, 1.0, 0.5));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, riGlass, 0.0, 0.0));

            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec4(12.0, 1.1, 0.0, 1.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, 1.0, 0.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, riGlass, 0.0, 0.0));

            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec4(14.0, 1.1, 0.0, 1.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 1.0, 1.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, riGlass, 0.0, 0.0));

            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec4(13.0, 1.1, 1.73205, 1.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, 0.0, 1.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, riGlass, 0.0, 0.0));


            //shader.setUniform(std::format("{}Bounds[{}]", shapeName, iBounds), sf::Glsl::Vec4(1.75, 2.5, 0.25, 3.5));
            shader.setUniform(std::format("{}Bounds[{}]", shapeName, iBounds), sf::Glsl::Vec4(0,0,0,100));
            shader.setUniform(std::format("{}BoundCoverage[{}]", shapeName, iBounds++), iMat / 3);

            shader.setUniform(std::format("{}Count", shapeName), iMat / 3);
        }
        
        // Tris (MAX 32)
        {
            // Shape:   vec3(v1 x3), vec3(v2 x3), vec3(v3 x3)
            // Mat:     vec4(color x3, opacity x1), vec4(emission x3, strength x1), vec4(reflectivity x1, rI x1, unused x2)

            const std::string shapeName = "tri";

            int iShape = 0, iMat = 0, iBounds = 0;
            /*{
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 0.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 10.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 0.0, 4.5));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.85, 0.2, 0.1, 1.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));

                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 0.0, 4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 10.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 10.0, 4.5));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.85, 0.2, 0.1, 1.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            }

            {
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 0.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 0.0, 4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(3.5, 0.0, -4.5));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, 1.0, 1.0, 1.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));

                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(3.5, 0.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 0.0, 4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(3.5, 0.0, 4.5));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(1.0, 1.0, 1.0, 1.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            }

            {
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 10.0, 4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 10.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(3.5, 10.0, 4.5));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.2, 0.2, 0.9, 1.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));

                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(3.5, 10.0, 4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 10.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(3.5, 10.0, -4.5));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.2, 0.2, 0.9, 1.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            }

            {
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(3.5, 0.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(3.5, 10.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 0.0, -4.5));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.2, 0.85, 0.1, 1.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));

                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 0.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(3.5, 10.0, -4.5));
                shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(-3.5, 10.0, -4.5));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.2, 0.85, 0.1, 1.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
                shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            }


            shader.setUniform(std::format("{}Bounds[{}]", shapeName, iBounds), sf::Glsl::Vec4(0.0, 5.0, 0.0, 7.7));
            shader.setUniform(std::format("{}BoundCoverage[{}]", shapeName, iBounds++), iMat / 3);*/

            shader.setUniform(std::format("{}Count", shapeName), iMat / 3);
        }
        
        // Planes (MAX 8)
        {
            // Shape:   vec3(center x3), vec3(normal x3)
            // Mat:     vec4(color x3, opacity x1), vec4(emission x3, strength x1), vec4(reflectivity x1, rI x1, unused x2)

            const std::string shapeName = "plane";

            int iShape = 0, iMat = 0;
            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(0.0, -0.001, 0.0));
            shader.setUniform(std::format("{}Shapes[{}]", shapeName, iShape++), sf::Glsl::Vec3(0.0, 1.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.8, 0.8, 0.8, 1.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));
            shader.setUniform(std::format("{}Mats[{}]", shapeName, iMat++), sf::Glsl::Vec4(0.0, 0.0, 0.0, 0.0));

            shader.setUniform(std::format("{}Count", shapeName), iMat / 3);
        }
    }



    unsigned int 
        cumulativeFrameCount = 0,
        totFrames = 0;
    while (window.isOpen())
    {
        lT = tT;
        tT = clock.getElapsedTime().asSeconds();
        dT = tT - lT;

        bool hasMoved = false;

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
                {
                    float lFov = cam.fov;
                    cam.fov = std::clamp(cam.fov - event.mouseWheelScroll.delta, 0.01f, 179.99f);
                    
                    if (abs(cam.fov - lFov) > 0.000001)
                        hasMoved = true;
                }
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::L)
                {
                    disableLighting = !disableLighting;
                    hasMoved = true;
                }
                else if (event.key.code == sf::Keyboard::R)
                    randomizeSampleDir = !randomizeSampleDir;
                else if (event.key.code == sf::Keyboard::C)
                {
                    cumulativeFrameCount = 0;
                    cumulativeLighting = !cumulativeLighting;
                }
                else if (event.key.code == sf::Keyboard::V)
                    hasMoved = true;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            window.close();

        if (giveControl)
        {
            double speedMult = (double)cam.speed * dT * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 3.0 : 1.0);
            speedMult /= (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) ? 6.0 : 1.0);

            Vec3 camLOrigin = cam.origin;
            Vec3 camLFwd = cam.fwd;
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

            if ((camLOrigin - cam.origin).MagSqr() > 0.000001 || (camLFwd - cam.fwd).MagSqr() > 0.000001)
                hasMoved = true;
        }

        if (cumulativeLighting && hasMoved)
        {
            cumulativeFrameCount = 0;
            renderTex.clear();
        }


        {
            shader.setUniform("rndSeed", (int)((long)utils::VeryRand(h * w, 4294967295u) - 2147483647));

            float
                viewHeight = tanf((cam.fov / 2.0f) * (float)utils::PI / 180.0f) * 2.0f,
                viewWidth = viewHeight / ((float)h / (float)w);

            shader.setUniform("viewHeight", viewHeight);
            shader.setUniform("viewWidth", viewWidth);

            shader.setUniform("camPos", cam.origin.ToShader());
            shader.setUniform("camFwd", cam.fwd.ToShader());
            shader.setUniform("camUp", cam.up.ToShader());
            shader.setUniform("camRight", cam.right.ToShader());

            shader.setUniform("disableLighting", disableLighting);
            shader.setUniform("samples", (int)perPixelSamples);
            shader.setUniform("maxBounces", (int)maxBounces);
            shader.setUniform("randomizeDir", randomizeSampleDir);

            shader.setUniform("frameCount", cumulativeLighting ? (int)cumulativeFrameCount : 0);

            /*{
                double offset = (utils::RandNum() - 0.5) / 3.0;
                shader.setUniform("aabbShapes[2]", sf::Glsl::Vec3(1.1 + offset, 3.2, -2.3));
                shader.setUniform("aabbShapes[3]", sf::Glsl::Vec3(2.4 + offset, 3.6, -2.0));
            }*/
        }

        tex.loadFromImage(img);
        sprite.setTexture(tex);

        shader.setUniform("lastFrame", renderTex.getTexture());

        renderTex.draw(sprite, &shader);
        renderTex.display();
        drawSprite.setTexture(renderTex.getTexture());

        window.clear();
        window.draw(drawSprite);
        window.display();

        cumulativeFrameCount++;
        totFrames++;
    }
    return 0;
}
