#pragma once
#include "Graphics.h"
#include "SFML/Graphics/Shader.hpp"
#include "SFML/Graphics.hpp"
#include <string>

struct Viewport
{
	unsigned int
		w = 800,
		h = 800,
		dim = w * h;

	Viewport() { }

	Viewport(unsigned int w, unsigned int h) :
		w(w), h(h), dim(w * h) { }

	void Bind(sf::Shader& shader) const
	{
		shader.setUniform("imgW", (int)w);
		shader.setUniform("imgH", (int)h);
	}

	sf::Vector2u ToVecU() const
	{
		return sf::Vector2u(w, h);
	}

	sf::Vector2i ToVecI() const
	{
		return sf::Vector2i(w, h);
	}

	sf::Vector2f ToVecF() const
	{
		return sf::Vector2f(w, h);
	}
};

struct Cam
{
	float fov = 75.0f;
	bool perspective = true;
	float speed = 5.0f;

	Vec3
		origin = Vec3(0,0,0),
		fwd = Vec3(0,0,1), 
		right = Vec3(1,0,0), 
		up = Vec3(0,1,0);

	Viewport viewport;

	Cam() {}


	Cam(float fov, bool perspective, float speed, const Vec3& origin, const Vec3& fwd, Viewport viewport) :
		fov(fov), perspective(perspective), speed(speed), origin(origin), fwd(fwd), viewport(viewport)
	{
		UpdateRotation();
	}

	void UpdateRotation()
	{
		fwd.Normalize();

		right = fwd.Cross({ 0, -1, 0 });
		right.Normalize();

		up = fwd.Cross(right);
		up.Normalize();
	}

	void Bind(sf::Shader& shader) const
	{
		float
			viewHeight = tanf((fov / 2.0f) * (float)utils::PI / 180.0f) * 2.0f,
			viewWidth = viewHeight / ((float)viewport.h / (float)viewport.w);

		shader.setUniform("viewHeight", viewHeight);
		shader.setUniform("viewWidth", viewWidth);

		shader.setUniform("camPos", origin.ToShader());
		shader.setUniform("camFwd", fwd.ToShader());
		shader.setUniform("camUp", up.ToShader());
		shader.setUniform("camRight", right.ToShader());

		viewport.Bind(shader);
	}
};

struct Skybox
{
	bool showSkybox = false;
	sf::Glsl::Vec3 peakCol = sf::Glsl::Vec3(0.21375, 0.2565, 0.285);
	sf::Glsl::Vec3 horizonCol = sf::Glsl::Vec3(0.153, 0.19125, 0.255);
	sf::Glsl::Vec3 voidCol = sf::Glsl::Vec3(0.001, 0.005, 0.01);
	sf::Glsl::Vec3 sunCol = sf::Glsl::Vec3(1000.0, 900.0, 600.0);
	sf::Glsl::Vec3 sunDir = sf::Glsl::Vec3(4, 5, 2).normalized();
	float sunSize = 0.1f;
	float sunFlare = 128.0f;

	Skybox() {}

	void Bind(sf::Shader& shader)
	{
		peakCol = sf::Glsl::Vec3(std::max(0.0f, peakCol.x), std::max(0.0f, peakCol.y), std::max(0.0f, peakCol.z));
		horizonCol = sf::Glsl::Vec3(std::max(0.0f, horizonCol.x), std::max(0.0f, horizonCol.y), std::max(0.0f, horizonCol.z));
		voidCol = sf::Glsl::Vec3(std::max(0.0f, voidCol.x), std::max(0.0f, voidCol.y), std::max(0.0f, voidCol.z));
		sunCol = sf::Glsl::Vec3(std::max(0.0f, sunCol.x), std::max(0.0f, sunCol.y), std::max(0.0f, sunCol.z));
		sunDir = sunDir.normalized();

		shader.setUniform("showSkybox", showSkybox);
		shader.setUniform("peakCol", peakCol);
		shader.setUniform("horizonCol", horizonCol);
		shader.setUniform("voidCol", voidCol);
		shader.setUniform("sunCol", sunCol);
		shader.setUniform("sunDir", sunDir);
		shader.setUniform("sunSize", sunSize);
		shader.setUniform("sunFlare", sunFlare);
	}
};

struct RaytracerData
{
	Cam cam;
	Skybox skybox;

	RaytracerData() {}

	void BindAll(sf::Shader& shader)
	{
		cam.Bind(shader);
		skybox.Bind(shader);
	}

	void BindContinuous(sf::Shader& shader) const
	{
		cam.Bind(shader);
	}
};