/// <copyright file="device_info.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <array>
#include <cassert>
#include <cinttypes>
#include <ctime>

#if defined(_WIN32)
#include <Windows.h>
#include <SetupAPI.h>
#endif /* defined(_WIN32) */

#include "trrojan/sysinfo/export.h"


namespace trrojan {
namespace sysinfo {

    /// <summary>
    /// Encapsulates information about a single piece of hardware and
    /// potentially its drivers.
    /// </summary>
    class TRROJANSNFO_API device_info {

    public:

        /// <summary>
        /// The type to specify the date of the driver.
        /// </summary>
        typedef std::time_t date_type;

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
        /// Initialises a new instance.
        /// </summary>
        inline device_info(void) : impl(nullptr) { }

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        inline device_info(const device_info& rhs) : impl(nullptr) {
            *this = rhs;
        }

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        inline device_info(device_info&& rhs) : impl(rhs.impl) {
            rhs.impl = nullptr;
        }

#if defined(WIN32)
        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        device_info(HDEVINFO hDev, SP_DEVINFO_DATA& data);
#endif /* defined(WIN32) */

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~device_info(void);

        /// <summary>
        /// Answer the date when the current driver of the device was
        /// released.
        /// </summary>
        /// <returns></returns>
        inline const date_type& driver_date(void) const {
            assert(this->impl != nullptr);
            return this->impl->driver_date();
        }

        /// <summary>
        /// Answer who provides the driver for the device.
        /// </summary>
        /// <returns></returns>
        inline const char *driver_provider(void) const {
            assert(this->impl != nullptr);
            return this->impl->driver_provider();
        }

        /// <summary>
        /// Answer the version of the driver.
        /// </summary>
        /// <returns></returns>
        inline const version_type& driver_version(void) const {
            assert(this->impl != nullptr);
            return this->impl->driver_version();
        }

        /// <summary>
        /// Answer the hardware ID of the device.
        /// </summary>
        /// <returns></returns>
        inline const char *hardware_id(void) const {
            assert(this->impl != nullptr);
            return this->impl->hardware_id();
        }

        /// <summary>
        /// Answer the manufacturer of the hardware.
        /// </summary>
        /// <returns></returns>
        inline const char *manufacturer(void) const {
            assert(this->impl != nullptr);
            return this->impl->manufacturer();
        }

        /// <summary>
        /// Answer the friendly name of the device.
        /// </summary>
        /// <returns></returns>
        inline const char *name(void) const {
            assert(this->impl != nullptr);
            return this->impl->name();
        }

        device_info& operator =(const device_info& rhs);

        device_info& operator =(device_info&& rhs);

    private:

        device_info *impl;
    };

} /* end namespace trrojan */
} /* end namespace sysinfo */
