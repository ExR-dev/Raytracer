#version 130
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_gpu_shader_fp64 : enable













/*=======================================================================================================*/
/*                                                 UTILS                                                 */
/*=======================================================================================================*/

const float PI = 3.1415926536;
const float MINVAL = 0.000025;
const float MAXVAL = 10000000.0;

const float riVacuum = 1.0;
const float riAir = 1.000293;
const float riWater = 1.333;
const float riGlass = 1.52;
const float riDiamond = 2.417;

uniform int imgW;
uniform int imgH;


uniform int rndSeed;

uint NextRandom(inout uint state)
{
    state = state * 747796405 + 2891336453;
	uint result = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
	result = (result >> 22) ^ result;
	return result;
}

float RandomValue(inout uint state)
{
	return float(NextRandom(state)) / 4294967295.0;
}

// Random value in normal distribution (with mean=0 and sd=1)
float RandomValueNormalDistribution(inout uint state)
{
	float theta = 2.0 * PI * RandomValue(state);
	float rho = sqrt(-2.0 * log(RandomValue(state)));
	return rho * cos(theta);
}

vec3 RandomDirection(inout uint state)
{
	float x = RandomValueNormalDistribution(state);
	float y = RandomValueNormalDistribution(state);
	float z = RandomValueNormalDistribution(state);
	return normalize(vec3(x, y, z));
}

vec3 RandDir(inout uint state)
{
    vec3 v;
    float m = MAXVAL;

    while (true)
    {
        do
        {
            v = (vec3(RandomValue(state), RandomValue(state), RandomValue(state)) - 0.5) * 2.0;
            m = v.x*v.x + v.y*v.y + v.z*v.z;
        }
        while (m > 1.0);

        if (m*m > MINVAL*2.0)
            return v / sqrt(m);
    }
}

vec2 RandomPointInCircle(inout uint state)
{
	float angle = RandomValue(state) * 2.0 * PI;
	vec2 pointOnCircle = vec2(cos(angle), sin(angle));
	return pointOnCircle * sqrt(RandomValue(state));
}

float Lerp(float p0, float p1, float t)
{
    return (1.0 - t) * p0 + t * p1;
}

vec3 Lerp(vec3 p0, vec3 p1, float t)
{
    return (1.0 - t) * p0 + t * p1;
}

vec4 Lerp(vec4 p0, vec4 p1, float t)
{
    return (1.0 - t) * p0 + t * p1;
}

vec3 ACESFilm(vec3 x)
{
    return clamp((x*(2.51*x + 0.03)) / (x*(2.43*x + 0.59) + 0.14), 0.0, 1.0);
}

/*=======================================================================================================*/
/*                                                 UTILS                                                 */
/*=======================================================================================================*/













/*=======================================================================================================*/
/*                                                SHAPES                                                 */
/*=======================================================================================================*/

const int MATVALS = 5;


// Make sure to invert irD beforehand
bool CheckBoundingBox(in vec3 rO, in vec3 irD, in vec3 bMin, in vec3 bMax)
{
    float tx1 = (bMin.x - rO.x) * irD.x;
    float tx2 = (bMax.x - rO.x) * irD.x;

    float tmin = min(tx1, tx2);
    float tmax = max(tx1, tx2);

    float ty1 = (bMin.y - rO.y) * irD.y;
    float ty2 = (bMax.y - rO.y) * irD.y;

    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));

    float tz1 = (bMin.z - rO.z) * irD.z;
    float tz2 = (bMax.z - rO.z) * irD.z;

    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));

    return (tmax >= max(0.0, tmin)) && (tmin < MAXVAL);
}

bool CheckBoundingSphere(in vec3 rO, in vec3 rD, in vec4 s)
{
    vec3 oc = rO - s.xyz;
    float b = dot(oc, rD);

    vec3 qc = oc - rD * b;
    float h = (s.w*s.w) - dot(qc, qc);

    if (h < -MINVAL) 
        return false;

    return b * abs(b) < h;
}






// AABB
const int AABBMAX = 16;
uniform int aabbCount;

uniform vec4 aabbBounds[AABBMAX];
uniform int aabbBoundCoverage[AABBMAX];

uniform vec3 aabbShapes[AABBMAX*2];
uniform vec4 aabbMats[AABBMAX*MATVALS];

