/// <copyright file="plugin.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
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

        typedef trrojan::plugin_base::environment_list environment_list;

        inline plugin(void) : trrojan::plugin_base("stream") { }

        virtual ~plugin(void);

        virtual size_t create_environments(environment_list& dst) const;

    };

}
}
