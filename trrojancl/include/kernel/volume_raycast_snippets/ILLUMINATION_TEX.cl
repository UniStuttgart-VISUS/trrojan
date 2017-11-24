/*ILLUMINATION_TEX*/
    tfColor.xyz = illumination(volData, pos, tfColor.xyz, fast_normalize(camPos.xyz - pos.xyz));
