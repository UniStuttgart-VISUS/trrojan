/*ILLUMINATION_TEX*/
    float3 offset = native_divide((float3)(1.0f), (float3)volRes.x);
    float3 s1;
    float3 s2;
    s1.x = native_divide((float)read_imageui(volData, sampler, pos + (float4)(-offset.x, 0, 0, 0)).x, 255.0f);
    s2.x = native_divide((float)read_imageui(volData, sampler, pos + (float4)(+offset.x, 0, 0, 0)).x, 255.0f);
    s1.y = native_divide((float)read_imageui(volData, sampler, pos + (float4)(0, -offset.y, 0, 0)).x, 255.0f);
    s2.y = native_divide((float)read_imageui(volData, sampler, pos + (float4)(0, +offset.y, 0, 0)).x, 255.0f);
    s1.z = native_divide((float)read_imageui(volData, sampler, pos + (float4)(0, 0, -offset.z, 0)).x, 255.0f);
    s2.z = native_divide((float)read_imageui(volData, sampler, pos + (float4)(0, 0, +offset.z, 0)).x, 255.0f);
    float3 n = fast_normalize(s2 - s1); // / (2.0f * offset);

    illumColor = shading(n, (normalize((float4)(1.0f, -1.0f, -1.0f, 0.0f) - pos)).xyz, (pos - rayDir).xyz);
    tfColor.xyz = mix(tfColor.xyz, illumColor.xyz, 0.5f);
