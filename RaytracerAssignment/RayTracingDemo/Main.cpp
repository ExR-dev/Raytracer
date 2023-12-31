
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Ray.h"
#include "Disc.h"
#include "Sphere.h"
#include "OBB.h"
#include "Triangle.h"
#include "Plane.h"

#include <iostream>
#include <cstdint>


struct Cam
{
    float fov;

    Vector3D
        origin,
        fwd, right, up;


    Cam(float fov, const Vector3D& origin, const Vector3D& forward) :
        fov(fov), origin(origin), 
        fwd(forward), right({0,0,0}), up({0,0,0})
    {
        fwd.Normalize();

        // Calculate right & up directions based on given forward direction
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
                continue; // Currently closest intersection is still closer

            closestHit = tempHit;
            closestShape = currShape;
        }
    }

    if (closestShape != nullptr)
    {
        double light = 1.0 / (1.0 + closestHit / 8.0); // Apply light based on distance
        return closestShape->GetColour() * std::min(light, 1.0);
    }
    return Vector3D(0.0, 0.0, 0.0); // No hit, return ambient light
}



int main()
{
    // Build Scene
    Cam cam(
        70.0f, // fov
        Vector3D(0.0, 0.0, -5.0), // origin
        Vector3D(0.0, 0.0, 1.0) // direction
    );

    // Collection of all shapes in the scene
    Shape* shapePtrs[] = {
        new Plane(
            Vector3D(0.0, 0.1, 0.4),
            Vector3D(0, 0, 10),
            Vector3D(0, 0, -1)
        ),

        new OBB( // Blue box in center
            Vector3D(0.5, 0.5, 1.0),
            Vector3D(0.0, 0.0, 3.0),
            Vector3D(1, 1, 1),
            Vector3D(0, -1, 1),
            Vector3D(-2, 1, 1),
            0.6, 0.7, 0.9
        ),

        new Sphere( // Red sphere in center
            Vector3D(1.0, 0.5, 0.5),
            Vector3D(0.0, 0.0, 3.0),
            1.0
        ),

        new Triangle( // Green triangle
            Vector3D(0.5, 1.0, 0.5),
            Vector3D(2, -0.3, 1),
            Vector3D(1.4, 0.9, 2),
            Vector3D(4.0, 1.4, 9.9)
        ),

        new Triangle( // Wine-red triangle
            Vector3D(0.75, 0.25, 0.5),
            Vector3D(2.0, 2.0, 5.5),
            Vector3D(3.3, -0.9, 2.0),
            Vector3D(2.25, -1.2, 1.0)
        ),


        new OBB( // Physically Inaccurate Sun
            Vector3D(5.75, 5.0, 1.0),
            Vector3D(-3.0, 3.0, 10.0),
            Vector3D(1, 0, 0),
            Vector3D(0, 1, 0),
            Vector3D(0, 0, 1),
            0.3, 0.3, 0.3
        ),
        new Sphere( // Moon
            Vector3D(1.5, 1.5, 1.35),
            Vector3D(-6.0, 1.0, 8.0), 
            0.25
        ),
        new Sphere( // Water
            Vector3D(0.1, 0.4, 1.0),
            Vector3D(-4.0, 0.0, 5.0), 
            1.0
        ),
        new Sphere( // North pole
            Vector3D(0.95, 1.0, 1.1),
            Vector3D(-4.0, 0.03, 5.0),
            0.974
        ),
        new Sphere( // South pole
            Vector3D(0.95, 1.0, 1.1),
            Vector3D(-4.0, -0.035, 5.0),
            0.9745
        ),
        new Sphere( // North America
            Vector3D(0.4, 1.0, 0.2),
            Vector3D(-4.03, 0.0165, 4.975),
            0.9595
        ),
        new Sphere( // Practically Brazil
            Vector3D(0.4, 1.0, 0.2),
            Vector3D(-4.029, -0.0075, 4.975),
            0.962
        ),
        new Sphere( // South South America
            Vector3D(0.4, 1.0, 0.2),
            Vector3D(-4.032, -0.0178, 4.975),
            0.95625
        ),
        new Sphere( // Europe
            Vector3D(0.375, 0.95, 0.2),
            Vector3D(-3.9975, 0.016, 4.97),
            0.9678
        ),
        new Sphere( // Asia
            Vector3D(0.4, 1.0, 0.2),
            Vector3D(-3.977, 0.015, 4.975),
            0.967
        ),
        new Sphere( // Mid Africa
            Vector3D(1.0, 0.95, 0.45),
            Vector3D(-3.9975, -0.005, 4.9675),
            0.9678
        ),
        new Sphere( // West Africa
            Vector3D(1.0, 0.95, 0.45),
            Vector3D(-4.006, -0.004, 4.9675),
            0.9672
        ),
        new Sphere( // South Africa
            Vector3D(1.0, 0.95, 0.45),
            Vector3D(-3.9975, -0.014, 4.9675),
            0.965
        ),
        new Sphere( // Oceania
            Vector3D(1.0, 0.9, 0.3),
            Vector3D(-3.97, -0.015, 4.975),
            0.95875
        ),
        new Sphere( // You Are Here
            Vector3D(2.0, 0.0, 0.0),
            Vector3D(-4.125, 0.65, 4.25),
            0.025
        ),
    };
    int shapeCount = sizeof(shapePtrs) / sizeof(Shape*);

    Scene scene(shapePtrs, shapeCount);


    // Render Scene
    const unsigned int 
        channels = 3,
        width = 1920,
        height = 1080,
        dim = width * height;

    uint8_t* pixelBuffer = new uint8_t[width * height * channels];

    double 
        viewHeight = (double)cam.fov / 8.0, 
        viewWidth = viewHeight / ((double)height / (double)width);

    #pragma omp parallel for num_threads(8) // Make it go faster (Vroom vroom)
    for (int i = 0; i < dim; i++)
    {
        unsigned int // Current pixel-coordinates
            x = i % width,
            y = i / width;

        double // 0-1 clamped screen-coordinates
            u = (double)x / (double)width,
            v = 1.0 - (double)y / (double)height;

        Vector3D // Calculate orthographic ray
            pixPos = cam.origin + cam.right * (viewWidth * (u - 0.5)) + cam.up * (viewHeight * (v - 0.5)),
            pixDir = cam.fwd;

        Ray ray(pixPos, pixDir);
        Vector3D hitSurface = CastRayInScene(scene, ray);

        double 
            rCol = hitSurface.GetX() * 255.0, 
            gCol = hitSurface.GetY() * 255.0, 
            bCol = hitSurface.GetZ() * 255.0;

        pixelBuffer[i * 3 + 0] = (uint8_t)(std::max(0.0, std::min(rCol, 255.0)));
        pixelBuffer[i * 3 + 1] = (uint8_t)(std::max(0.0, std::min(gCol, 255.0)));
        pixelBuffer[i * 3 + 2] = (uint8_t)(std::max(0.0, std::min(bCol, 255.0)));
    }


    // Save render
	stbi_write_png("Traced.png", width, height, channels, pixelBuffer, width * channels);


    // Free memory
    for (int i = 0; i < shapeCount; i++)
        delete shapePtrs[i];
	delete[] pixelBuffer;
	return 0;
}