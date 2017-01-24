/*TEXTURE*/
		sample4 = read_imageui(volData, sampler, pos);
		sample = native_divide((float)sample4.x, precisionDiv);
		tfColor = sample;

		/***TFF_LOOKUP***/
		/***ISO_SURFACE_TEX***/

		/***ILLUMINATION***/
