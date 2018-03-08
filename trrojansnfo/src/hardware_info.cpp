/// <copyright file="hardware_info.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/sysinfo/hardware_info.h"

#include <iostream>

#include "setupapiutil.h"


/*
 * trrojan::sysinfo::hardware_info::_DOWEL
 */
void trrojan::sysinfo::hardware_info::_DOWEL(void) {
    //detail::enum_class_devices(nullptr, [](HDEVINFO hDev, SP_DEVINFO_DATA& data) {
    //    auto name = detail::get_device_registry_property(hDev, data, SPDRP_DEVICEDESC);
    //    auto n = reinterpret_cast<char *>(name.data());
    //    std::cout << n << std::endl;
    //    return true;
    //}, DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE);

    //auto ID = trrojan::sysinfo::detail::display_device_class;
    auto ID = GUID_DEVINTERFACE_DISK;

    detail::enum_class_devices(&ID, [&ID](HDEVINFO hDev, SP_DEVINFO_DATA& data) {
        auto desc= detail::get_device_registry_property(hDev, data, SPDRP_DEVICEDESC);
        auto name = detail::get_device_registry_property(hDev, data, SPDRP_FRIENDLYNAME);
        auto hwid = detail::get_device_registry_property(hDev, data, SPDRP_HARDWAREID);
        auto mfg = detail::get_device_registry_property(hDev, data, SPDRP_MFG);
        auto svc = detail::get_device_registry_property(hDev, data, SPDRP_SERVICE);
        auto drv = detail::get_device_registry_property(hDev, data, SPDRP_DRIVER);

        std::cout << reinterpret_cast<TCHAR *>(name.data()) << std::endl
            << reinterpret_cast<TCHAR *>(desc.data()) << std::endl
            << reinterpret_cast<TCHAR *>(hwid.data()) << std::endl
            << reinterpret_cast<TCHAR *>(mfg.data()) << std::endl
            << reinterpret_cast<TCHAR *>(svc.data()) << std::endl
            << reinterpret_cast<TCHAR *>(drv.data()) << std::endl
            << std::endl;

        detail::enum_device_interfaces(hDev, data, ID, [](HDEVINFO hDev, SP_DEVINFO_DATA& d, SP_DEVICE_INTERFACE_DATA& i) {
            auto detail = detail::get_device_interface_detail(hDev, i);
            //auto name = detail::get_device_registry_property(hDev, i, SPDRP_DEVICEDESC);
            //auto n = reinterpret_cast<char *>(name.data());
            //std::cout << "\t" << n << std::endl;


            return true;
        });

        return true;
    }, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
}
