/*BUFFER*/
        uint3 iCoords = (uint3)(pos.x * (volRes.x-1), pos.y * (volRes.y-1), pos.z * (volRes.z-1));
        uint numVoxels = volRes.x*volRes.y*volRes.z;
        uint id = (iCoords.x + iCoords.y * volRes.x + iCoords.z * volRes.x * volRes.y); //, 0u, numVoxels - 1u);
        sample = native_divide(((float)volData[id]), 255.0f);
        tfColor = sample;

        /***TFF_LOOKUP***/

        /***ILLUMINATION***/
