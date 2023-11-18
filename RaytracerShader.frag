#version 120
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_gpu_shader_fp64 : enable













/*=======================================================================================================*/
/*                                                 UTILS                                                 */
/*=======================================================================================================*/

const float PI = 3.1415926536;
const float MINVAL = 0.000025;
const float MAXVAL = 10000000.0;

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

/*=======================================================================================================*/
/*                                                 UTILS                                                 */
/*=======================================================================================================*/













/*=======================================================================================================*/
/*                                                SHAPES                                                 */
/*=======================================================================================================*/

// make sure to invert irD beforehand
bool CheckBounds(in vec3 rO, in vec3 irD, in vec3 bMin, in vec3 bMax)
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



// AABB
#define AABBCOUNT 1
const vec3 aabbShapes[AABBCOUNT * 2] = vec3[AABBCOUNT * 2](
    vec3(-1.0, 2.0, -1.0), vec3(1.0, 4.0, 1.0)
);
const vec4 aabbMats[AABBCOUNT * 3] = vec4[AABBCOUNT * 3](
    // vec4(color x3, opacity x1), vec4(emission x3, strength x1), vec4(reflectivity x1, refractivity x1, unused x2)
    vec4(1.0, 1.0, 1.0, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(1.0, 0.0, 0.0, 0.0)
);

bool RayAABBIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n)
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
    int sgn = (tmin > 0.0) ? 1 : -1;

    if (l == tx1)
    { 
        p.x = aabbShapes[i].x; 
        n = vec3(-sgn,0,0); 
    }
    else if (l == tx2)
    { 
        p.x = aabbShapes[i+1].x; 
        n = vec3(sgn,0,0); 
    }
    else if (l == ty1)
    { 
        p.y = aabbShapes[i].y; 
        n = vec3(0,-sgn,0); 
    }
    else if (l == ty2)
    { 
        p.y = aabbShapes[i+1].y; 
        n = vec3(0,sgn,0);
    }
    else if (l == tz1)
    { 
        p.z = aabbShapes[i].z; 
        n = vec3(0,0,-sgn); 
    }
    else if (l == tz2)
    { 
        p.z = aabbShapes[i+1].z; 
        n = vec3(0,0,sgn); 
    }
    return true;
}
// AABB


// OBB
#define OBBCOUNT 1
const vec3 obbShapes[OBBCOUNT * 5] = vec3[OBBCOUNT * 5](
    // vec3(center x3), vec3(halfLength x3), 
    // vec3(x-axis x3), vec3(y-axis x3), vec3(z-axis x3)
    vec3(1.0, 4.5, -2.0), vec3(0.75, 0.5, 1.0),
    normalize(vec3(6.0, 4.0, -2.0)), normalize(vec3(-0.01965655, -0.384051845, -0.92310227)), normalize(vec3(-0.5970381, 0.73607948, -0.3189553))
);
const vec4 obbMats[OBBCOUNT * 3] = vec4[OBBCOUNT * 3](
    // vec4(color x3, opacity x1), vec4(emission x3, strength x1), vec4(reflectivity x1, refractivity x1, unused x2)
    vec4(0.1, 1.0, 0.5, 1.0), vec4(0.5, 1.0, 0.7, 0.1), vec4(0.2, 0.0, 0.0, 0.0)
);

bool RayOBBIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n)
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
    }
    else
    {
        l = maxV;
        n = -nMax;
    }

    p = rO + rD * l;
    n = normalize(n);
    return true;
}
// OBB


// SPHERE
#define SPHERECOUNT 1
const vec4 sphereShapes[SPHERECOUNT] = vec4[SPHERECOUNT](
    vec4(2.0, 1.5, 2.5, 1.0)
);
const vec4 sphereMats[SPHERECOUNT * 3] = vec4[SPHERECOUNT * 3](
    // vec4(color x3, opacity x1), vec4(emission x3, strength x1), vec4(reflectivity x1, refractivity x1, unused x2)
    vec4(1.0, 1.0, 1.0, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(1.0, 0.0, 0.0, 0.0)
);

bool RaySphereIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n)
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
        t0 = t1;
        if (t0 < 0.0)
            return false;
    }

    l = t0;
    p = rO + rD * l;
    n = (p - sphereShapes[i].xyz) / sphereShapes[i].w;

    if (dot(n, rD) > 0.0)
        n *= -1.0;
    return true;
}
// SPHERE


