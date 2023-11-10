#pragma once

#include "Utils.h"
#include "Vec3.h"
#include "Phys.h"
#include "Graphics.h"
#include "Shapes.h"
#include "Lights.h"

#include <algorithm>
#include <cstdlib>
#include <vector>



struct SurfaceHitInfo
{
    Color surfaceColor;
    Color surfaceEmission;
    Color cumulativeLight;

    SurfaceHitInfo(Color surfaceColor, Color surfaceEmission, Color cumulativeLight) :
        surfaceColor(surfaceColor), surfaceEmission(surfaceEmission), cumulativeLight(cumulativeLight)
    {}
};

struct Refraction
{
    double ri;
    void* shape;

    bool operator==(const Refraction r) const
    {
        return (ri == r.ri && shape == r.shape);
    }
    bool operator!=(const Refraction r) const
    {
        return !(*this == r);
    }
};

struct Skybox
{
    Color horizonCol;
    double horizonStr;

    Color peakCol;
    double peakStr;

    Color sunCol;
    double sunStr;

    Vec3 sunDir;
    double sunWidth, sunFlare;


    Skybox(Color horizonCol, double horizonStr, 
           Color peakCol, double peakStr, 
           Color sunCol, double sunStr, 
           Vec3 sunDir, double sunWidth, double sunFlare) :
        horizonCol(horizonCol), horizonStr(horizonStr),
        peakCol(peakCol), peakStr(peakStr),
        sunCol(sunCol), sunStr(sunStr),
        sunDir(sunDir), sunWidth(0), sunFlare(0)
    {
        this->sunDir.Normalize();
        this->sunWidth = sunWidth * sunWidth;
        this->sunFlare = sunFlare * sunFlare;
    }