bool RayAABBIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n, out int side)
{
    i *= 2;
    vec3 inv_dir = 1.0 / rD;

    float tx1 = (aabbShapes[i].x - rO.x) * inv_dir.x;
    float tx2 = (aabbShapes[i+1].x - rO.x) * inv_dir.x;

    float tmin = min(tx1, tx2);
    float tmax = max(tx1, tx2);

    float ty1 = (aabbShapes[i].y - rO.y) * inv_dir.y;
    float ty2 = (aabbShapes[i+1].y - rO.y) * inv_dir.y;

    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));

    float tz1 = (aabbShapes[i].z - rO.z) * inv_dir.z;
    float tz2 = (aabbShapes[i+1].z - rO.z) * inv_dir.z;

    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));

    if (!((tmax >= max(0.0, tmin)) && (tmin < MAXVAL)))
        return false;

    l = (tmin > 0.0) ? tmin : tmax;

    p = rO + (rD * l);
    side = (tmin > 0.0) ? 1 : -1;

    if (l == tx1)
    { 
        p.x = aabbShapes[i].x; 
        n = vec3(-side,0,0);
    }
    else if (l == tx2)
    { 
        p.x = aabbShapes[i+1].x; 
        n = vec3(side,0,0); 
    }
    else if (l == ty1)
    { 
        p.y = aabbShapes[i].y; 
        n = vec3(0,-side,0); 
    }
    else if (l == ty2)
    { 
        p.y = aabbShapes[i+1].y; 
        n = vec3(0,side,0);
    }
    else if (l == tz1)
    { 
        p.z = aabbShapes[i].z; 
        n = vec3(0,0,-side); 
    }
    else if (l == tz2)
    { 
        p.z = aabbShapes[i+1].z; 
        n = vec3(0,0,side); 
    }

    if (length(n) < 0.5)
    {
        p = rO + rD * 0.1;
        l = 0.1;
        n = vec3(1,1,1); 
    }

    return true;
}
// AABB


// OBB
const int OBBMAX = 16;
uniform int obbCount;

uniform vec4 obbBounds[OBBMAX];
uniform int obbBoundCoverage[OBBMAX];

uniform vec3 obbShapes[OBBMAX*5];
uniform vec4 obbMats[OBBMAX*MATVALS];

bool RayOBBIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n, out int side)
{
    i *= 5;
    float
        minV = -MAXVAL, 
        maxV = MAXVAL;

    vec3
        pl = obbShapes[i] - rO,
        nMin = vec3(0),
        nMax = vec3(0);

    for (int j = 0; j < 3; j++)
    {
        vec3 axis = obbShapes[i+2+j];
        float halfLength = obbShapes[i+1][j];

        float e = dot(axis, pl);
        float f = dot(axis, rD);

        if (abs(f) > MINVAL)
        {
            vec3 tnMin = axis;
            vec3 tnMax = axis * -1.0;

            float
                t0 = (e + halfLength) / f,
                t1 = (e - halfLength) / f;

            if (t0 > t1)
            {
                float temp = t0;
                t0 = t1;
                t1 = temp;
                tnMin = tnMax;
                tnMax = axis;
            }

            if (t0 > minV)
            {
                minV = t0;
                nMin = tnMin;
            }
            if (t1 < maxV)
            {
                maxV = t1;
                nMax = tnMax;
            }
            
            if (minV > maxV)	return false;
            if (maxV < 0.0)	    return false;
        }
        else if (-e - halfLength > 0.0 || -e + halfLength < 0.0)
            return false;
    }

    if (minV > 0.0)
    {
        l = minV;
        n = nMin;
        side = 1;
    }
    else
    {
        l = maxV;
        n = -nMax;
        side = -1;
    }

    p = rO + rD * l;
    n = normalize(n);
    return true;
}
// OBB


// SPHERE
const int SPHEREMAX = 16;
uniform int sphereCount;

uniform vec4 sphereBounds[SPHEREMAX];
uniform int sphereBoundCoverage[SPHEREMAX];

uniform vec4 sphereShapes[SPHEREMAX*1];
uniform vec4 sphereMats[SPHEREMAX*MATVALS];

