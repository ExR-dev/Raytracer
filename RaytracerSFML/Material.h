#pragma once
#include "SFML/Graphics/Shader.hpp"
#include <SFML/Graphics.hpp>
#include <string>

struct Material
{
	const static int len = 5;

	sf::Glsl::Vec4
		albedo, 
		specular, 
		emission, 
		absorption;

	float
		albedoReflectivity,
		specularReflectivity,
		reflectiveIndex;

	Material() = default;

	void Bind(const std::string& shapeName, sf::Shader& shader, int index) const
	{
		shader.setUniform(std::format("{}Mats[{}]", shapeName, index * len), sf::Glsl::Vec4(albedoReflectivity, specularReflectivity, reflectiveIndex, 0.0));
		shader.setUniform(std::format("{}Mats[{}]", shapeName, index * len + 1), albedo);
		shader.setUniform(std::format("{}Mats[{}]", shapeName, index * len + 2), specular);
		shader.setUniform(std::format("{}Mats[{}]", shapeName, index * len + 3), emission);
		shader.setUniform(std::format("{}Mats[{}]", shapeName, index * len + 4), absorption);
	}
};