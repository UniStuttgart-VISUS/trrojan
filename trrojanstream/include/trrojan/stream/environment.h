/// <copyright file="environment.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/environment.h"

#include "trrojan/stream/export.h"


namespace trrojan {
namespace stream {

    // TODO: remove this class? The stream benchmark does not need an env.

    /// <summary>
    /// The environment for memory streaming benchmarks.
    /// </summary>
    class TRROJANSTREAM_API environment : public trrojan::environment_base {

    public:

        typedef environment_base::device_list device_list;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline environment(void) : environment_base("stream") { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~environment(void);

        /// <inheritdoc />
        virtual size_t get_devices(device_list& dst);

        /// <inheritdoc />
        virtual void on_initialise(const std::vector<std::string> &cmdLine);

    };

}
}
