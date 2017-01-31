/*ILLUMINATION_TEX*/
    float3 volResf;
    volResf.x = (float)(volRes.x);
    volResf.y = (float)(volRes.y);
    volResf.z = (float)(volRes.z);
    float3 offset = native_divide((float3)(1.0f, 1.0f, 1.0f), volResf);
    float3 s1;
    float3 s2;
    s1.x = read_imagef(volData, sampler, pos + (float4)(-offset.x, 0, 0, 0)).x * precisionDiv;
    s2.x = read_imagef(volData, sampler, pos + (float4)(+offset.x, 0, 0, 0)).x * precisionDiv;
    s1.y = read_imagef(volData, sampler, pos + (float4)(0, -offset.y, 0, 0)).x * precisionDiv;
    s2.y = read_imagef(volData, sampler, pos + (float4)(0, +offset.y, 0, 0)).x * precisionDiv;
    s1.z = read_imagef(volData, sampler, pos + (float4)(0, 0, -offset.z, 0)).x * precisionDiv;
    s2.z = read_imagef(volData, sampler, pos + (float4)(0, 0, +offset.z, 0)).x * precisionDiv;
    float3 n = fast_normalize(s2 - s1).xyz; // / (2.0f * offset);
    // Intel IGP (beignet) cannot do fast_normalize() but Intel CPU cannot do normalize()...
//     float3 n = normalize(s2 - s1).xyz; 

    illumColor = shading(n, (normalize((float4)(1.0f, -1.0f, -1.0f, 0.0f) - pos)).xyz, (pos - rayDir).xyz);
    tfColor.xyz = mix(tfColor.xyz, illumColor.xyz, 0.5f);
