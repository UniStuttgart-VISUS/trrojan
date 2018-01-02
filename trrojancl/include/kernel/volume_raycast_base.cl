#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

constant sampler_t linearSmp = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE |
                                CLK_FILTER_LINEAR;
constant sampler_t nearestSmp = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE |
                                CLK_FILTER_NEAREST;

inline float3 transformPoint3(const float16 m, const float3 x)
{
//    return (float3)(dot(m.s012, x)+m.s3, dot(m.s456, x)+m.s7, dot(m.s89a, x)+m.sb);
    return (float3)(dot(m.s048, x)+m.sc, dot(m.s159, x)+m.sd, dot(m.s26a, x)+m.se);
}

inline float4 transformPoint(const float16 m, const float4 v)
{
//     return (float4)( dot(m.s048c, v), dot(m.s159d, v), dot(m.s26ae, v), dot(m.s37bf, v) );
     return (float4)(dot(m.s0123, v), dot(m.s4567, v), dot(m.s89ab, v), dot(m.scdef, v));
}

// Transform and project
inline float3 transformPointW(const float16 m, const float3 x)
{
    return transformPoint3(m, x)/(dot(m.s37b, x)+m.sf);
}

// Compute gradient using central difference: f' = ( f(x+h)-f(x-h) ) / 2*h
float3 gradientCentralDiff(read_only image3d_t vol, const float4 pos)
{
    float3 volResf = convert_float3(get_image_dim(vol).xyz);
    float3 offset = native_divide((float3)(1.0f), volResf);
    float3 s1;
    float3 s2;
    s1.x = read_imagef(vol, linearSmp, pos + (float4)(-offset.x, 0, 0, 0)).x;
    s2.x = read_imagef(vol, linearSmp, pos + (float4)(+offset.x, 0, 0, 0)).x;
    s1.y = read_imagef(vol, linearSmp, pos + (float4)(0, -offset.y, 0, 0)).x;
    s2.y = read_imagef(vol, linearSmp, pos + (float4)(0, +offset.y, 0, 0)).x;
    s1.z = read_imagef(vol, linearSmp, pos + (float4)(0, 0, -offset.z, 0)).x;
    s2.z = read_imagef(vol, linearSmp, pos + (float4)(0, 0, +offset.z, 0)).x;

    float3 normal = fast_normalize(s2 - s1).xyz;
    if (length(normal) == 0.0f) // TODO: zero correct
        normal = (float3)(1.f, 0.f, 0.f);

    return normal;
}

// specular part of blinn-phong shading model
float3 specularBlinnPhong(float3 lightColor, float specularExp, float3 materialColor,
                          float3 normal, float3 toLightDir, float3 toCameraDir)
{
    float3 h = toCameraDir + toLightDir;
    // check for special case where the light source is exactly opposite
    // to the view direction, i.e. the length of the halfway vector is zero
    if (dot(h, h) < 1.e-6f) // check for squared length
        return (float3)(0.0f);

    h = normalize(h);
    return materialColor * lightColor * native_powr(max(dot(normal, h), 0.f), specularExp);
}

// simple illumination based on central differences
float3 illumination(read_only image3d_t vol, const float4 pos, float3 color, float3 toLightDir)
{
    float3 n = fast_normalize(-gradientCentralDiff(vol, pos));
    float3 l = fast_normalize(toLightDir.xyz);

    float3 amb = color * 0.2f;
    float3 diff = color * max(0.f, dot(n, l)) * 0.8f;
    float3 spec = specularBlinnPhong((float3)(1.f), 40.f, (float3)(1.f), n, l, toLightDir) * 0.2f;

    return (amb + diff + spec);
}

// intersect ray with a box
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
int intersectBox(float4 rayOrig,
                 float3 rayDir,
                 float *tnear,
                 float *tfar)
{
    // compute intersection of ray with all six bbox planes
    float4 invRay = native_divide((float4)(1.0f), (float4)(rayDir, 1.0f));
    float4 tBot = invRay * ((float4)(-1.0f, -1.0f, -1.0f, 1.0f) - rayOrig);
    float4 tTop = invRay * ((float4)(1.0f, 1.0f, 1.0f, 1.0f) - rayOrig);

    // re-order intersections to find smallest and largest on each axis
    float4 tMin = min(tTop, tBot);
    float4 tMax = max(tTop, tBot);

    // find the largest tMin and the smallest tMax
    float maxTmin = max(max(tMin.x, tMin.y), max(tMin.x, tMin.z));
    float minTmax = min(min(tMax.x, tMax.y), min(tMax.x, tMax.z));

    *tnear = maxTmin;
    *tfar = minTmax;

    return (int)(minTmax > maxTmin);
}


