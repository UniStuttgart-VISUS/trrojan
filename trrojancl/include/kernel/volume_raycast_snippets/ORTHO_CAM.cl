/*ORTHO_CAM*/
    /***ORTHO_VIEW***/
    float4 camPos = nearPlanePos;
    hit = intersectBox(camPos, rayDir, &tnear, &tfar);
