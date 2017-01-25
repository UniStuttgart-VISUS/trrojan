/*ORTHO_CAM*/
    float4 nearPlanePos = (float4)(imgCoords.xy, 1.0f/viewMat[11], 0.0f); 
    float4 camPos = nearPlanePos;
    rayDir = (float4)(-viewMat[8], -viewMat[9], -viewMat[10], 0);
    
    hit = intersectBox(camPos, rayDir, &tnear, &tfar);
