/// <copyright file="environment.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/environment.h"

#include "trrojan/d3d11/debug_view.h"
#include "trrojan/d3d11/device.h"
#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// The Direct3D 11 environment.
    /// </summary>
    class TRROJAND3D11_API environment : public trrojan::environment_base {

    public:

        typedef environment_base::device_list device_list;

        typedef std::shared_ptr<environment> pointer;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline environment(void) : environment_base("d3d11") { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~environment(void);

        /// <inheritdoc />
        virtual size_t get_devices(device_list& dst);

        /// <inheritdoc />
        virtual void on_activate(void);

        /// <inheritdoc />
        virtual void on_deactivate(void) noexcept;

        /// <inheritdoc />
        virtual void on_finalise(void) noexcept;

        /// <inheritdoc />
        virtual void on_initialise(const cmd_line& cmdLine);

    private:

        std::shared_ptr<debug_view> debug_view;

        std::vector<device::pointer> devices;

    };
}
}