    Color Sample(Vec3 dir, bool applyStr)
    {
        Color result = horizonCol * (applyStr ? horizonStr : 1.0);
        result = result.Lerp(
            peakCol * (applyStr ? peakStr : 1.0),
            std::clamp(dir.Dot(Vec3(0.0, 1.0, 0.0)), 0.0, 1.0)
        );

        double sunOffset = dir.Dot(-sunDir) * 0.5 + 0.5 - (sunWidth);
        sunOffset /= 1.0 - sunWidth;
        sunOffset /= sunFlare;
        sunOffset = 1.0 - sunOffset;

        if (sunOffset > 1.0)
            return sunCol * (applyStr ? sunStr : 1.0);
        if (sunOffset < 0.0)
            return result;

        sunOffset = pow(exp2(sunOffset * 0.95) - 1.0, 3.0);

        result = result.Lerp(
            sunCol * (applyStr ? sunStr : 1.0),
            std::clamp(sunOffset, 0.0, 1.0)
        );

        return result;
    }
};


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
    Skybox* sky;

    Light** lightPtrs;
    int lightCount;

    Shape** shapePtrs;
    int shapeCount;

    bool disableLighting;
    int lightingType;

    int maxBounces;
    int raySplits;

    Scene(Skybox* sky, bool disableLighting, int lightingType, int maxBounces, int raySplits) :
        sky(sky),
        lightPtrs(nullptr), lightCount(0),
        shapePtrs(nullptr), shapeCount(0),
        disableLighting(disableLighting), lightingType(lightingType),
        maxBounces(maxBounces), raySplits(raySplits)
    {}

    SurfaceHitInfo CastRay(Ray ray, Hit& hit, std::vector<Refraction> riQueue, int bounce = 0)
    {
        SurfaceHitInfo surface = SurfaceHitInfo(
            Color(),
            Color(),
            Color()
        );

        if (bounce > 0)
            ray.origin += ray.dir * utils::MINVAL;

        Hit bestHit = {};
        double len = 0.0;
        bool hasHitSomething = false;

        for (int j = 0; j < shapeCount; j++)
        {
            Shape* currShape = shapePtrs[j];

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

            if (disableLighting)
            {
                surface.cumulativeLight = Color(1, 1, 1);
                goto endHit;
            }

            if (lightingType != 2)
            {
                for (int j = 0; j < lightCount; j++)
                {
                    Light* currLight = lightPtrs[j];
                    Vec3 dirToLight = currLight->GetRelativePos(bestHit.origin);
                    dirToLight.Normalize();

                    if (bestHit.normal.Dot(dirToLight) <= 0)
                        continue;

                    double distSqr = currLight->GetDistSqr(bestHit.origin);

                    Ray lightRay(bestHit.origin, dirToLight);
                    Hit lightHit = {};

                    bool isBlocked = false;
                    for (int k = 0; k < shapeCount; k++)
                    {
                        Shape* currShape = shapePtrs[k];

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

            if (lightingType != 0 && bounce <= maxBounces)
            {
                for (int i = 0; i <= raySplits; i++)
                {
                    Color totCol = Color(0.0, 0.0, 0.0);
                    double opacity = hitShape->mat.opacity;

                    if (opacity > 0)
                    {
                        Vec3 randDir = bestHit.normal + RandDir();

                        Vec3 reflectDir = ray.dir.Reflect(bestHit.normal);

                        Vec3 bounceDir = randDir.Lerp(reflectDir, hitShape->mat.reflectivity);
                        Ray bounceRay(bestHit.origin, bounceDir);
                        Hit bounceHit = Hit(bestHit);

                        SurfaceHitInfo bounceSurface =
                            CastRay(
                                bounceRay,
                                bounceHit,
                                riQueue,
                                bounce + ((opacity <= 0.5) ? 2 : 1)
                            );

                        Color bounceCol = (bounceSurface.surfaceColor * bounceSurface.cumulativeLight) + bounceSurface.surfaceEmission;
                        totCol += bounceCol * opacity;
                    }

                    if (opacity < 1)
                    {
                        double rayToNormalDot = ray.dir.Dot(bestHit.normal);

                        double n1 = riQueue.back().ri;
                        double n2 = hitShape->mat.refractIndex;

                        if (rayToNormalDot > 0.0)
                        {
                            Refraction ref = {n2, hitShape};
                            if (std::find(riQueue.begin(), riQueue.end(), ref) != riQueue.end())
                                riQueue.erase(std::remove(riQueue.begin(), riQueue.end(), ref), riQueue.end());
                            n2 = riQueue.back().ri;
                        }
                        else
                        {
                            riQueue.push_back({n2, hitShape});
                        }

                        Vec3 refractDir = ray.dir.Refract(bestHit.normal * -1.0, n1, n2); // Snell's law
                        Ray refractRay(bestHit.origin + refractDir * utils::MINVAL, refractDir);
                        Hit refractHit = Hit(bestHit);

                        SurfaceHitInfo refractSurface =
                            CastRay(
                                refractRay,
                                refractHit,
                                riQueue,
                                bounce + ((opacity <= 0.5) ? 1 : 2)
                            );

                        Color refractCol = (refractSurface.surfaceColor * refractSurface.cumulativeLight) + refractSurface.surfaceEmission;
                        totCol += refractCol * (1.0 - opacity);
                    }

                    totCol /= (double)(raySplits + 1);
                    surface.cumulativeLight += totCol;
                }
            }
        }
        else
        {
            if (sky != nullptr)
            {
                if (bounce == 0)
                    surface.surfaceEmission = sky->Sample(ray.dir, true);
                else
                    surface.surfaceEmission = sky->Sample(ray.dir, true) * abs(hit.normal.Dot(-ray.dir));
            }
        }

    endHit:
        hit.len = bestHit.len;
        hit.normal = bestHit.normal;
        hit.origin = bestHit.origin;
        hit.target = bestHit.target;

        surface.surfaceColor.Max();
        surface.surfaceEmission.Max();
        surface.cumulativeLight.Max();
        return surface;
    }
};




