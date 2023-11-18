
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




int main()
{
    if (!sf::Shader::isAvailable())
        return 1;

    // Build Scene
    Cam cam(
        75.0f, true, 3.5f,
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
        perPixelSamples = 9;
        maxBounces = 6;
    }

    {
        shader.setUniform("imgW", (int)w);
        shader.setUniform("imgH", (int)h);

        shader.setUniform("maxBounces", (int)maxBounces);
    }


    unsigned int 
        cumulativeFrameCount = 0,
        totFrames = 0;
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
            else if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseWheelScrolled)
            {
                if (event.mouseButton.button == 1)
                    giveControl = !giveControl;

                if (event.mouseWheelScroll.delta != 0.0f)
                    cam.fov = std::clamp(cam.fov - event.mouseWheelScroll.delta, 0.01f, 179.99f);
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::L)
                    disableLighting = !disableLighting;
                else if (event.key.code == sf::Keyboard::R)
                    randomizeSampleDir = !randomizeSampleDir;
                else if (event.key.code == sf::Keyboard::O)
                    cam.perspective = !cam.perspective;
                else if (event.key.code == sf::Keyboard::C)
                {
                    cumulativeFrameCount = 0;
                    cumulativeLighting = !cumulativeLighting;
                }
                else if (event.key.code == sf::Keyboard::V)
                {
                    cumulativeFrameCount = 0; 
                    renderTex.clear();
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
