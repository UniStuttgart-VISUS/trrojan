/// <copyright file="hardware_info.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cassert>

#include "trrojan/sysinfo/device_info.h"


namespace trrojan {
namespace sysinfo {

    /// <summary>
    /// Provides information about the hardware installed in the system the
    /// software is running on.
    /// </summary>
    class TRROJANSNFO_API hardware_info {

    public:

        /// <summary>
        /// Collects information about the hardware on the system the software
        /// is currently running on.
        /// </summary>
        /// <returns>The hardware information for the system.</return>
        static hardware_info collect(void);

        /// <summary>
        /// Initialise a new instance.
        /// </summary>
        inline hardware_info(void) : impl(nullptr) { }

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        inline hardware_info(const hardware_info& rhs) : impl(nullptr) {
            *this = rhs;
        }

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        inline hardware_info(hardware_info&& rhs) : impl(rhs.impl) {
            rhs.impl = nullptr;
        }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~hardware_info(void);

        hardware_info& operator =(const hardware_info& rhs);

        hardware_info& operator =(hardware_info&& rhs);

    private:

        hardware_info *impl;

    };

} /* end namespace trrojan */
} /* end namespace sysinfo */
