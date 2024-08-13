/// <copyright file="device_info_impl.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "device_info_impl.h"
#include "setupapiutil.h"
#include "utilities.h"


#if defined(_WIN32)
/*
 * trrojan::sysinfo::detail::device_info_impl::device_info_impl
 */
trrojan::sysinfo::detail::device_info_impl::device_info_impl(HDEVINFO hDev,
        SP_DEVINFO_DATA& data) {
    auto desc = detail::get_device_registry_property(hDev, data,
        SPDRP_DEVICEDESC);
    auto hwid = detail::get_device_registry_property(hDev, data,
        SPDRP_HARDWAREID);

    detail::add_device_install_flags(hDev, &data, 0,
        DI_FLAGSEX_INSTALLEDDRIVER);

    detail::enum_driver_info(hDev, &data, SPDIT_COMPATDRIVER, [this](HDEVINFO h,
            PSP_DEVINFO_DATA d, SP_DRVINFO_DATA& drv) {
        // From https://cyberspock.com/2015/10/02/some-time_point-to-from-filetime-conversions/
        ULARGE_INTEGER ticks;
        ticks.LowPart = drv.DriverDate.dwLowDateTime;
        ticks.HighPart = drv.DriverDate.dwHighDateTime;
        this->_driver_date = ticks.QuadPart / 10000000ULL - 11644473600ULL;

        auto version = detail::split_driver_version(drv.DriverVersion);
        this->_driver_version.major = version[0];
        this->_driver_version.minor = version[1];
        this->_driver_version.build = version[2];
        this->_driver_version.revision = version[3];

        this->_driver_provider =  drv.ProviderName;
        this->_manufacturer =  drv.MfgName;
        
        return false;
    });

    this->_hardware_id = reinterpret_cast<char *>(hwid.data());
    this->_name = reinterpret_cast<char *>(desc.data());
}
#endif /* defined(_WIN32) */


/*
 * trrojan::sysinfo::detail::device_info_impl::driver_date
 */
const trrojan::sysinfo::detail::device_info_impl::date_type&
trrojan::sysinfo::detail::device_info_impl::driver_date(void) const {
    return this->_driver_date;
}


/*
 * trrojan::sysinfo::detail::device_info_impl::driver_provider
 */
const char *trrojan::sysinfo::detail::device_info_impl::driver_provider(
        void) const {
    return this->_driver_provider.c_str();
}


/*
 * trrojan::sysinfo::detail::device_info_impl::driver_version
 */
const trrojan::sysinfo::detail::device_info_impl::version_type&
trrojan::sysinfo::detail::device_info_impl::driver_version(void) const {
    return this->_driver_version;
}


/*
 * trrojan::sysinfo::detail::device_info_impl::hardware_id
 */
const char *trrojan::sysinfo::detail::device_info_impl::hardware_id(
        void) const {
    return this->_hardware_id.c_str();
}


/*
 * trrojan::sysinfo::detail::device_info_impl::manufacturer
 */
const char *trrojan::sysinfo::detail::device_info_impl::manufacturer(
        void) const {
    return this->_manufacturer.c_str();
}


/*
 * trrojan::sysinfo::detail::device_info_impl::name
 */
const char *trrojan::sysinfo::detail::device_info_impl::name(void) const {
    return this->_name.c_str();
}