bool RaySphereIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n, out int side)
{
    vec3 oc = rO - sphereShapes[i].xyz;
    float b = dot(oc, rD);

    vec3 qc = oc - rD * b;
    float h = (sphereShapes[i].w * sphereShapes[i].w) - dot(qc, qc);

    if (h < -MINVAL)
        return false;

    h = sqrt(max(0.0, h));

    float t0 = -b - h;
    float t1 = -b + h;

    if (t0 > t1)
    {
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    if (t0 < 0.0)
    {
        if (t1 < 0.0) return false;
        t0 = t1;
    }

    l = t0;
    p = rO + rD * l;
    n = (p - sphereShapes[i].xyz) / sphereShapes[i].w;
    side = 1;

    if (dot(n, rD) > 0.0)
    {
        n *= -1.0; 
        side = -1;
    }
    return true;
}
// SPHERE


// TRI
const int TRIMAX = 32;
uniform int triCount;

uniform vec4 triBounds[TRIMAX];
uniform int triBoundCoverage[TRIMAX];

uniform vec3 triShapes[TRIMAX*3];
uniform vec4 triMats[TRIMAX*MATVALS];

bool RayTriIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n, out int side)
{
    i *= 3;
    vec3 edge1 = triShapes[i+1] - triShapes[i];
    vec3 edge2 = triShapes[i+2] - triShapes[i];

    // Backface-culling
    vec3 iN = cross(edge1, edge2);
    if (dot(iN, rD) >= 0.0)
        return false;

    vec3 h = cross(rD, edge2);
    float a = dot(edge1, h);

    if (a > -MINVAL && a < MINVAL)
        return false;

    vec3 s = rO - triShapes[i];
    float f = 1.0 / a;
    float u = f * dot(s, h);

    if (u < 0.0 || u > 1.0)
        return false;

    vec3 q = cross(s, edge1);
    float v = f * dot(rD, q);

    if (v < 0.0 || u + v > 1.0)
        return false;

    float t = f * dot(edge2, q);

    if (t > 0.0)
    {
        l = t;
        p = rO + rD * t;
        n = normalize(cross(edge1, edge2));
        side = (dot(n, rD) < 0.0) ? 1 : -1;

        // If no backface-culling
        /*if (dot(n, rD) > 0.0)
            n *= -1.0;*/
        return true;
    }
    return false;
}
// TRI


// PLANE
const int PLANEMAX = 8;
uniform int planeCount;

uniform vec3 planeShapes[PLANEMAX*2];
uniform vec4 planeMats[PLANEMAX*MATVALS];

bool RayPlaneIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n, out int side)
{
    i *= 2;
    float a = dot(planeShapes[i+1], rD);

    if ((a >= 0.0) != (dot(planeShapes[i+1], planeShapes[i] - rO) >= 0.0))
        return false;

    if (dot(planeShapes[i+1], planeShapes[i] - rO) >= 0.0)
        return false;

    float t = (dot(planeShapes[i+1], planeShapes[i]) - dot(planeShapes[i+1], rO)) / a;

    l = t;
    p = rO + rD * t;
    n = planeShapes[i+1];
    side = (dot(n, rD) < 0.0) ? 1 : -1;
    return true;
}
// PLANE

/*=======================================================================================================*/
/*                                                SHAPES                                                 */
/*=======================================================================================================*/













/*=======================================================================================================*/
/*                                               RENDERING                                               */
/*=======================================================================================================*/

uniform int maxBounces;

uniform vec3 camPos;
uniform vec3 camFwd;
uniform vec3 camUp;
uniform vec3 camRight;

uniform float viewHeight;
uniform float viewWidth;

uniform bool disableLighting;
uniform bool viewBounds;



uniform vec4 peakCol = vec4(0.6, 0.7, 0.8, 1.0);
uniform vec4 horizonCol = vec4(0.4, 0.6, 0.7, 1.0);
uniform vec4 voidCol = vec4(0.01, 0.05, 0.2, 1.0);
uniform vec3 sunCol = vec3(1.0, 0.95, 0.2);
uniform vec3 sunDir = normalize(vec3(30, 60, 15));
uniform float sunFlare = 512.0;

