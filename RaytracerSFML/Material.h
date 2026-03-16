#pragma once
#include "SFML/Graphics/Shader.hpp"
#include <SFML/Graphics.hpp>

// Mat:     
//      vec4(albedo reflectivity x1, specular reflectivity x1, reflective index x1, unused x1), 
//      vec4(albedo x3, opacity x1), 
//      vec4(specular x3, opacity x1), 
//      vec4(emission x3, opacity x1), 
//      vec4(absorption x3, offset x1)
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
};