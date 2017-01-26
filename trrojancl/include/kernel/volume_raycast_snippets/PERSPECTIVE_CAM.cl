    /*PERSPECTIVE_CAM*/
    // z position of view plane is -1.0 to fit the cube to the screen quad when axes are aligned, 
    // zoom is -1 and the data set is uniform in each dimension
    // (with FoV of 90° and near plane in range [-1,+1]).
    float4 nearPlanePos = fast_normalize((float4)(imgCoords, -1.0f, 0.0f));
    // transform nearPlane to world space    
    rayDir.x = dot((float4)(viewMat[ 0], viewMat[ 1], viewMat[ 2], viewMat[ 3]), nearPlanePos);
    rayDir.y = dot((float4)(viewMat[ 4], viewMat[ 5], viewMat[ 6], viewMat[ 7]), nearPlanePos);
    rayDir.z = dot((float4)(viewMat[ 8], viewMat[ 9], viewMat[10], viewMat[11]), nearPlanePos);
    rayDir.w = dot((float4)(viewMat[12], viewMat[13], viewMat[14], viewMat[15]), nearPlanePos);
    
    // origin is translation from view matrix
    float4 camPos = (float4)(viewMat[3], viewMat[7], viewMat[11], 1.0f);    
    
    rayDir = fast_normalize(rayDir);
    hit = intersectBox(camPos, rayDir, &tnear, &tfar);
