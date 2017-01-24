/*ORTHO_TOP*/
    float4 nearPlanePos = (float4)(imgCoords.x, 1.0f/viewMat[11], imgCoords.y, 0.0f);
    rayDir = (float4)(0, -1, 0, 0);
