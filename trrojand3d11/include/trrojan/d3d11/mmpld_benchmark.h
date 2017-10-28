/// <copyright file="mmpld_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <vector>

#include <Windows.h>
#include <d3d11.h>

#include "trrojan/benchmark.h"
#include "trrojan/enum_parse_helper.h"
#include "trrojan/mmpld_reader.h"
#include "trrojan/timer.h"

#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements a particle rendering benchmark using MMPLD files.
    /// </summary>
    class TRROJAND3D11_API mmpld_benchmark : public trrojan::benchmark_base {

    public:

        /// <summary>
        /// Creates a vector descriptor for the given MMPLD list header.
        /// </summary>
        static std::vector<D3D11_INPUT_ELEMENT_DESC> to_d3d11(
            const mmpld_reader::list_header& header);

        mmpld_benchmark(void);

        virtual ~mmpld_benchmark(void);

        virtual size_t run(const configuration_set& configs,
            const on_result_callback& callback);

        virtual trrojan::result run(const configuration& config);
    };

}
}
