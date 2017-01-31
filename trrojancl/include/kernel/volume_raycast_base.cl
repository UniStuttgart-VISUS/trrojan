#pragma OPENCL EXTENSION cl_khr_fp64 : enable

constant sampler_t linearSmp = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE |
                                CLK_FILTER_LINEAR;
constant sampler_t nearestSmp = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE |
                                CLK_FILTER_NEAREST;

// Lambert shading
float4 shading(const float3 n, const float3 l, const float3 v)
{
    float4 ambient = (float4)(1.0f, 1.0f, 1.0f, 0.2f);
    float4 diffuse = (float4)(1.0f, 1.0f, 1.0f, 0.8f);

    float intensity = fabs(dot(n, l)) * diffuse.w;
    diffuse *= intensity;

    float4 color4 = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    color4.xyz = ambient.xyz * ambient.w + diffuse.xyz;
    return color4;
}

// intersect ray with a box
// http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
int intersectBox(float4 rayOrig,
                 float4 rayDir,
                 float *tnear,
                 float *tfar)
{
    // compute intersection of ray with all six bbox planes
    float4 invRay = native_divide((float4)(1.0f, 1.0f, 1.0f, 1.0f), rayDir);
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
                           /***PRECISION***/ volData,
                           __write_only image2d_t outData,
                           __read_only image1d_t tffData,     // constant transfer function values
                           __constant float *viewMat,
                           __global const int *shuffledIds,
                           const float stepSizeFactor,
                           const int4 volRes,
                           const sampler_t sampler,
                           const float precisionDiv
                           /***OFFSET_ARGS***/
                        )
{
    float stepSize = native_divide(stepSizeFactor, (float)max(volRes.x, max(volRes.y, volRes.z)));
    stepSize *= 8.0f; // normalization to octile

    uint idX = get_global_id(0);
    uint idY = get_global_id(1);
    long gId = get_global_id(0) + get_global_id(1) * get_global_size(0);

    /***OFFSET***/
    
    /***SHUFFLE***/

    int2 texCoords = (int2)(idX, idY);
    float aspectRatio = native_divide((float)get_global_size(1), (float)(get_global_size(0)));
    aspectRatio = min(aspectRatio, native_divide((float)get_global_size(0), (float)(get_global_size(1))));
    int maxSize = max(get_global_size(0), get_global_size(1));
    float2 imgCoords;
    imgCoords.x = native_divide(((float)idX + 0.5f), (float)(maxSize)) * 2.0f;
    imgCoords.y = native_divide(((float)idY + 0.5f), (float)(maxSize)) * 2.0f;
    // calculate correct offset based on aspect ratio
    imgCoords -= get_global_size(0) > get_global_size(1) ? 
                        (float2)(1.0f, aspectRatio) : (float2)(aspectRatio, 1.0);
    
    // flip y-coordinate to point in right direction
    imgCoords.y *= -1.0f;

    float4 rayDir = (float4)(0, 0, 0, 0);
    float tnear = 0.0f;
    float tfar = 1.0f;
    int hit = 0;
    
    /***CAMERA***/
    /***ORTHO_NEAR***/
    
    if (!hit)
    {
        // write output color: transparent black
        float4 color = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
        write_imagef(outData, texCoords, color);
        return;
    }

    tnear = max(0.0f, tnear);     // clamp to near plane

    // march along ray from front to back, accumulating color
    float4 color = (float4)(1.0f, 1.0f, 1.0f, 0.0f);
    float4 illumColor = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float alpha = 0.0f;
    float4 pos = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float sample = 0.0f;
    float4 sample4;
    float4 tfColor = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float opacity = 0.0f;
    uint i = 0;
    float t = 0.0f;

    // raycasting loop
    while (true)
    {
        t = (tnear + stepSize*i);
        pos = camPos + t*rayDir;
        pos = pos * 0.5f + 0.5f;

        /***DATA_SOURCE***/
        
        opacity = 1.0f - pow(1.0f - tfColor.w, stepSizeFactor);

        color.xyz = color.xyz - ((float3)(1.0f, 1.0f, 1.0f) - tfColor.xyz) * opacity * (1.0f - alpha);
        alpha = alpha + opacity * (1.0f - alpha);

        /***ERT***/

        if (t > tfar) break;
        ++i;
    }
    color.w = alpha;

    /***SAMPLECNT***/

    color *= (float4)(255.0f, 255.0f, 255.0f, 255.0f);
    write_imagef(outData, texCoords, color);
    
//     uint4 colorui;
//     colorui.x = (uint)floor(color.x);
//     colorui.y = (uint)floor(color.y);
//     colorui.z = (uint)floor(color.z);
//     colorui.w = (uint)floor(color.w);
//     write_imageui(outData, texCoords, colorui);
}

