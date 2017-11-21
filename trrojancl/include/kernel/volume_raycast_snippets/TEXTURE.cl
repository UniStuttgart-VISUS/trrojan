/*TEXTURE*/
        // read_image always returns a 4 component vector: (r, 0.0, 0.0, 1.0)
        // We have to use floating point textures in order to use linear interpolation. (!)
        sample4 = read_imagef(volData, linearSmp, pos);
        sample = (float)sample4.x;
        tfColor = (float4)(sample, 0.0f, 0.0f, 0.1f);

        /***TFF_LOOKUP***/
        /***ISO_SURFACE_TEX***/

        /***ILLUMINATION***/
