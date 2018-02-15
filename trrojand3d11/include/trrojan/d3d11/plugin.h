/// <copyright file="plugin.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/plugin.h"

#include <cinttypes>
#include <vector>

#include <Windows.h>

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

        /// <summary>
        /// Load a resource from the Direct3D plugin's DLL.
        /// </summary>
        /// <param name="name">The name of the resource. Alternately, rather
        /// than a pointer, this parameter can be MAKEINTRESOURCE(ID), where ID
        /// is the integer identifier of the resource.</param>
        /// <param name="type">The resource type. Alternately, rather than a
        /// pointer, this parameter can be MAKEINTRESOURCE(ID), where ID is the
        /// integer identifier of the given resource type. </param>
        /// <returns>The content of the resource.</returns>
        static std::vector<std::uint8_t> load_resource(LPCTSTR name,
            LPCSTR type);

        inline plugin(void) : trrojan::plugin_base("d3d11") { }

        virtual ~plugin(void);

        virtual size_t create_benchmarks(benchmark_list& dst) const;

        virtual size_t create_environments(environment_list& dst) const;

    };

}
}
