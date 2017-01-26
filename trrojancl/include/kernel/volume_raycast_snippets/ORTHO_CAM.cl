/*ORTHO_CAM*/
    float4 camPos = (float4)(viewMat[3], viewMat[7], viewMat[11], 1.0f);    
    float4 viewPlane_x = (float4)(viewMat[0], viewMat[4], viewMat[ 8], viewMat[12]);
    float4 viewPlane_y = (float4)(viewMat[1], viewMat[5], viewMat[ 9], viewMat[13]);
    float4 viewPlane_z = (float4)(viewMat[2], viewMat[6], viewMat[10], viewMat[14]);
    float4 nearPlanePos = camPos + imgCoords.x*viewPlane_x + imgCoords.y*viewPlane_y;

    rayDir = -viewPlane_z;
    rayDir.w = 0;
    rayDir = fast_normalize(rayDir);
    camPos = nearPlanePos;
    
    hit = intersectBox(camPos, rayDir, &tnear, &tfar);
