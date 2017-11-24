/*ORTHO_CAM*/
    float4 camPos = (float4)(viewLocal.s37b, 1.0f);    
    float4 viewPlane_x = viewLocal.s048c;
    float4 viewPlane_y = viewLocal.s159d;
    float4 viewPlane_z = viewLocal.s26ae;
    float4 nearPlanePos = camPos + pixelScreenSpace.x*viewPlane_x + pixelScreenSpace.y*viewPlane_y;

    rayDir = fast_normalize(-viewPlane_z.xyz*modelScale);
    camPos = nearPlanePos;
    camPos.xyz *= modelScale;
    
    hit = intersectBox(camPos, rayDir, &tnear, &tfar);
