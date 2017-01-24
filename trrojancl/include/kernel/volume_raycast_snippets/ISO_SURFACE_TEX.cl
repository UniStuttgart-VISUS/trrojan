    /* ISO_SURFACE_TEX*/
    float4 nextPos = pos + stepSize*rayDir;
    float4 nextSample = read_imagef(volData, sampler, nextPos);
    
    float isoValue = 0.3f;

    if ((sample - isoValue)*(nextSample.x - isoValue) <= 0.0f)
    {
        float d = (isoValue - sample) / (nextSample.x - sample);
        pos += d*(nextPos - pos);
        illumColor = (float4)(0.5f, 0.5f, 0.5f, 0.0f);
        
        /***ILLUMINATION_TEX_ISO***/

        color = illumColor;
        break;
    }
