// <copyright file="environment.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/environment.h"

#include "trrojan/d3d12/device.h"
#include "trrojan/d3d12/export.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// The Direct3D 12 environment.
    /// </summary>
    class TRROJAND3D12_API environment : public trrojan::environment_base {

    public:

        typedef environment_base::device_list device_list;

        typedef std::shared_ptr<environment> pointer;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline environment(void) : environment_base("d3d12") { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~environment(void);

        /// <inheritdoc />
        virtual size_t get_devices(device_list& dst);

        /// <inheritdoc />
        virtual void on_activate(void);

        /// <inheritdoc />
        virtual void on_deactivate(void);

        /// <inheritdoc />
        virtual void on_finalise(void);

        /// <inheritdoc />
        virtual void on_initialise(const cmd_line& cmdLine);

    private:

        std::vector<device::pointer> _devices;

    };
}
}
