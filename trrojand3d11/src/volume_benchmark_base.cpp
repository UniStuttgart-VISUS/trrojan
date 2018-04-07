/// <copyright file="volume_benchmark_base.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/volume_benchmark_base.h"


#define _VOL_BENCH_DEFINE_FACTOR(f)                                            \
const char *trrojan::d3d11::volume_benchmark_base::factor_##f = #f

_VOL_BENCH_DEFINE_FACTOR(data_set);
_VOL_BENCH_DEFINE_FACTOR(frame);
_VOL_BENCH_DEFINE_FACTOR(frame);
_VOL_BENCH_DEFINE_FACTOR(gpu_counter_iterations);
_VOL_BENCH_DEFINE_FACTOR(min_prewarms);
_VOL_BENCH_DEFINE_FACTOR(min_wall_time);
_VOL_BENCH_DEFINE_FACTOR(step_size);
_VOL_BENCH_DEFINE_FACTOR(xfer_func);
