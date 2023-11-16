
/*=============================================================================*/
/*                                    UTILS                                    */
/*=============================================================================*/

const float PI = 3.1415926535898;
const float MINVAL = 0.00000001;
const float MAXVAL = 100000000.0;


uniform float rndSeed;

float RandNum(vec2 p)
{
    // We need irrationals for pseudo randomness.
    // Most (all?) known transcendental numbers will (generally) work.
    const vec2 r = vec2(
      23.1406926327792690,  // e^pi (Gelfond's constant)
       2.6651441426902251); // 2^sqrt(2) (Gelfond–Schneider constant)
    
    return fract(cos(mod(
        123456789.0, 
        1e-7 + 256.0 * dot(p, r)
    )));  
}

/*=============================================================================*/
/*                                    UTILS                                    */
/*=============================================================================*/







/*=============================================================================*/
/*                                   SHAPES                                    */
/*=============================================================================*/

bool CheckBounds(vec3 rO, vec3 irD, vec3 bMin, vec3 bMax)
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
#define AABBCOUNT 2
const vec3 aabbShapes[AABBCOUNT * 2] = vec3[AABBCOUNT * 2](
    vec3(0.0, 0.0, 0.0), vec3(8.0, 2.0, 1.0),
    vec3(8.0, 0.0, 3.0), vec3(8.1, 8.0, 3.1)
);
const vec4 aabbColors[AABBCOUNT] = vec4[AABBCOUNT](
    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0)
);

bool RayAABBIntersect(vec3 rO, vec3 rD, int i, out float l, out vec3 p, out vec3 n)
{
    vec3 inv_dir = 1.0 / rD;
    i *= 2;

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

    vec3 bMid = (aabbShapes[i] + aabbShapes[i+1]) / 2.0;
    vec3 bSize = abs(aabbShapes[i+1] - bMid);
    vec3 mToP = p - bMid;
    vec3 eToP = abs(abs(mToP) - bSize);

    if (eToP.x <= eToP.y && eToP.x <= eToP.z)
        n = vec3(sign(mToP.x), 0, 0);
    else if (eToP.y <= eToP.x && eToP.y <= eToP.z)
        n = vec3(0, sign(mToP.y) * sign(rD.y), 0);
    else if (eToP.z <= eToP.x && eToP.z <= eToP.y)
        n = vec3(0, 0, sign(mToP.z) * sign(rD.z));

    if (dot(rD, n) > 0.0)
        n = -n;

    return true;
}
// AABB


// OBB
#define OBBCOUNT 2
const vec3 obbShapes[OBBCOUNT * 5] = vec3[OBBCOUNT * 5](
    vec3(2.5, 1.25, -1.0), vec3(1.0, 0.5, 1.666),
    vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0),

    vec3(2.5, 1.25, 5.0), vec3(2.5, 2.0, 0.1),
    vec3(0.71, 0.71, 0.0), vec3(-0.71, 0.71, 0.0), vec3(0.0, 0.0, 1.0)
);
const vec4 obbColors[OBBCOUNT] = vec4[OBBCOUNT](
    vec4(1.0, 1.0, 0.0, 1.0),
    vec4(1.0, 0.2, 0.2, 1.0)
);

bool RayOBBIntersect(vec3 rO, vec3 rD, int i, out float l, out vec3 p, out vec3 n)
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
#define SPHERECOUNT 2
const vec4 sphereShapes[SPHERECOUNT] = vec4[SPHERECOUNT](
    vec4(0.0, 1.0, 2.0, 1.4),
    vec4(1.0, 1.5, 2.5, 0.75)
);
const vec4 sphereColors[SPHERECOUNT] = vec4[SPHERECOUNT](
    vec4(1.0, 0.0, 1.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0)
);

bool RaySphereIntersect(vec3 rO, vec3 rD, int i, out float l, out vec3 p, out vec3 n)
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
#define TRICOUNT 1
const vec3 triShapes[TRICOUNT*3] = vec3[TRICOUNT*3](
    vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.)
);
const vec4 triColors[TRICOUNT] = vec4[TRICOUNT](
    vec4(0.0, 0.0, 0.0, 1.0)
);

