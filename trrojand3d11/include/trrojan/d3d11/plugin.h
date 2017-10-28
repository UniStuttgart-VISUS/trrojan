/// <copyright file="plugin.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/plugin.h"

#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Descriptor for the Direct3D plugin.
    /// </summary>
    class TRROJAND3D11_API plugin : public trrojan::plugin_base {

    public:

        typedef trrojan::plugin_base::benchmark_list benchmark_list;
        typedef trrojan::plugin_base::environment_list environment_list;

        inline plugin(void) : trrojan::plugin_base("d3d11") { }

        virtual ~plugin(void);

        virtual size_t create_benchmarks(benchmark_list& dst) const;

        virtual size_t create_environments(environment_list& dst) const;

    };

}
}
