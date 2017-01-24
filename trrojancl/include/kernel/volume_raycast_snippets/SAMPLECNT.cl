    /*SAMPLECNT*/
    float rayDepth = min(1.0f, i*stepSize);
    write_imagef(outData, texCoords, (float4)(rayDepth, rayDepth, rayDepth, 1.0f));
    return;
