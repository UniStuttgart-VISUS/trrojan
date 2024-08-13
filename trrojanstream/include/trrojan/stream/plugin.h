/// <copyright file="plugin.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/plugin.h"

#include "trrojan/stream/export.h"


namespace trrojan {
namespace stream {

    /// <summary>
    /// Descriptor for the RAM stream benchmark plugin.
    /// </summary>
    class TRROJANSTREAM_API plugin : public trrojan::plugin_base {

    public:

        typedef trrojan::plugin_base::benchmark_list benchmark_list;
        typedef trrojan::plugin_base::environment_list environment_list;

        inline plugin(void) : trrojan::plugin_base("stream") { }

        virtual ~plugin(void);

        virtual size_t create_benchmarks(benchmark_list& dst) const;

        virtual size_t create_environments(environment_list& dst) const;

    };

}
}