// TRI
#define TRICOUNT 14
const vec3 triShapes[TRICOUNT * 3] = vec3[TRICOUNT * 3](
    vec3(-3.5, 0.0, -4.5), vec3(-3.5, 10.0, -4.5), vec3(-3.5, 0.0, 4.5),
    vec3(-3.5, 0.0, 4.5), vec3(-3.5, 10.0, -4.5), vec3(-3.5, 10.0, 4.5),

    vec3(3.5, 0.0, 4.5), vec3(3.5, 10.0, 4.5), vec3(3.5, 0.0, -4.5),
    vec3(3.5, 0.0, -4.5), vec3(3.5, 10.0, 4.5), vec3(3.5, 10.0, -4.5),


    vec3(-3.5, 0.0, -4.5), vec3(-3.5, 0.0, 4.5), vec3(3.5, 0.0, -4.5),
    vec3(3.5, 0.0, -4.5), vec3(-3.5, 0.0, 4.5), vec3(3.5, 0.0, 4.5),

    vec3(-3.5, 10.0, 4.5), vec3(-3.5, 10.0, -4.5), vec3(3.5, 10.0, 4.5),
    vec3(3.5, 10.0, 4.5), vec3(-3.5, 10.0, -4.5), vec3(3.5, 10.0, -4.5),


    vec3(3.5, 0.0, -4.5), vec3(3.5, 10.0, -4.5), vec3(-3.5, 0.0, -4.5),
    vec3(-3.5, 0.0, -4.5), vec3(3.5, 10.0, -4.5), vec3(-3.5, 10.0, -4.5),

    vec3(-3.5, 0.0, 4.5), vec3(-3.5, 10.0, 4.5), vec3(3.5, 0.0, 4.5),
    vec3(3.5, 0.0, 4.5), vec3(-3.5, 10.0, 4.5), vec3(3.5, 10.0, 4.5),

    
    vec3(-1.5, 9.99, 2.5), vec3(-1.5, 9.99, -2.5), vec3(1.5, 9.99, 2.5),
    vec3(1.5, 9.99, 2.5), vec3(-1.5, 9.99, -2.5), vec3(1.5, 9.99, -2.5)
);
const vec4 triMats[TRICOUNT * 3] = vec4[TRICOUNT * 3](
    // vec4(color x3, opacity x1), vec4(emission x3, strength x1), vec4(reflectivity x1, refractivity x1, unused x2)
    vec4(0.85, 0.2, 0.1, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),
    vec4(0.85, 0.2, 0.1, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),

    vec4(0.15, 0.7, 0.85, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),
    vec4(0.15, 0.7, 0.85, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),


    vec4(1.0, 1.0, 1.0, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),
    vec4(1.0, 1.0, 1.0, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),

    vec4(0.2, 0.2, 0.9, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),
    vec4(0.2, 0.2, 0.9, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),


    vec4(0.2, 0.85, 0.1, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),
    vec4(0.2, 0.85, 0.1, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),

    vec4(0.7, 0.15, 0.85, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),
    vec4(0.7, 0.15, 0.85, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0),

    

    vec4(1.0, 1.0, 1.0, 1.0), vec4(1.0, 1.0, 1.0, 2.0), vec4(0.0, 0.0, 0.0, 0.0),
    vec4(1.0, 1.0, 1.0, 1.0), vec4(1.0, 1.0, 1.0, 2.0), vec4(0.0, 0.0, 0.0, 0.0)
);

bool RayTriIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n)
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

        // If no backface-culling
        /*if (dot(n, rD) > 0.0)
            n *= -1.0;*/
        return true;
    }
    return false;
}
// TRI


// PLANE
#define PLANECOUNT 0
const vec3 planeShapes[2 + PLANECOUNT * 2] = vec3[2 + PLANECOUNT * 2](
    vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0)
);
const vec4 planeMats[3 + PLANECOUNT * 3] = vec4[3 + PLANECOUNT * 3](
    // vec4(color x3, opacity x1), vec4(emission x3, strength x1), vec4(reflectivity x1, refractivity x1, unused x2)
    vec4(0.7, 1.0, 0.5, 1.0), vec4(0.0, 0.0, 0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0)
);

bool RayPlaneIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n)
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