vec3 SampleSkybox(in vec3 rD)
{				
	float skyGradientT = pow(smoothstep(0.0, 0.7, rD.y), 0.8);
	float groundToSkyT = smoothstep(-0.06, 0.0, rD.y);
	vec3 skyGradient = Lerp(horizonCol.xyz*horizonCol.w, peakCol.xyz*peakCol.w, skyGradientT);
	float sun = pow(max(0.0, dot(rD, sunDir)), sunFlare) * 256.0;
	// Combine ground, sky, and sun
	vec3 composite = Lerp(voidCol.xyz*voidCol.w, skyGradient, groundToSkyT) + sunCol * sun * float(groundToSkyT >= 1);
	return composite;
}

float FresnelReflectAmount(vec3 dir, vec3 normal, float reflectivity, float n1, float n2)
{
        // Schlick aproximation
        float r0 = (n1 - n2) / (n1 + n2);
        r0 *= r0;
        float cosX = -dot(normal, dir);
        if (n1 > n2)
        {
            float n = n1 / n2;
            float sinT2 = (n * n) * (1.0 - cosX * cosX);
            // Total internal reflection
            if (sinT2 > 1.0)
                return 1.0;
            cosX = sqrt(1.0 - sinT2);
        }
        float x = 1.0 - cosX;
        float ret = r0 + (1.0 - r0) * (x*x*x*x*x);
 
        // adjust reflect multiplier for object reflectivity
        ret = (reflectivity + (1.0 - reflectivity) * ret);
        return ret;
}