__kernel void volumeRender(
                           /***PRECISION***/ volData
                           , __write_only image2d_t outData
                           , __read_only image1d_t tffData     // constant transfer function value
                           , const float16 viewMat
                           , __global const int *shuffledIds
                           , const float stepSizeFactor
                           , const uint3 volRes
                           , const sampler_t sampler
                           , const float precisionDiv
                           , const float3 modelScale
                           /***ESS_ARGS***/
                           /***OFFSET_ARGS***/
                        )
{
    int2 globalId = (int2)(get_global_id(0), get_global_id(1));
    if(any(globalId >= get_image_dim(outData)))
        return;

    // pseudo random number [0,1] for ray offsets to avoid moire patterns
    float iptr;
    float rand = fract(sin(dot(convert_float2(globalId),
                       (float2)(12.9898f, 78.233f))) * 43758.5453f, &iptr);

    long gId = get_global_id(0) + get_global_id(1) * get_global_size(0);
    float16 viewLocal = viewMat;

    /***OFFSET***/
    
    /***SHUFFLE***/

    int2 texCoords = globalId;
    float aspectRatio = native_divide((float)get_global_size(1), (float)(get_global_size(0)));
    aspectRatio = min(aspectRatio, native_divide((float)get_global_size(0), (float)(get_global_size(1))));
    int maxSize = max(get_global_size(0), get_global_size(1));

    float2 pixelScreenSpace;    // [-1,1]
    pixelScreenSpace.x = native_divide(((float)globalId.x + 0.5f), convert_float(maxSize)) * 2.0f;
    pixelScreenSpace.y = native_divide(((float)globalId.y + 0.5f), convert_float(maxSize)) * 2.0f;
    // calculate correct offset based on aspect ratio
    pixelScreenSpace -= get_global_size(0) > get_global_size(1) ?
                        (float2)(1.0f, aspectRatio) : (float2)(aspectRatio, 1.0);
    // flip y-coordinate to point in right direction
    pixelScreenSpace.y *= -1.0f;

    float3 rayDir = (float3)(0.0f);
    float tnear = 0.0f;
    float tfar = FLT_MAX;
    int hit = 0;
    
    /***CAMERA***/
    /***ORTHO_NEAR***/

    float4 color = (float4)(1.f, 1.f, 1.f, 0.f);
    if (!hit)
    {
        // write output color: transparent white
        // color *= (float4)(255.f);
        write_imagef(outData, texCoords, color);
        return;
    }

    tnear = max(0.f, tnear); // clamp to near plane
    float samplingRate = 1.f/stepSizeFactor;
    float sampleDist = tfar - tnear;
    if (sampleDist < 1e-6f)
        return;
    float stepSize = min(sampleDist, sampleDist /
                            (samplingRate*length(sampleDist*rayDir.xyz*convert_float3(volRes))));
    float samples = ceil(sampleDist/stepSize);
    stepSize = sampleDist/samples;
    // offset by 'random' distance
//    tnear += rand*stepSize;

    float4 illumColor = (float4)(0.f);
    float alpha = 0.f;
    float4 pos = (float4)(0.f);
    float sample = 0.f;
    float4 sample4 = (float4)(0.f);
    float4 tfColor = (float4)(0.f);
    float opacity = 0.f;
    float t = tnear;
    float t_exit = tfar;

    /***ESS_PART_1***/

    // raycasting loop
    while (t < t_exit)
    {
        pos = camPos + (float4)(t*rayDir, 1.f);
        pos = pos * 0.5f + 0.5f;

        /***DATA_SOURCE***/
        
        opacity = 1.0f - native_powr(1.0f - tfColor.w, stepSizeFactor);
        color.xyz = color.xyz - ((float3)(1.0f) - tfColor.xyz) * opacity * (1.0f - alpha);
        alpha = alpha + opacity * (1.0f - alpha);

        /***ERT***/

        if (t >= tfar) break;
        t += stepSize;
    }

    /***ESS_PART_2***/

    /***SAMPLECNT***/

    color.w = alpha;
    //color *= (float4)(255.f);	// cimg scaling
    write_imagef(outData, texCoords, color);
}



//************************** Generate brick volume ***************************

__kernel void generateBricks( __read_only image3d_t volData
                             ,__write_only image3d_t volBrickData
                            )
{
    int3 coord = (int3)(get_global_id(0), get_global_id(1), get_global_id(2));
    if(any(coord >= get_image_dim(volBrickData).xyz))
        return;

    int3 voxPerCell = convert_int3(ceil(convert_float4(get_image_dim(volData))/
                                          convert_float4(get_image_dim(volBrickData))).xyz);
    int3 brickOrigin = voxPerCell * coord;
    int3 brickBounds = clamp(brickOrigin + voxPerCell, (int3)(0), get_image_dim(volData).xyz);

    float maxVal = 0.f;
    float minVal = 1.f;
    float value = 0.f;
		
    for (int k = brickOrigin.z; k < brickBounds.z; ++k)
    {
        for (int j = brickOrigin.y; j < brickBounds.y; ++j)
        {
            for (int i = brickOrigin.x; i < brickBounds.x; ++i)
            {
                value = read_imagef(volData, (int4)(i, j, k, 0)).x;  // [0; 1]
                minVal = min(minVal, value);
                maxVal = max(maxVal, value);
            }
        }
    }
	minVal = clamp(minVal, 0.f, 1.f);
	maxVal = clamp(maxVal, 0.f, 1.f);

    write_imagef(volBrickData, (int4)(coord, 0), (float4)(minVal, maxVal, 0.f, 1.f));
}
