/*ORTHO_CAM*/
    float4 camPos = (float4)(viewLocal.s37b, 1.0f);    
    float4 viewPlane_x = viewLocal.s048c;
    float4 viewPlane_y = viewLocal.s159d;
    float4 viewPlane_z = viewLocal.s26ae;
    float4 nearPlanePos = camPos + imgCoords.x*viewPlane_x + imgCoords.y*viewPlane_y;

    rayDir = fast_normalize(-viewPlane_z.xyz);
    camPos = nearPlanePos;
    
    hit = intersectBox(camPos, rayDir, &tnear, &tfar);
