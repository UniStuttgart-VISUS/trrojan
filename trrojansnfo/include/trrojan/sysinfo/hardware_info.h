/// <copyright file="hardware_info.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/sysinfo/export.h"


namespace trrojan {
namespace sysinfo {

    /// <summary>
    /// Provides information about the hardware installed in the system the
    /// software is running on.
    /// </summary>
    class TRROJANSNFO_API hardware_info {

    public:

        static void _DOWEL(void);

    };

} /* end namespace trrojan */
} /* end namespace sysinfo */
