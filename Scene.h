#pragma once

#include "Utils.h"
#include "Vec3.h"
#include "Phys.h"
#include "Graphics.h"
#include "Shapes.h"
#include "Lights.h"



struct Cam
{
    float fov;
    bool perspective;

    Vec3
        origin,
        fwd, right, up;


    Cam(float fov, bool perspective, const Vec3& origin, const Vec3& fwd) :
        fov(fov), perspective(perspective), origin(origin), fwd(fwd)
    {
        UpdateRotation();
    }

    void UpdateRotation()
    {
        fwd.Normalize();

        right = fwd.Cross({0, -1, 0});
        right.Normalize();

        up = fwd.Cross(right);
        up.Normalize();
    }
};

struct Scene
{
    bool disableLighting;
    int lightingType;

    int maxBounces;
    int raySplits;

    Color skyCol;

    Light** lightPtrs;
    int lightCount;

    Shape** shapePtrs;
    int shapeCount;

    Scene(bool disableLighting, int lightingType, int maxBounces, int raySplits, Color skyCol) :
        disableLighting(disableLighting), lightingType(lightingType),
        maxBounces(maxBounces), raySplits(raySplits),
        skyCol(skyCol),
        lightPtrs(nullptr), lightCount(0),
        shapePtrs(nullptr), shapeCount(0)
    {}
};

struct SurfaceHitInfo
{
    Color surfaceColor;
    Color surfaceEmission;
    Color cumulativeLight;

    SurfaceHitInfo(Color surfaceColor, Color surfaceEmission, Color cumulativeLight) :
        surfaceColor(surfaceColor), surfaceEmission(surfaceEmission), cumulativeLight(cumulativeLight)
    {}
};


SurfaceHitInfo CastRayInScene(Scene& scene, Ray ray, Hit& hit, int bounce = 0, double refractIndex = riAir)
{
    SurfaceHitInfo surface = SurfaceHitInfo(
        Color(),
        Color(),
        Color()
    );

    if (bounce > 0)
        ray.origin += ray.dir * MINVAL;

    Hit bestHit = {};
    double len = 0.0;
    bool hasHitSomething = false;

    for (int j = 0; j < scene.shapeCount; j++)
    {
        Shape* currShape = scene.shapePtrs[j];

        Hit tempHit = {};
        if (currShape->RayIntersect(ray, &tempHit))
        {
            if (hasHitSomething && tempHit.len >= len)
                continue;

            bestHit = tempHit;
            len = tempHit.len;
            hasHitSomething = true;
        }
    }

    if (hasHitSomething)
    {
        Shape* hitShape = (Shape*)bestHit.target;

        surface.surfaceColor = hitShape->mat.col;
        surface.surfaceEmission = hitShape->mat.emissionCol * hitShape->mat.emissionStr;

        if (scene.disableLighting)
        {
            surface.cumulativeLight = Color(1, 1, 1);
            goto endHit;
        }

        if (scene.lightingType != 2)
        {
            for (int j = 0; j < scene.lightCount; j++)
            {
                Light* currLight = scene.lightPtrs[j];
                Vec3 dirToLight = currLight->GetRelativePos(bestHit.origin);
                dirToLight.Normalize();

                if (bestHit.normal.Dot(dirToLight) <= 0)
                    continue;

                double distSqr = currLight->GetDistSqr(bestHit.origin);

                Ray lightRay(bestHit.origin, dirToLight);
                Hit lightHit = {};

                bool isBlocked = false;
                for (int k = 0; k < scene.shapeCount; k++)
                {
                    Shape* currShape = scene.shapePtrs[k];

                    if (currShape->RayIntersect(lightRay, &lightHit))
                        if ((lightHit.len * lightHit.len) < distSqr)
                            isBlocked = true;

                    if (isBlocked)
                        break;
                }

                if (isBlocked)
                    continue;

                surface.cumulativeLight += currLight->col *
                    currLight->GetIntensity(lightRay, bestHit.normal);
            }
        }

        if (scene.lightingType != 0 && bounce <= scene.maxBounces)
        {
            for (int i = 0; i <= scene.raySplits; i++)
            {
                Color totCol = Color(0,0,0);

                Vec3 randDir = bestHit.normal + RandDir();
                randDir.Normalize();

                Vec3 reflectDir = ray.dir.Reflect(bestHit.normal);

                Vec3 bounceDir = randDir.VLerp(reflectDir, hitShape->mat.reflectivity);
                Ray bounceRay(bestHit.origin, bounceDir);
                Hit bounceHit = {0.0, Vec3(), Vec3(), bestHit.target};

                SurfaceHitInfo bounceSurface = CastRayInScene(scene, bounceRay, bounceHit, bounce + 1, refractIndex);
                Color bounceCol = (bounceSurface.surfaceColor * bounceSurface.cumulativeLight) + bounceSurface.surfaceEmission;

                totCol += bounceCol * hitShape->mat.opacity;

                if (hitShape->mat.opacity < 1.0)
                {
                    double n1 = refractIndex;
                    double n2 = hitShape->mat.refractIndex;

                    if (ray.dir.Dot(bestHit.normal) > 0.0)
                    {
                        n2 = refractIndex;
                    }


                    double newRefractIndex = hitShape->mat.refractIndex;
                    
                    if (refractIndex != riAir && newRefractIndex != riAir)
                    {
                        newRefractIndex  += refractIndex - 1.0;
                    }


                    Vec3 refractDir = ray.dir.Refract(bestHit.normal * -1, n1, n2); // Do Snell's law
                    Ray refractRay(bestHit.origin, refractDir);
                    Hit refractHit = {0.0, Vec3(), Vec3(), bestHit.target};

                    SurfaceHitInfo refractSurface = CastRayInScene(scene, refractRay, refractHit, bounce + 1, newRefractIndex);
                    Color refractCol = (bounceSurface.surfaceColor * bounceSurface.cumulativeLight) + bounceSurface.surfaceEmission;

                    totCol += refractCol * (1.0 - hitShape->mat.opacity);
                }

                totCol /= (double)(scene.raySplits + 1);
                surface.cumulativeLight += totCol;
            }
        }
    }
    else
    {
        if (bounce == 0)
            surface.surfaceEmission = scene.skyCol;
        else
            surface.surfaceEmission = scene.skyCol * hit.normal.Dot(ray.dir * -1.0);
    }

endHit:
    hit = bestHit;

    surface.surfaceColor.Max();
    surface.surfaceEmission.Max();
    surface.cumulativeLight.Max();
    return surface;
}
