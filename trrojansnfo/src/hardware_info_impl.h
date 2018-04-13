/// <copyright file="hardware_info_impl.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <vector>

#include "device_info_impl.h"


namespace trrojan {
namespace sysinfo {
namespace detail {

    /// <summary>
    /// Opque implementation of <see cref="hardware_info" />.
    /// </summary>
    struct hardware_info_impl {

        void update(void);

        std::vector<device_info_impl> gpus;

    };

} /* end namespace detail */
} /* end namespace trrojan */
} /* end namespace sysinfo */
