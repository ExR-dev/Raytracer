#pragma once
#include "Material.h"
#include <string>


struct Shape
{
	Material mat;

	virtual const char* Name() const = 0;
};

struct ShapeAABB : Shape
{
	sf::Glsl::Vec3 
		min = sf::Glsl::Vec3(-1, -1, -1),
		max = sf::Glsl::Vec3(1, 1, 1);

	static size_t GetMax() { return 16; }
	static const char* GetShapeName() { return "aabb"; }

	const char* Name() const override { return GetShapeName(); }
};

struct ShapeOBB : Shape
{
	sf::Glsl::Vec3 
		center = sf::Glsl::Vec3(0, 0, 0),
		halfLength = sf::Glsl::Vec3(1, 1, 1),
		xAxis = sf::Glsl::Vec3(1, 0, 0),
		yAxis = sf::Glsl::Vec3(0, 1, 0),
		zAxis = sf::Glsl::Vec3(0, 0, 1);

	static size_t GetMax() { return 16; }
	static const char* GetShapeName() { return "obb"; }

	const char* Name() const override { return GetShapeName(); }
};

struct ShapeSphere : Shape
{
	sf::Glsl::Vec3 pos = sf::Glsl::Vec3(0, 0, 0);
	float rad = 1;

	static size_t GetMax() { return 16; }
	static const char* GetShapeName() { return "sphere"; }

	const char* Name() const override { return GetShapeName(); }
};

struct ShapeTri : Shape
{
	sf::Glsl::Vec3 
		v1 = sf::Glsl::Vec3(0, 0, 0),
		v2 = sf::Glsl::Vec3(0, 1, 0),
		v3 = sf::Glsl::Vec3(1, 0, 0);

	static size_t GetMax() { return 32; }
	static const char* GetShapeName() { return "tri"; }

	const char* Name() const override { return GetShapeName(); }
};

struct ShapePlane : Shape
{
	sf::Glsl::Vec3 
		center = sf::Glsl::Vec3(0, 0, 0),
		normal = sf::Glsl::Vec3(0, 1, 0);

	static size_t GetMax() { return 8; }
	static const char* GetShapeName() { return "plane"; }

	const char* Name() const override { return GetShapeName(); }
};


void BindShapes(const std::vector<Shape*>& shapes, sf::Shader& shader)
{
	// Separate shapes by type
	std::vector<ShapeAABB*> aabbs;
	std::vector<ShapeOBB*> obbs;
	std::vector<ShapeSphere*> spheres;
	std::vector<ShapeTri*> tris;
	std::vector<ShapePlane*> planes;

	for (Shape* shape : shapes)
	{
		if (auto aabb = dynamic_cast<ShapeAABB*>(shape))
			aabbs.push_back(aabb);
		else if (auto obb = dynamic_cast<ShapeOBB*>(shape))
			obbs.push_back(obb);
		else if (auto sphere = dynamic_cast<ShapeSphere*>(shape))
			spheres.push_back(sphere);
		else if (auto tri = dynamic_cast<ShapeTri*>(shape))
			tris.push_back(tri);
		else if (auto plane = dynamic_cast<ShapePlane*>(shape))
			planes.push_back(plane);
	}

	int aabbCount = std::min(aabbs.size(), ShapeAABB::GetMax());
	int obbCount = std::min(obbs.size(), ShapeOBB::GetMax());
	int sphereCount = std::min(spheres.size(), ShapeSphere::GetMax());
	int triCount = std::min(tris.size(), ShapeTri::GetMax());
	int planeCount = std::min(planes.size(), ShapePlane::GetMax());

	// Bind each shape type to the shader
	for (size_t i = 0; i < aabbCount; ++i)
	{
		const std::string shapeName = ShapeAABB::GetShapeName();
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 2), aabbs[i]->min);
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 2 + 1), aabbs[i]->max);

		aabbs[i]->mat.Bind(shapeName, shader, i);
	}

	for (size_t i = 0; i < obbCount; ++i)
	{
		const std::string shapeName = ShapeOBB::GetShapeName();
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 5), obbs[i]->center);
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 5 + 1), obbs[i]->halfLength);
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 5 + 2), obbs[i]->xAxis);
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 5 + 3), obbs[i]->yAxis);
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 5 + 4), obbs[i]->zAxis);

		obbs[i]->mat.Bind(shapeName, shader, i);
	}

	for (size_t i = 0; i < sphereCount; ++i)
	{
		const std::string shapeName = ShapeSphere::GetShapeName();
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i), sf::Glsl::Vec4(spheres[i]->pos.x, spheres[i]->pos.y, spheres[i]->pos.z, spheres[i]->rad));

		spheres[i]->mat.Bind(shapeName, shader, i);
	}

	for (size_t i = 0; i < triCount; ++i)
	{
		const std::string shapeName = ShapeTri::GetShapeName();
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 3), tris[i]->v1);
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 3 + 1), tris[i]->v2);
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 3 + 2), tris[i]->v3);

		tris[i]->mat.Bind(shapeName, shader, i);
	}

	for (size_t i = 0; i < planeCount; ++i)
	{
		const std::string shapeName = ShapePlane::GetShapeName();
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 2), planes[i]->center);
		shader.setUniform(std::format("{}Shapes[{}]", shapeName, i * 2 + 1), planes[i]->normal);

		planes[i]->mat.Bind(shapeName, shader, i);
	}

	shader.setUniform("aabbBounds[0]", sf::Glsl::Vec4(0, 0, 0, -1));
	shader.setUniform("obbBounds[0]", sf::Glsl::Vec4(0, 0, 0, -1));
	shader.setUniform("sphereBounds[0]", sf::Glsl::Vec4(0, 0, 0, -1));
	shader.setUniform("triBounds[0]", sf::Glsl::Vec4(0, 0, 0, -1));

	shader.setUniform("aabbBoundCoverage[0]", aabbCount);
	shader.setUniform("obbBoundCoverage[0]", obbCount);
	shader.setUniform("sphereBoundCoverage[0]", sphereCount);
	shader.setUniform("triBoundCoverage[0]", triCount);

	shader.setUniform("aabbCount", aabbCount);
	shader.setUniform("obbCount", obbCount);
	shader.setUniform("sphereCount", sphereCount);
	shader.setUniform("triCount", triCount);
	shader.setUniform("planeCount", planeCount);
}