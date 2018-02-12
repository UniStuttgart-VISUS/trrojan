/// <copyright file="SpherePixelShaderInt.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "SpherePipeline.hlsli"


/// <summary>
/// Transfer function.
/// </summary>
Texture1D TransferFunction : register(t0);


/// <summary>
/// Linear sampler for transfer function lookup.
/// </summary>
SamplerState LinearSampler : register(s0);



/// <summary>
/// </summary>
/// <param name="ray">The eye to fragment ray vector.</param>
/// <param name="normal">The normal of this fragment.</param>
/// <param name="lightPos">The position of the light source.</param>
/// <param name="colour">The base material colour.</param>
/// <returns>The lit colour of the pixel</returns>
float3 LocalLighting(const in float3 ray, const in float3 normal,
        const in float3 lightPos, const in float3 colour) {
    // TODO: rewrite!

    float3 lightDir = normalize(lightPos);

    float4 lightparams = float4(0.2, 0.8, 0.4, 10.0);
#define LIGHT_AMBIENT lightparams.x
#define LIGHT_DIFFUSE lightparams.y
#define LIGHT_SPECULAR lightparams.z
#define LIGHT_EXPONENT lightparams.w

    float nDOTl = dot(normal, lightDir);

    float3 r = normalize(2.0 * nDOTl * normal - lightDir);
    return LIGHT_AMBIENT * colour
        + LIGHT_DIFFUSE * colour * max(nDOTl, 0.0)
        + LIGHT_SPECULAR * float3(pow(max(dot(r, -ray), 0.0), LIGHT_EXPONENT).xxx);
}


/// <summary>
/// Entry point of the pixel shader doing the raycasting of a sphere on the
/// sprite.
/// </summary>
PsOutput Main(PsInput input) {
    PsOutput retval = (PsOutput) 0;

    //#define BILLBOARD
#ifdef BILLBOARD
    retval.Colour = float4(1.0, 1.0, 0.0, 1.0);
    return retval;
#endif

#ifdef HOLOMOL
    const uint eye = input.Eye;
#else /* HOLOMOL */
    const uint eye = 0;
#endif /* HOLOMOL */

    float4 coord;
    float3 ray;
    float lambda;
    float3 sphereintersection = 0.0.xxx;
    float3 normal;

    float4 camPos = input.CameraPosition;
    float4 camIn = input.CameraDirection;
    float4 camUp = input.CameraUp;
    float4 camRight = input.CameraRight;
    //float eyeSep = input.EyeSeparation;

    float4 objPos = float4(input.SphereParams.xyz, 1.0);

    // TODO
    float4 lightPos = normalize(float4(0.5, -1.0, -1.0, 0));
    lightPos *= -1;
    lightPos = mul(lightPos, ViewInvMatrix[eye]);
    lightPos = input.CameraDirection;
    float rad = input.SphereParams.w;
    float squarRad = rad * rad;

    // transform fragment coordinates from window coordinates to view coordinates.
    // TODO: Replace NV-stuff with own implementation to save texture sampling
//#define STEREO_WORLD_POS
#ifdef STEREO_WORLD_POS
    coord = ComputeWorldPosition(input.Position, true);
#else    
    input.Position.y = Viewport.w - input.Position.y;
    coord = input.Position
        * float4(2.0 / Viewport.z, 2.0 / Viewport.w, 1.0, 0.0)
        + float4(-1.0, -1.0, 0.0, 1.0);

    // transform fragment coordinates from view coordinates to object coordinates.
    coord = mul(coord, ViewProjInvMatrix[eye]);
#endif
    coord /= coord.w;
    coord -= objPos; // ... and to glyph space

                     // calc the viewing ray
    ray = coord.xyz - camPos.xyz;
    ray = normalize(ray);

    // calculate the geometry-ray-intersection
    float d1 = -dot(camPos.xyz, ray);                       // projected length of the cam-SphereParams-vector onto the ray
    float d2s = dot(camPos.xyz, camPos.xyz) - d1 * d1;      // off axis of cam-SphereParams-vector and ray
    float radicand = squarRad - d2s;                        // square of difference of projected length and lambda
    lambda = d1 - sqrt(radicand);                           // lambda

    if ((radicand < 0.0f) || (lambda < 0.0f)) {
        discard;
        //retval.Colour = 0.8.xxxx;
        retval.Colour = float4(1.0f, 0.0f, 0.0f, 1.0f);
        //retval.Depth = input.Position.z;
        return retval;
    } else {
        // chose color for lighting
        sphereintersection = lambda * ray + camPos.xyz;    // intersection point
                                                           // "calc" normal at intersection point
        normal = sphereintersection / rad;
        //normal = normalize(sphereintersection);
        // phong lighting with directional light
        float start = min(IntRangeGlobalRadTessFactor.x, IntRangeGlobalRadTessFactor.y);
        float range = abs(IntRangeGlobalRadTessFactor.y - IntRangeGlobalRadTessFactor.x);
        float texCoords = (input.Colour.r - start) / range;
        texCoords = 1.0f - texCoords;   // Invert for debugging purposes.
        float4 baseColour = TransferFunction.SampleLevel(LinearSampler, texCoords, 0);
        retval.Colour = float4(LocalLighting(ray, normal, lightPos.xyz, baseColour.rgb), baseColour.a);
    }

    // calculate depth
#define DEPTH
#ifdef DEPTH
    float4 Ding = float4(sphereintersection + objPos.xyz, 1.0);
    float depth = dot(ViewProjMatrix[eye]._13_23_33_43, Ding);
    float depthW = dot(ViewProjMatrix[eye]._14_24_34_44, Ding);
    //retval.Depth = ((depth / depthW) + 1.0) * 0.5;
    retval.Depth = (depth / depthW);
#else
    retval.Depth = input.Position.z;
#endif // DEPTH

    //retval.Colour = float4(1.0, 0.0, 0.0, 1.0);
    return retval;
}
