/// <copyright file="win32_device_info.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#if defined(WIN32)
#include <string>

#include "trrojan/sysinfo/device_info.h"


namespace trrojan {
namespace sysinfo {
namespace detail {

    /// <summary>
    /// An implementation of <see cref="device_info" /> for Windows.
    /// </summary>
    /// <remarks>
    /// This implementation has a <see cref="device_info::impl" /> pointer,
    /// which always should be <c>nullptr</c>. This is a nasty hack, because
    /// I am too lazy to add an interface class ...
    /// </remarks>
    class win32_device_info : public device_info {

    public:

        typedef device_info::date_type date_type;
        typedef device_info::version_type version_type;

        win32_device_info(HDEVINFO hDev, SP_DEVINFO_DATA& data);

        win32_device_info(const device_info *rhs);

        win32_device_info(const win32_device_info&) = delete;

        ~win32_device_info(void) = default;

        inline const date_type& driver_date(void) const {
            return this->_driver_date;
        }

        inline const char *driver_provider(void) const {
            return this->_driver_provider.c_str();
        }

        inline const version_type& driver_version(void) const {
            return this->_driver_version;
        }

        inline const char *hardware_id(void) const {
            return this->_hardware_id.c_str();
        }

        inline const char *manufacturer(void) const {
            return this->_manufacturer.c_str();
        }

        inline const char *name(void) const {
            return this->_name.c_str();
        }

        win32_device_info& operator =(const win32_device_info&) = delete;

    private:

        date_type _driver_date;
        std::string _driver_provider;
        version_type _driver_version;
        std::string _hardware_id;
        std::string _manufacturer;
        std::string _name;
    };

} /* end namespace detail */
} /* end namespace trrojan */
} /* end namespace sysinfo */
#endif /* defined(WIN32) */
