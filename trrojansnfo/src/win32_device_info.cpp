/// <copyright file="hardware_info.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#if defined(_WIN32)
#include "win32_device_info.h"

#include "setupapiutil.h"
#include "utilities.h"


/*
 * trrojan::sysinfo::detail::win32_device_info::win32_device_info
 */
trrojan::sysinfo::detail::win32_device_info::win32_device_info(HDEVINFO hDev,
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


/*
 * trrojan::sysinfo::detail::win32_device_info::win32_device_info
 */
trrojan::sysinfo::detail::win32_device_info::win32_device_info(
        const device_info *rhs) {
    assert(rhs != nullptr);
    auto& r = dynamic_cast<const win32_device_info&>(*rhs);
    this->_driver_date = r._driver_date;
    this->_driver_provider = r._driver_provider;
    this->_driver_version = r._driver_version;
    this->_hardware_id = r._hardware_id;
    this->_manufacturer = r._manufacturer;
    this->_name = r._name;
}

#endif /* defined(_WIN32) */
