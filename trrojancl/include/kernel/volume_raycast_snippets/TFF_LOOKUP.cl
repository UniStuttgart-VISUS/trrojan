/*TFF_LOOKUP*/
        // use nearest sampler because Intel seems to nnot support linear
        uint4 tfColorui = read_imageui(tfColors, nearestSmp, sample);
        tfColor.x = native_divide(tfColorui.x, 255.0f);
        tfColor.y = native_divide(tfColorui.y, 255.0f);
        tfColor.z = native_divide(tfColorui.z, 255.0f);
        tfColor.w = native_divide(tfColorui.w, 255.0f);
