/// <copyright file="device_info.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <ctime>

#include "trrojan/sysinfo/export.h"


namespace trrojan {
namespace sysinfo {

    /// <summary>
    /// Defines the interface for the information that is known about a piece
    /// of hardware and its device driver.
    /// </summary>
    class TRROJANSNFO_API device_info {

    public:

        /// <summary>
        /// The type used to specifie a driver version.
        /// </summary>
        struct version_type {
            std::uint16_t major;
            std::uint16_t minor;
            std::uint16_t build;
            std::uint16_t revision;
        };

        /// <summary>
        /// The type to specify the date of the driver.
        /// </summary>
        typedef std::time_t date_type;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~device_info(void) = default;

        /// <summary>
        /// Answer the date when the current driver of the device was
        /// released.
        /// </summary>
        /// <returns></returns>
        virtual  const date_type& driver_date(void) const = 0;

        /// <summary>
        /// Answer who provides the driver for the device.
        /// </summary>
        /// <returns></returns>
        virtual const char *driver_provider(void) const = 0;

        /// <summary>
        /// Answer the version of the driver.
        /// </summary>
        /// <returns></returns>
        virtual const version_type& driver_version(void) const = 0;

        /// <summary>
        /// Answer the hardware ID of the device.
        /// </summary>
        /// <returns></returns>
        virtual const char *hardware_id(void) const = 0;

        /// <summary>
        /// Answer the manufacturer of the hardware.
        /// </summary>
        /// <returns></returns>
        virtual const char *manufacturer(void) const = 0;

        /// <summary>
        /// Answer the friendly name of the device.
        /// </summary>
        /// <returns></returns>
        virtual const char *name(void) const = 0;

    protected:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        device_info(void) = default;
    };

} /* end namespace trrojan */
} /* end namespace sysinfo */
