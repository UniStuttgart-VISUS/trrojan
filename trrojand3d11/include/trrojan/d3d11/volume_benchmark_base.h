/// <copyright file="volume_benchmark_base.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "datraw.h"

#include "trrojan/d3d11/benchmark_base.h"



namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Base class for D3D11 benchmarks.
    /// </summary>
    class TRROJAND3D11_API volume_benchmark_base : public benchmark_base {

    public:

        typedef benchmark_base::manoeuvre_step_type manoeuvre_step_type;

        typedef benchmark_base::manoeuvre_type manoeuvre_type;

        typedef benchmark_base::point_type point_type;

        typedef benchmark_base::viewport_type viewport_type;

        static const char *factor_data_set;
        static const char *factor_frame;
        static const char *factor_gpu_counter_iterations;
        static const char *factor_min_prewarms;
        static const char *factor_min_wall_time;
        static const char *factor_step_size;
        static const char *factor_xfer_func;

        virtual ~volume_benchmark_base(void) = default;

    protected:

        inline volume_benchmark_base(const std::string& name)
            : benchmark_base(name) { }

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
