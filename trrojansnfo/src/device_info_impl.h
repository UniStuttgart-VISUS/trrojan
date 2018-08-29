/// <copyright file="device_info_impl.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <string>

#if defined(_WIN32)
#include <Windows.h>
#include <SetupAPI.h>
#endif /* defined(_WIN32) */

#include "trrojan/sysinfo/device_info.h"


namespace trrojan {
namespace sysinfo {
namespace detail {

    /// <summary>
    /// Generic implementation of <see cref="device_info" />.
    /// </summary>
    class device_info_impl : public device_info {

    public:

        typedef device_info::date_type date_type;
        typedef device_info::version_type version_type;

#if defined(WIN32)
        device_info_impl(HDEVINFO hDev, SP_DEVINFO_DATA& data);
#endif /* defined(WIN32) */

        virtual ~device_info_impl(void) = default;

        virtual const date_type& driver_date(void) const;

        virtual const char *driver_provider(void) const;

        virtual const version_type& driver_version(void) const;

        virtual const char *hardware_id(void) const;

        virtual const char *manufacturer(void) const;

        virtual const char *name(void) const;

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
