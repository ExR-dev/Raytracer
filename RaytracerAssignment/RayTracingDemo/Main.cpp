#include <cstdint>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Ray.h"
#include "Disc.h"
#include "Sphere.h"
#include "OBB.h"
#include "Triangle.h"
#include "Plane.h"

#include <iostream>



struct Cam
{
    float fov;
    bool perspective;

    Vector3D
        origin,
        fwd, right, up;


    Cam(float fov, bool perspective, const Vector3D& origin, const Vector3D& forward) :
        fov(fov), perspective(perspective), origin(origin), 
        fwd(forward), right({0,0,0}), up({0,0,0})
    {
        fwd.Normalize();

        right = fwd ^ Vector3D(0, -1, 0);
        right.Normalize();

        up = fwd ^ right;
        up.Normalize();
    }
};

struct Scene
{
    Shape** shapePtrs;
    int shapeCount;

    Scene(Shape** shapePtrs, int shapeCount) : 
        shapePtrs(shapePtrs), shapeCount(shapeCount)
    {}
};


Vector3D CastRayInScene(const Scene& scene, const Ray& ray)
{
    Shape* closestShape = nullptr;
    double closestHit = std::numeric_limits<double>::max();

    for (int j = 0; j < scene.shapeCount; j++)
    {
        Shape* currShape = scene.shapePtrs[j];

        double tempHit = 0.0;
        if (currShape->Intersection(ray, tempHit))
        {
            if (closestShape != nullptr && tempHit >= closestHit)
                continue;

            closestHit = tempHit;
            closestShape = currShape;
        }
    }

    if (closestShape != nullptr)
    {
        double light = 1.0 / (1.0 + closestHit / 7.5);
        return closestShape->GetColour() * std::min(light, 1.0);
    }
    return {0,0,0};
}



int main()
{
    // Build Scene
    Cam cam(
        //70.0f, true,
        70.0f, false,
        Vector3D(0.0, 0.0, -5.0),
        {0.0, 0.0, 1.0}
    );

    Shape* shapePtrs[] = {
        new Plane(
            Vector3D(1, 1, 1),
            Vector3D(0, 0, 10),
            Vector3D(0, 0, -1)
        ),

        new Sphere(
            Vector3D(1.0, 0.5, 0.5),
            Vector3D(0.0, 0.0, 3.0), 
            1.0
        ),

        new Triangle(
            Vector3D(0.5, 1.0, 0.5),
            Vector3D(2, -0.3, 1),
            Vector3D(1.4, 0.9, 2),
            Vector3D(4.0, 1.4, 9.9)
        ),

        new OBB(
            Vector3D(0.5, 0.5, 1.0),
            Vector3D(0.0, 0.0, 3.0),
            Vector3D(1, 1, 1),
            Vector3D(0, -1, 1),
            Vector3D(-2, 1, 1),
            0.6, 0.7, 0.9
        ),
    };
    int shapeCount = sizeof(shapePtrs) / sizeof(Shape*);

    Scene scene(shapePtrs, shapeCount);


    // Render Scene
    const int channels = 3;
    const unsigned int
        width = 1920,
        height = 1080,
        dim = width * height;

    uint8_t* imageData = new uint8_t[width * height * channels];


    double viewHeight, viewWidth;

    if (cam.perspective)
        viewHeight = tan(((double)cam.fov / 2.0) * (3.14159265 / 180.0)) * 2.0;
    else
        viewHeight = (double)cam.fov / 8.0;
    viewWidth = viewHeight / ((double)height / (double)width);

    Vector3D botLeftLocal(-viewWidth / 2.0, -viewHeight / 2.0, 1.0);

#pragma omp parallel for num_threads(8)
    for (int i = 0; i < dim; i++)
    {
        unsigned int
            x = i % width,
            y = i / width;

        double u, v;
        v = 1.0 - (double)y / (double)height;
        u = (double)x / (double)width;

        Vector3D
            pixPos = cam.origin,
            pixDir = cam.fwd;

        if (cam.perspective)
        {
            Vector3D dirLocal = botLeftLocal + Vector3D(viewWidth * u, viewHeight * v, 0.0);
            pixDir = (cam.right * dirLocal.GetX()) + (cam.up * dirLocal.GetY()) + (cam.fwd * dirLocal.GetZ());
            pixDir.Normalize();
        }
        else
            pixPos = pixPos + cam.right * (viewWidth * (u - 0.5)) + cam.up * (viewHeight * (v - 0.5));

        Ray ray(pixPos, pixDir);
        Vector3D hitSurface = CastRayInScene(scene, ray);

        double 
            rCol = hitSurface.GetX() * hitSurface.GetX() * 255.0, 
            gCol = hitSurface.GetY() * hitSurface.GetY() * 255.0, 
            bCol = hitSurface.GetZ() * hitSurface.GetZ() * 255.0;

        imageData[i * 3 + 0] = (uint8_t)(std::max(0.0, std::min(rCol, 255.0)));
        imageData[i * 3 + 1] = (uint8_t)(std::max(0.0, std::min(gCol, 255.0)));
        imageData[i * 3 + 2] = (uint8_t)(std::max(0.0, std::min(bCol, 255.0)));
    }


    // Save render
	stbi_write_png("Traced.png", width, height, channels, imageData, width * channels);


    // Free memory
    for (int i = 0; i < shapeCount; i++)
        delete shapePtrs[i];
	delete[] imageData;
	return 0;
}