bool RayTriIntersect(vec3 rO, vec3 rD, int i, out float l, out vec3 p, out vec3 n)
{
    vec3 edge1 = triShapes[i*3+1] - triShapes[i*3];
    vec3 edge2 = triShapes[i*3+2] - triShapes[i*3];

    // Backface-culling
    vec3 iN = cross(edge1, edge2);
    if (dot(iN, rD) >= 0.0)
        return false;

    vec3 h = cross(rD, edge2);
    float a = dot(edge1, h);

    if (a > -MINVAL && a < MINVAL)
        return false;

    vec3 s = rO - triShapes[i*3];
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
        return true;
    }
    return false;
}
// TRI


// PLANE
#define PLANECOUNT 1
const vec3 planeShapes[PLANECOUNT * 2] = vec3[PLANECOUNT * 2](
    vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0)
);
const vec4 planeColors[PLANECOUNT] = vec4[PLANECOUNT](
    vec4(0.0, 1.0, 0.0, 1.0)
);

bool RayPlaneIntersect(vec3 rO, vec3 rD, int i, out float l, out vec3 p, out vec3 n)
{
    float a = dot(planeShapes[i*2+1], rD);

    if ((a >= 0.0) != (dot(planeShapes[i*2+1], planeShapes[i*2] - rO) >= 0.0))
        return false;

    if (dot(planeShapes[i*2+1], planeShapes[i*2] - rO) >= 0.0)
        return false;

    float t = (dot(planeShapes[i*2+1], planeShapes[i*2]) - dot(planeShapes[i*2+1], rO)) / a;

    l = t;
    p = rO + rD * t;
    n = planeShapes[i*2+1];
    return true;
}
// PLANE

/*=============================================================================*/
/*                                   SHAPES                                    */
/*=============================================================================*/






/*=============================================================================*/
/*                                  RENDERING                                  */
/*=============================================================================*/

uniform vec3 camPos;
uniform vec3 camFwd;
uniform vec3 camUp;
uniform vec3 camRight;

uniform float viewHeight;
uniform float viewWidth;



bool GetFirstHit(vec3 rO, vec3 rD, inout float l, out vec3 p, out vec3 n, out vec4 col)
{
    rO += rD * MINVAL;

    bool hasHit = false;
    float nl;
    vec3 np, nn;

    for (int i = 0; i < AABBCOUNT; i++)
    {
        if (RayAABBIntersect(rO, rD, i, nl, np, nn))
        {
            if (nl < l)
            {
                l = nl; p = np; n = nn;
                col = aabbColors[i];
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
                l = nl; p = np; n = nn;
                col = obbColors[i];
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
                l = nl; p = np; n = nn;
                col = sphereColors[i];
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
                l = nl; p = np; n = nn;
                col = triColors[i];
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
                l = nl; p = np; n = nn;
                col = planeColors[i];
                hasHit = true;
            }
        }
    }

    return hasHit;
}


vec4 Raytrace(in vec3 rO, in vec3 rD, in int bounce)
{
    float l = MAXVAL;
    vec3 p, n;
    vec4 col = vec4(0);
    bool hasHit = GetFirstHit(rO, rD, l, p, n, col);

    if (hasHit) 
        col /= log2(2.0 + l / 16.0);

    return col;
}

/*=============================================================================*/
/*                                  RENDERING                                  */
/*=============================================================================*/





/*=============================================================================*/
/*                                    MAIN                                     */
/*=============================================================================*/

uniform sampler2D lastFrame;
uniform int frameCount;


void main(void)
{
    vec3 botLeftLocal = vec3(-viewWidth / 2.0, -viewHeight / 2.0, 1.0);
    vec4 lFrame = texture2D(lastFrame, vec2(gl_TexCoord[0].x, 1.0 - gl_TexCoord[0].y));

    vec2 uv = vec2(gl_TexCoord[0].x, 1.0 - gl_TexCoord[0].y);
    vec3 dirLocal = botLeftLocal + vec3(viewWidth * uv.x, viewHeight * uv.y, 0.0);
    vec3 pixDir = camRight * dirLocal.x + camUp * dirLocal.y + camFwd * dirLocal.z;
    pixDir = normalize(pixDir);

    vec4 col = Raytrace(camPos, pixDir, 0);
    gl_FragColor = col;

    /*
    float avgWeight = 1.0 / (float(frameCount) + 1.0);
    col = (lFrame * (1.0 - avgWeight)) + (col * avgWeight);

    // multiply it by the color
    gl_FragColor = gl_Color * col;
    */
}

/*=============================================================================*/
/*                                    MAIN                                     */
/*=============================================================================*/