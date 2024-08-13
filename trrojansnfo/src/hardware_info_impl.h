/// <copyright file="hardware_info_impl.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
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