bool GetFirstHit(in vec3 rO, in vec3 rD, inout float l, inout vec3 p, inout vec3 n, out vec4 col, out vec4 emission, out vec4 surface)
{
    //rO += rD * MINVAL;
    rO += n * MINVAL;

    bool hasHit = false;
    float nl;
    vec3 np, nn;

    for (int i = 0; i < AABBCOUNT; i++)
    {
        if (RayAABBIntersect(rO, rD, i, nl, np, nn))
        {
            if (nl < l)
            {
                l = nl; 
                p = np; 
                n = nn;

                col = aabbMats[i*3];
                emission = aabbMats[i*3+1];
                surface = aabbMats[i*3+2];
                hasHit = true;
            }
        }
    }

    for (int i = 0; i < OBBCOUNT; i++)
    {
        if (RayOBBIntersect(rO, rD, i, nl, np, nn))
        {
            if (nl < l)
            {
                l = nl; 
                p = np; 
                n = nn;

                col = obbMats[i*3];
                emission = obbMats[i*3+1];
                surface = obbMats[i*3+2];
                hasHit = true;
            }
        }
    }
    
    for (int i = 0; i < SPHERECOUNT; i++)
    {
        if (RaySphereIntersect(rO, rD, i, nl, np, nn))
        {
            if (nl < l)
            {
                l = nl; 
                p = np; 
                n = nn;

                col = sphereMats[i*3];
                emission = sphereMats[i*3+1];
                surface = sphereMats[i*3+2];
                hasHit = true;
            }
        }
    }
    
    for (int i = 0; i < TRICOUNT; i++)
    {
        if (RayTriIntersect(rO, rD, i, nl, np, nn))
        {
            if (nl < l)
            {
                l = nl; 
                p = np; 
                n = nn;

                col = triMats[i*3];
                emission = triMats[i*3+1];
                surface = triMats[i*3+2];
                hasHit = true;
            }
        }
    }
    
    for (int i = 0; i < PLANECOUNT; i++)
    {
        if (RayPlaneIntersect(rO, rD, i, nl, np, nn))
        {
            if (nl < l)
            {
                l = nl; 
                p = np; 
                n = nn;

                col = planeMats[i*3];
                emission = planeMats[i*3+1];
                surface = planeMats[i*3+2];
                hasHit = true;
            }
        }
    }

    return hasHit;
}


vec3 Raytrace(in vec3 rO, in vec3 rD, inout uint seed)
{
	vec3 incomingLight = vec3(0);
	vec3 rayColour = vec3(1);

    for (int i = 0; i <= maxBounces; i++)
    {
        //rO += rD * MINVAL;
        
        float l = MAXVAL;
        vec3 p, n;

        vec4 color = vec4(0);
        vec4 emission = vec4(0);
        vec4 surface = vec4(0);

        if (GetFirstHit(rO, rD, l, p, n, color, emission, surface))
        {
            if (disableLighting /*&& i == 1*/)
                return color.xyz + emission.xyz * emission.w;

            rO = p;
			vec3 diffuseDir = normalize(n + RandDir(seed));
			vec3 specularDir = reflect(rD, n);
			rD = normalize(Lerp(diffuseDir, specularDir, surface.x));

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
        {
			//incomingLight += rD * rayColour;
		    //incomingLight += vec3(0.1) * rayColour;
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
        uv.y += ((RandomValue(seed) - 0.5)) / float(imgH);
        uv.x += ((RandomValue(seed) - 0.5)) / float(imgW);
    }
    
    vec3 botLeftLocal = vec3(-viewWidth / 2.0, -viewHeight / 2.0, 1.0);
    vec3 dirLocal = botLeftLocal + vec3(viewWidth * uv.x, viewHeight * uv.y, 0.0);
    vec3 pixDir = camRight * dirLocal.x + camUp * dirLocal.y + camFwd * dirLocal.z;
    pixDir = normalize(pixDir);

    for (int i = 0; i < samples; i++)
        outCol += Raytrace(camPos, pixDir, seed);
    outCol /= samples;

    float avgWeight = 1.0 / (float(frameCount + 1));
    outCol = (lFrame * (1.0 - avgWeight)) + (outCol * avgWeight);
    gl_FragColor = vec4(outCol, 1);
}

/*=======================================================================================================*/
/*                                                 MAIN                                                  */
/*=======================================================================================================*/













