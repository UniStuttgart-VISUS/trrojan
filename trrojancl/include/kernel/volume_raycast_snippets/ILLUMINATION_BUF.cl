/*ILLUMINATION_BUF*/
        float3 s1 = (float3)(0.0f, 0.0f, 0.0f);
        float3 s2 = (float3)(0.0f, 0.0f, 0.0f);
        
        id = clamp((iCoords.x - 1u) + iCoords.y * volRes.x + iCoords.z * volRes.x * volRes.y, 0u, numVoxels - 1u);
        s1.x = native_divide(((float)volData[id]), precisionDiv);

        id = clamp((iCoords.x + 1u) + iCoords.y * volRes.x + iCoords.z * volRes.x * volRes.y, 0u, numVoxels - 1u);
        s2.x = native_divide(((float)volData[id]), precisionDiv);

        id = clamp(iCoords.x + (iCoords.y - 1u) * volRes.x + iCoords.z * volRes.x * volRes.y, 0u, numVoxels - 1u);
        s1.y = native_divide(((float)volData[id]), precisionDiv);

        id = clamp(iCoords.x + (iCoords.y + 1u) * volRes.x + iCoords.z * volRes.x * volRes.y, 0u, numVoxels - 1u);
        s2.y = native_divide(((float)volData[id]), precisionDiv);

        id = clamp(iCoords.x + iCoords.y * volRes.x + (iCoords.z - 1u) * volRes.x * volRes.y, 0u, numVoxels - 1u);
        s1.z = native_divide(((float)volData[id]), precisionDiv);

        id = clamp(iCoords.x + iCoords.y * volRes.x + (iCoords.z + 1u) * volRes.x * volRes.y, 0u, numVoxels - 1u);
        s2.z = native_divide(((float)volData[id]), precisionDiv);

        float3 n = fast_normalize(s2 - s1); // use normalize() for Intel IGP

        illumColor = shading(n, (fast_normalize((float4)(1.0f, -1.0f, -1.0f, 0.0f) - pos)).xyz, rayDir.xyz);
        tfColor.xyz = mix(tfColor.xyz, illumColor.xyz, 0.5f);
