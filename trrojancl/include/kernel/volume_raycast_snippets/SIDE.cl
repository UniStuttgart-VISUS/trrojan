/*ORTHO_SIDE*/
    //float4 nearPlanePos = (float4)(1.0f, normCoords.y, normCoords.x, 1.0f);
    float4 nearPlanePos = (float4)(1.0f/viewMat[11], imgCoords, 0.0f);
    rayDir = (float4)(-1, 0, 0, 0); 
