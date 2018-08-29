/// <copyright file="hardware_info.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/sysinfo/device_info.h"
#include "trrojan/sysinfo/export.h"


namespace trrojan {
namespace sysinfo {

    /* Forward declarations. */
    namespace detail { struct hardware_info_impl; }

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
        ~hardware_info(void);

        /// <summary>
        /// Returns the GPUs found in the system.
        /// </summary>
        /// <param name ="outDevices">Receives the pointers to the
        /// <see cref="device_info" />s if the pointer is not <c>nullptr</c>
        /// and the buffer is large enough. The memory designated by the
        /// pointers is owned by the callee and stays valid as long as the
        /// <see cref="hardware_info" /> lives.</param>
        /// <param name ="inOutCntDevices">On entry, the size of the array
        /// passed to <paramref name="outDevices" />, on exit the number of
        /// devices that have been or should have been returned.</param>
        /// <returns><c>true</c> if all devices have been returned,
        /// <c>false</c> if the buffer was too small.</returns>
        bool gpus(const device_info **outDevices,
            size_t& inOutCntDevices) const;

        /// <summary>
        /// Assignment.
        /// </summary>
        hardware_info& operator =(const hardware_info& rhs);

        /// <summary>
        /// Move assignment.
        /// </summary>
        hardware_info& operator =(hardware_info&& rhs);

        /// <summary>
        /// Answer whether the hardware information is valid.
        /// </summary>
        inline operator bool(void) const {
            return (this->impl != nullptr);
        }

    private:

        detail::hardware_info_impl *impl;

    };

} /* end namespace trrojan */
} /* end namespace sysinfo */
