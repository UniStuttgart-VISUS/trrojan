/// <copyright file="win32_hardware_info.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <vector>

#include "trrojan/sysinfo/hardware_info.h"


namespace trrojan {
namespace sysinfo {
namespace detail {

    /// <summary>
    /// </summary>
    class win32_hardware_info : public hardware_info {

    public:


    private:

        std::vector<device_info> _gpus;
    };

} /* end namespace detail */
} /* end namespace trrojan */
} /* end namespace sysinfo */
