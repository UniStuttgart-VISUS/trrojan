/*ORTHO_FRONT*/
    float4 nearPlanePos = (float4)(imgCoords.xy, 1.0f/viewMat[11], 0.0f);
    rayDir = (float4)(0, 0, -1, 0);