bool GetFirstHit(
    in vec3 rO, in vec3 rD, in bool showBounds,
    inout float l, inout vec3 p, inout vec3 n, inout int s, 
    out vec4 col, out vec4 emission, out vec4 surface, out vec4 specular, out vec4 absorption)
{
    if (dot(rD, n) > 0.0)
        rO += n * MINVAL;
    else
        rO -= n * MINVAL;

    bool hasHit = false;
    float nl;
    vec3 np, nn;
    int ss;
    
    if (showBounds)
    {
        col = vec4(0);
        emission = vec4(0);
        surface = vec4(0);
        specular = vec4(0);
        absorption = vec4(0);
    }

    int boundsID = 0;
    int boundOffset = 0;
    while (boundOffset < aabbCount)
    {
        if (CheckBoundingSphere(rO, rD, aabbBounds[boundsID]))
        {
            if (showBounds)
            {
                l = 1.0;
                p = rD * l; 
                n = -rD;
                s = 1;
                col *= vec4(1.0,0.85,0.85,1.0);
                emission += vec4(0.25,0.0,0.0,0.25);
                hasHit = true;
            }
            else
            {
                for (int i = boundOffset; i < min(boundOffset + aabbBoundCoverage[boundsID], aabbCount); i++)
                {
                    if (RayAABBIntersect(rO, rD, i, nl, np, nn, ss))
                    {
                        if (nl < l)
                        {
                            l = nl; 
                            p = np; 
                            n = nn;
                            s = ss;

                            col = aabbMats[i*MATVALS];
                            emission = aabbMats[i*MATVALS+1];
                            surface = aabbMats[i*MATVALS+2];
                            specular = aabbMats[i*MATVALS+3];
                            absorption = aabbMats[i*MATVALS+4];
                            hasHit = true;
                        }
                    }
                }

            }

            
        }
        boundOffset += aabbBoundCoverage[boundsID++];
    }
    
    boundsID = 0;
    boundOffset = 0;
    while (boundOffset < obbCount)
    {
        if (CheckBoundingSphere(rO, rD, obbBounds[boundsID]))
        {
            if (showBounds)
            {
                l = 1.0;
                p = rD * l; 
                n = -rD;
                s = 1;
                col *= vec4(0.85,1.0,0.85,1.0);
                emission += vec4(0.0,0.25,0.0,0.25);
                hasHit = true;
            }
            else
            {
                for (int i = boundOffset; i < min(boundOffset + obbBoundCoverage[boundsID], obbCount); i++)
                {
                    if (RayOBBIntersect(rO, rD, i, nl, np, nn, ss))
                    {
                        if (nl < l)
                        {
                            l = nl; 
                            p = np; 
                            n = nn;
                            s = ss;

                            col = obbMats[i*MATVALS];
                            emission = obbMats[i*MATVALS+1];
                            surface = obbMats[i*MATVALS+2];
                            specular = obbMats[i*MATVALS+3];
                            absorption = obbMats[i*MATVALS+4];
                            hasHit = true;
                        }
                    }
                }
            }
        }
        boundOffset += obbBoundCoverage[boundsID++];
    }
    
    boundsID = 0;
    boundOffset = 0;
    while (boundOffset < sphereCount)
    {
        if (CheckBoundingSphere(rO, rD, sphereBounds[boundsID]))
        {    
            if (showBounds)
            {
                l = 1.0;
                p = rD * l; 
                n = -rD;
                s = 1;
                col *= vec4(0.85,0.85,1.0,1.0);
                emission += vec4(0.0,0.0,0.25,0.25);
                hasHit = true;
            }
            else
            {
                for (int i = boundOffset; i < min(boundOffset + sphereBoundCoverage[boundsID], sphereCount); i++)
                {
                    if (RaySphereIntersect(rO, rD, i, nl, np, nn, ss))
                    {
                        if (nl < l)
                        {
                            l = nl;
                            p = np; 
                            n = nn;
                            s = ss;

                            col = sphereMats[i*MATVALS];
                            emission = sphereMats[i*MATVALS+1];
                            surface = sphereMats[i*MATVALS+2];
                            specular = sphereMats[i*MATVALS+3];
                            absorption = sphereMats[i*MATVALS+4];
                            hasHit = true;
                        }
                    }
                }
            }
        }
        boundOffset += sphereBoundCoverage[boundsID++];
    }
    
    boundsID = 0;
    boundOffset = 0;
    while (boundOffset < triCount)
    {
        if (CheckBoundingSphere(rO, rD, triBounds[boundsID]))
        {    
            if (showBounds)
            {
                l = 1.0;
                p = rD * l; 
                n = -rD;
                s = 1;
                col *= vec4(1.0,1.0,1.0,1.0);
                emission += vec4(0.15,0.15,0.15,0.25);
                hasHit = true;
            }
            else
            {
                for (int i = boundOffset; i < min(boundOffset + triBoundCoverage[boundsID], triCount); i++)
                {
                    if (RayTriIntersect(rO, rD, i, nl, np, nn, ss))
                    {
                        if (nl < l)
                        {
                            l = nl; 
                            p = np; 
                            n = nn;
                            s = ss;

                            col = triMats[i*MATVALS];
                            emission = triMats[i*MATVALS+1];
                            surface = triMats[i*MATVALS+2];
                            specular = triMats[i*MATVALS+3];
                            absorption = triMats[i*MATVALS+4];
                            hasHit = true;
                        }
                    }
                }
            }
        }
        boundOffset += triBoundCoverage[boundsID++];
    }
    
    for (int i = 0; i < planeCount; i++)
    {
        if (showBounds)
            break;

        if (RayPlaneIntersect(rO, rD, i, nl, np, nn, ss))
        {
            if (nl < l)
            {
                l = nl; 
                p = np; 
                n = nn;
                s = ss;

                col = planeMats[i*MATVALS];
                emission = planeMats[i*MATVALS+1];
                surface = planeMats[i*MATVALS+2];
                specular = planeMats[i*MATVALS+3];
                absorption = planeMats[i*MATVALS+4];

                int tile = (int((abs(p.x) + floor(p.x)) * 2.0) % 2 + int((abs(p.z) + floor(p.z)) * 2.0) % 2);
                col *= (tile % 2 == 0) ? 1.0 : 0.666;
                hasHit = true;
            }
        }
    }

    return hasHit;
}


