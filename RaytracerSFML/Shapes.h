#pragma once
#include "Material.h"
#include <string>

struct Shape
{
	Material mat;

	virtual int GetMax() const = 0;
	virtual const char* GetShapeName() const = 0;
	virtual void Bind(sf::Shader& shader, int shapeNum) const = 0;
};

struct ShapeAABB : Shape
{
	sf::Glsl::Vec3 min, max; 

	int GetMax() const override { return 16; }
	const char* GetShapeName() const override { return "aabb"; }

	void Bind(sf::Shader& shader, int shapeNum) const override
	{

	}
};

struct ShapeOBB : Shape
{
	sf::Glsl::Vec3 center, halfLength, xAxis, yAxis, zAxis;

	int GetMax() const override { return 16; }
	const char* GetShapeName() const override { return "obb"; }
};

struct ShapeSphere : Shape
{
	sf::Glsl::Vec3 pos;
	float rad;

	int GetMax() const override { return 16; }
	const char* GetShapeName() const override { return "sphere"; }
};

struct ShapeTri : Shape
{
	sf::Glsl::Vec3 v1, v2, v3;

	int GetMax() const override { return 32; }
	const char* GetShapeName() const override { return "tri"; }
};

struct ShapePlane : Shape
{
	sf::Glsl::Vec3 center, normal;

	int GetMax() const override { return 8; }
	const char* GetShapeName() const override { return "plane"; }
};
