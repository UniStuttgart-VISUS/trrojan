    /*ESS_PART_1*/
	
    // 3D DDA initialization
    int3 bricksRes = get_image_dim(volBrickData).xyz;
    float3 brickLen = (float3)(1.f) / convert_float3(bricksRes);
    float3 invRay = 1.f/rayDir;
    int3 step = convert_int3(sign(rayDir));
    if (rayDir.x == 0.f)
    {
        invRay.x = FLT_MAX;
        step.x = 1;
    }
    if (rayDir.y == 0.f)
    {
        invRay.y = FLT_MAX;
        step.y = 1;
    }
    if (rayDir.z == 0.f)
    {
        invRay.z = FLT_MAX;
        step.z = 1;
    }
    float3 deltaT = convert_float3(step)*(brickLen*2.f*invRay);
    float3 voxIncr = (float3)0;

    // convert ray starting point to cell coordinates
    float3 rayOrigCell = (camPos.xyz + rayDir * tnear) - (float3)(-1.f);
    int3 cell = clamp(convert_int3(floor(rayOrigCell / (2.f*brickLen))),
                        (int3)(0), convert_int3(bricksRes.xyz) - 1);

    // add +1 to cells if ray dir component is negative: rayDir >= 0 ? (-1) : 0
    float3 tv = tnear + (convert_float3(cell - isgreaterequal(rayDir, (float3)(0))) * (2.f*brickLen) - rayOrigCell) * invRay;
    int3 exit = step * bricksRes.xyz;
    if (exit.x < 0) exit.x = -1;
    if (exit.y < 0) exit.y = -1;
    if (exit.z < 0) exit.z = -1;
    // length of diagonal of a brick => longest distance through brick
    float brickDia = length(brickLen)*2.f;

    // 3D DDA loop over low res grid for image order empty space skipping
    while (t < tfar)
    {
        float2 minMaxDensity = read_imagef(volBrickData, (int4)(cell, 0)).xy;

        // increment to next brick
        voxIncr.x = (tv.x <= tv.y) && (tv.x <= tv.z) ? 1 : 0;
        voxIncr.y = (tv.y <= tv.x) && (tv.y <= tv.z) ? 1 : 0;
        voxIncr.z = (tv.z <= tv.x) && (tv.z <= tv.y) ? 1 : 0;
        cell += convert_int3(voxIncr) * step;    // [0; res-1]

        t_exit = dot((float3)(1.f), tv * voxIncr);
        t_exit = clamp(t_exit, t+stepSize, t+brickDia);
        tv += voxIncr*deltaT;

        // skip bricks that contain only fully transparent voxels
        float alphaMax = read_imagef(tffData, linearSmp, minMaxDensity.y).w;

        if (alphaMax < 1e-6f)
        {
            uint prefixMin = read_imageui(tffPrefix, nearestSmp, minMaxDensity.x).x;
            uint prefixMax = read_imageui(tffPrefix, nearestSmp, minMaxDensity.y).x;
            if (prefixMin == prefixMax)
            {
                t = t_exit;
                continue;
            }
        }

