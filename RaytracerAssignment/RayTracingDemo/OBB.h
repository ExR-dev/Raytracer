#pragma once

#include "Shape.h"
#include "Vector3D.h"
#include "Ray.h"

class OBB : public Shape
{
private:
	Vector3D center;

	Vector3D axes[3];
	double halfLengths[3];

public:
	// colour: Shape colour
	// centerPoint: Point at which the OBB center lies
	// u: Directional vector for indicating orientation of the OBB
	// v: Directional vector for indicating orientation of the OBB
	// w: Directional vector for indicating orientation of the OBB
	// width: Distance from center to side in the u-axis of the OBB
	// height: Distance from center to side in the v-axis of the OBB 
	// depth: Distance from center to side in the w-axis of the OBB
	OBB(const Vector3D& colour, const Vector3D& centerPoint, const Vector3D& u, const Vector3D& v, const Vector3D& w, 
		double width, double height, double depth);

	bool Intersection(const Ray& ray, double& t) override;
};