vec3 Raytrace(in vec3 rO, in vec3 rD, in float ri, inout uint seed)
{
	vec3 incomingLight = vec3(0);
	vec3 rayColour = vec3(1);

	vec4 queuedAbsorption = vec4(0);

    for (int i = 0; i <= maxBounces; i++)
    {
        float l = MAXVAL;
        vec3 p, n;
        int s;

        vec4 color = vec4(0);
        vec4 emission = vec4(0);
        vec4 surface = vec4(0);
        vec4 specular = vec4(0);
        vec4 absorption = vec4(0);

        if (GetFirstHit(rO, rD, false, l, p, n, s, color, emission, surface, specular, absorption))
        {
            if (disableLighting) // && i == 1
                return color.xyz * color.w + emission.xyz * emission.w;

            rayColour *= exp(-queuedAbsorption.xyz * (l + queuedAbsorption.w));

            float 
                ri1 = ri,
                ri2 = surface.y;

            if (s < 0)
            {
                ri1 = ri2;
                ri2 = ri;
            }
                
			if (RandomValue(seed) > color.w)
            {
                bool TIR = false;
                vec3 nrD = refract(rD, n, ri1/ri2);
                
                if (abs(length(nrD) - 1.0) > 0.1)
                {
                    nrD = reflect(rD, n);
                    TIR = true;
                }
                rD = nrD;

                if (s > 0)
                {
                    if (!TIR)
                        queuedAbsorption = absorption;
                }
                else 
                {
                    if (queuedAbsorption != absorption)
                        rayColour *= exp(-absorption.xyz * (l + absorption.w));

                    queuedAbsorption = vec4(0);
                }
            }
            else
            {
                queuedAbsorption = vec4(0);

			    vec3 diffuseDir = normalize(n + RandDir(seed));
			    vec3 specularDir = reflect(rD, n);
                bool isSpecularBounce = specular.w >= RandomValue(seed);
			    rD = normalize(Lerp(diffuseDir, specularDir, surface.x * float(isSpecularBounce)));
			    //rD = normalize(Lerp(diffuseDir, specularDir, FresnelReflectAmount(rD, n, surface.x, ri1, ri2)));

                if (isSpecularBounce)
                    color.xyz = specular.xyz;
            }
            rO = p;

			// Update light calculations
			vec3 emittedLight = emission.xyz * emission.w;
			incomingLight += emittedLight * rayColour;
			rayColour *= color.xyz;
						
			float k = max(rayColour.r, max(rayColour.g, rayColour.b));
			if (RandomValue(seed) >= k)
				break;
			rayColour *= 1.0 / k; 
        }
        else
        { // Ambient
            if (disableLighting)
                return vec3(0);

            vec3 skyLight = SampleSkybox(rD);
			incomingLight += skyLight * rayColour;
			float k = max(rayColour.r, max(rayColour.g, rayColour.b));
			rayColour *= 1.0 / k; 
            break; 
        }
    }

    return incomingLight;
}

/*=======================================================================================================*/
/*                                               RENDERING                                               */
/*=======================================================================================================*/













/*=======================================================================================================*/
/*                                                 MAIN                                                  */
/*=======================================================================================================*/

uniform sampler2D lastFrame;
uniform int frameCount;
uniform int samples;

uniform bool randomizeDir;
uniform int iii;


void main(void)
{
    vec2 uv = vec2(gl_TexCoord[0].x, 1.0 - gl_TexCoord[0].y);
    vec3 lFrame = texture2D(lastFrame, uv).xyz;
    vec3 outCol = vec3(0);
    
    uint rndS = (rndSeed + 2147483647);
    uint seed = rndS + uint(uv.x * imgW) + uint(uv.y * imgW * imgH);

    if (randomizeDir)
    {
        uv.y += ((RandomValue(seed) - 0.5) / 1.5) / float(imgH);
        uv.x += ((RandomValue(seed) - 0.5) / 1.5) / float(imgW);
    }
    
    vec3 botLeftLocal = vec3(-viewWidth / 2.0, -viewHeight / 2.0, 1.0);
    vec3 dirLocal = botLeftLocal + vec3(viewWidth * uv.x, viewHeight * uv.y, 0.0);
    vec3 pixDir = camRight * dirLocal.x + camUp * dirLocal.y + camFwd * dirLocal.z;
    pixDir = normalize(pixDir);

    for (int i = 0; i < samples; i++)
        outCol += Raytrace(camPos, pixDir, riAir, seed);
    outCol /= samples;

    //outCol = clamp(outCol, 0.0, 1.0);
    outCol = ACESFilm(outCol);

    float avgWeight = 1.0 / (float(frameCount + 1));
    outCol = (lFrame * (1.0 - avgWeight)) + (outCol * avgWeight);
    gl_FragColor = vec4(outCol, 1);

    if (viewBounds)
    {
        float l, s;
        vec3 p, n;
        vec4 color, emission, surface, specular, absorption;
        if (GetFirstHit(camPos, pixDir, true, l, p, n, s, color, emission, surface, specular, absorption))
            gl_FragColor.xyz += color.xyz * color.w + emission.xyz * emission.w;
    }
}

/*=======================================================================================================*/
/*                                                 MAIN                                                  */
/*=======================================================================================================*/













