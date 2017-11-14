    /*PERSPECTIVE_CAM*/
    // z position of view plane is -1.0 to fit the cube to the screen quad when axes are aligned, 
    // zoom is -1 and the data set is uniform in each dimension
    // (with FoV of 90° and near plane in range [-1,+1]).
    float4 nearPlanePos = fast_normalize((float4)(imgCoords, -1.99f, 0.0f));
    // transform nearPlane to world space    
    rayDir = transformPoint(viewLocal, nearPlanePos).xyz;
    // origin is translation from view matrix
    float4 camPos = (float4)(viewLocal.s37b, 1.0f);  
    
    rayDir = fast_normalize(rayDir);
    hit = intersectBox(camPos, rayDir, &tnear, &tfar);
