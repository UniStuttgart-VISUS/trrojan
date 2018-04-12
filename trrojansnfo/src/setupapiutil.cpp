/// <copyright file="setupapiutil.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "setupapiutil.h"

#include <stdexcept>
#include <system_error>


#if defined(_WIN32)
/*
 * trrojan::sysinfo::detail::add_device_install_flags
 */
void trrojan::sysinfo::detail::add_device_install_flags(HDEVINFO hDevInfo,
        SP_DEVINFO_DATA *devInfo, const DWORD flags, const DWORD flagsEx) {
    SP_DEVINSTALL_PARAMS params;
    ::ZeroMemory(&params, sizeof(params));
    params.cbSize = sizeof(params);

    if (!::SetupDiGetDeviceInstallParams(hDevInfo, devInfo, &params)) {
        auto ec = std::error_code(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to get device installation "
            "parameters.");
    }

    params.Flags |= flags;
    params.FlagsEx |= flagsEx;

    if (!::SetupDiSetDeviceInstallParams(hDevInfo, devInfo, &params)) {
        auto ec = std::error_code(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to set device installation "
            "parameters.");
    }
}


/*
 * trrojan::sysinfo::detail::enum_class_devices
 */
size_t trrojan::sysinfo::detail::enum_class_devices(const GUID *class_guid,
        const enum_class_devices_cb& cb, const DWORD flags) {
    SP_DEVINFO_DATA devInfo;
    DWORD i = 0;

    auto hDevInfo = ::SetupDiGetClassDevs(class_guid, nullptr, NULL, flags);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        auto ec = std::error_code(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to get class devices.");
    }

    // See http://support.microsoft.com/kb/259695/de
    ::ZeroMemory(&devInfo, sizeof(devInfo));
    devInfo.cbSize = sizeof(devInfo);
    for (i = 0; ::SetupDiEnumDeviceInfo(hDevInfo, i, &devInfo); ++i) {
        if (!cb(hDevInfo, devInfo)) {
            break;
        }
    } /* end  for (i = 0; ::SetupDiEnumDeviceInfo(... */

    auto error = ::GetLastError();
    ::SetupDiDestroyDeviceInfoList(hDevInfo);
    if (error != ERROR_NO_MORE_ITEMS) {
        auto ec = std::error_code(error, std::system_category());
        throw std::system_error(ec, "Failed to enumerate class devices.");
    }

    return static_cast<size_t>(i);
}


/*
 * trrojan::sysinfo::detail::enum_device_interfaces
 */
size_t trrojan::sysinfo::detail::enum_device_interfaces(HDEVINFO hDevInfo,
        SP_DEVINFO_DATA& devInfo, const GUID& interfaceGuid,
        const enum_device_interfaces_cb& cb) {
    SP_DEVICE_INTERFACE_DATA devIfData;
    DWORD i = 0;

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        throw std::invalid_argument("A valid device info handle must be "
            "supplied.");
    }

    ::ZeroMemory(&devIfData, sizeof(devIfData));
    devIfData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    for (i = 0; ::SetupDiEnumDeviceInterfaces(hDevInfo, &devInfo,
            &interfaceGuid, i, &devIfData); ++i) {
        if (!cb(hDevInfo, devInfo, devIfData)) {
            break;
        }
    } /* end  for (i = 0; ::SetupDiEnumDeviceInterfaces(... */

    auto error = ::GetLastError();
    if (error != ERROR_NO_MORE_ITEMS) {
        auto ec = std::error_code(error, std::system_category());
        throw std::system_error(ec, "Failed to enumerate device interfaces.");
    }

    return static_cast<size_t>(i);
}


/*
 * trrojan::sysinfo::detail::enum_driver_info
 */
size_t trrojan::sysinfo::detail::enum_driver_info(HDEVINFO hDevInfo,
        SP_DEVINFO_DATA *devInfo, const DWORD driverType,
        const enum_driver_info_cb& cb) {
    SP_DRVINFO_DATA driverInfo;
    DWORD i = 0;

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        throw std::invalid_argument("A valid device info handle must be "
            "supplied.");
    }

    if (!::SetupDiBuildDriverInfoList(hDevInfo, devInfo, driverType)) {
        auto ec = std::error_code(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to build driver list.");
    }

    ::ZeroMemory(&driverInfo, sizeof(driverInfo));
    driverInfo.cbSize = sizeof(driverInfo);
    for (i = 0; ::SetupDiEnumDriverInfo(hDevInfo, devInfo, driverType, i,
            &driverInfo); ++i) {
        if (!cb(hDevInfo, devInfo, driverInfo)) {
            break;
        }
    } /* end  for (i = 0; ::SetupDiEnumDriverInfo(... */

    auto error = ::GetLastError();
    if (error != ERROR_NO_MORE_ITEMS) {
        auto ec = std::error_code(error, std::system_category());
        throw std::system_error(ec, "Failed to enumerate device drivers.");
    }

    return static_cast<size_t>(i);
}


/*
 * trrojan::sysinfo::detail::get_device_interface_detail
 */
std::vector<std::uint8_t> trrojan::sysinfo::detail::get_device_interface_detail(
        HDEVINFO hDevInfo, SP_DEVICE_INTERFACE_DATA& devIfData,
        SP_DEVINFO_DATA *outDevInfo) {
    DWORD size = 0;

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        throw std::invalid_argument("A valid device info handle must be "
            "supplied.");
    }

    /* Retrieve required buffer size. */
    ::SetupDiGetDeviceInterfaceDetail(hDevInfo, &devIfData, nullptr, 0, &size,
        nullptr);
    std::vector<std::uint8_t> retval(size);
    auto r = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(retval.data());
    r->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    /* Retrieve the data. */
    if (!::SetupDiGetDeviceInterfaceDetail(hDevInfo, &devIfData, r,
            static_cast<DWORD>(retval.size()), &size, outDevInfo)) {
        auto ec = std::error_code(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to retrieve device interface "
            "details.");
    }

    return retval;
}


/*
 * trrojan::sysinfo::detail::get_device_registry_property
 */
std::vector<std::uint8_t>
trrojan::sysinfo::detail::get_device_registry_property(HDEVINFO hDevInfo,
        SP_DEVINFO_DATA& devInfo, const DWORD property, DWORD *outRegDataType) {
    DWORD size = 0;

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        throw std::invalid_argument("A valid device info handle must be "
            "supplied.");
    }

    /* Retrieve required buffer size. */
    ::SetupDiGetDeviceRegistryProperty(hDevInfo, &devInfo, property, nullptr,
        nullptr, 0, &size);
    std::vector<std::uint8_t> retval(size);

    /* Retrieve the data. */
    if (!::SetupDiGetDeviceRegistryProperty(hDevInfo, &devInfo, property,
            outRegDataType, reinterpret_cast<BYTE *>(retval.data()),
            static_cast<DWORD>(retval.size()), &size)) {
        auto ec = std::error_code(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to retrieve device property.");
    }

    return retval;
}


/*
 * trrojan::sysinfo::detail::split_driver_version
 */
std::array<std::uint16_t, 4> trrojan::sysinfo::detail::split_driver_version(
        decltype(SP_DRVINFO_DATA::DriverVersion) version) {
    auto revision = static_cast<std::uint16_t>(version & 0xFFFF);
    version >>= 16;
    auto build = static_cast<std::uint16_t>(version & 0xFFFF);
    version >>= 16;
    auto minor = static_cast<std::uint16_t>(version & 0xFFFF);
    version >>= 16;
    auto major = static_cast<std::uint16_t>(version & 0xFFFF);
    version >>= 16;

    return { major, minor, build, revision };
}
#endif /* defined(_WIN32) */



#if 0
DWORD bufferSize = 0;
SP_DEVINFO_DATA devInfo;
std::vector<TCHAR> deviceName(MAX_PATH);
std::unordered_map<tstring, DiagnosticResult> results;

out << _T("Retrieving class devices ...") << std::endl;
auto hDevInfo = ::SetupDiGetClassDevs(&CLASS_DISPLAY, nullptr, NULL,
    DIGCF_PRESENT);
if (hDevInfo == INVALID_HANDLE_VALUE) {
    throw std::system_error(std::error_code(::GetLastError(),
        std::system_category()));
}

out << _T("Retrieving device information ...") << std::endl;
// See http://support.microsoft.com/kb/259695/de
::ZeroMemory(&devInfo, sizeof(devInfo));
devInfo.cbSize = sizeof(devInfo);
for (DWORD i = 0; ::SetupDiEnumDeviceInfo(hDevInfo, i, &devInfo); ++i) {
    out << _T("Retrieving device name ...") << std::endl;
    if (!::SetupDiGetDeviceRegistryProperty(hDevInfo, &devInfo,
        SPDRP_DEVICEDESC, nullptr,
        reinterpret_cast<BYTE *>(deviceName.data()),
        static_cast<DWORD>(deviceName.size() * sizeof(TCHAR)),
        &bufferSize)) {
        // Buffer was too small, try again.
        deviceName.resize(bufferSize / sizeof(TCHAR));
        if (!::SetupDiGetDeviceRegistryProperty(hDevInfo, &devInfo,
            SPDRP_DEVICEDESC, nullptr,
            reinterpret_cast<BYTE *>(deviceName.data()),
            static_cast<DWORD>(deviceName.size() * sizeof(TCHAR)),
            nullptr)) {
            ::_tcscpy_s(deviceName.data(), deviceName.size(),
                _T("Unknown device"));
        }
    }

    auto it = results.find(deviceName.data());
    if (it == results.end()) {
        DiagnosticResult r(this->Name(), deviceName.data());
        results.insert(std::make_pair(deviceName.data(), r));
        it = results.find(deviceName.data());
    }

    it->second.IncrementCount();
} /* end  for (DWORD i = 0; ::SetupDiEnumDeviceInfo(... */

::SetupDiDestroyDeviceInfoList(hDevInfo);

for (auto it : results) {
    *oit++ = it.second;
}
#endif


#if 0
list<wstring> GetPhysicalDisks()
{
    HDEVINFO hDeviceInfoSet;
    ULONG ulMemberIndex;
    ULONG ulErrorCode;
    BOOL bFound = FALSE;
    BOOL bOk;
    list<wstring> disks;

    // create a HDEVINFO with all present devices
    hDeviceInfoSet = ::SetupDiGetClassDevs(&GUID_DEVINTERFACE_DISK, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDeviceInfoSet == INVALID_HANDLE_VALUE) {
        _ASSERT(FALSE);
        return disks;
    }

    // enumerate through all devices in the set
    ulMemberIndex = 0;
    while (TRUE)
    {
        // get device info
        SP_DEVINFO_DATA deviceInfoData;
        deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        if (!::SetupDiEnumDeviceInfo(hDeviceInfoSet, ulMemberIndex, &deviceInfoData))
        {
            if (::GetLastError() == ERROR_NO_MORE_ITEMS) {
                // ok, reached end of the device enumeration
                break;
            } else {
                // error
                _ASSERT(FALSE);
                ::SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
                return disks;
            }
        }

        // get device interfaces
        SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
        deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        if (!::SetupDiEnumDeviceInterfaces(hDeviceInfoSet, NULL, &GUID_DEVINTERFACE_DISK, ulMemberIndex, &deviceInterfaceData))
        {
            if (::GetLastError() == ERROR_NO_MORE_ITEMS) {
                // ok, reached end of the device enumeration
                break;
            } else {
                // error
                _ASSERT(FALSE);
                ::SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
                return disks;
            }
        }

        // process the next device next time 
        ulMemberIndex++;

        // get hardware id of the device
        ULONG ulPropertyRegDataType = 0;
        ULONG ulRequiredSize = 0;
        ULONG ulBufferSize = 0;
        BYTE *pbyBuffer = NULL;
        if (!::SetupDiGetDeviceRegistryProperty(hDeviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, &ulPropertyRegDataType, NULL, 0, &ulRequiredSize))
        {
            if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                pbyBuffer = (BYTE *)::malloc(ulRequiredSize);
                ulBufferSize = ulRequiredSize;
                if (!::SetupDiGetDeviceRegistryProperty(hDeviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, &ulPropertyRegDataType, pbyBuffer, ulBufferSize, &ulRequiredSize))
                {
                    // getting the hardware id failed
                    _ASSERT(FALSE);
                    ::SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
                    ::free(pbyBuffer);
                    return disks;
                }
            } else {
                // getting device registry property failed
                _ASSERT(FALSE);
                ::SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
                return disks;
            }
        } else {
            // getting hardware id of the device succeeded unexpectedly
            _ASSERT(FALSE);
            ::SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
            return disks;
        }

        // pbyBuffer is initialized now!
        LPCWSTR pszHardwareId = (LPCWSTR)pbyBuffer;

        // retrieve detailed information about the device
        // (especially the device path which is needed to create the device object)
        SP_DEVICE_INTERFACE_DETAIL_DATA *pDeviceInterfaceDetailData = NULL;
        ULONG ulDeviceInterfaceDetailDataSize = 0;
        ulRequiredSize = 0;
        bOk = ::SetupDiGetDeviceInterfaceDetail(hDeviceInfoSet, &deviceInterfaceData, pDeviceInterfaceDetailData, ulDeviceInterfaceDetailDataSize, &ulRequiredSize, NULL);
        if (!bOk)
        {
            ulErrorCode = ::GetLastError();
            if (ulErrorCode == ERROR_INSUFFICIENT_BUFFER) {
                // insufficient buffer space
                // => that's ok, allocate enough space and try again
                pDeviceInterfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA *)::malloc(ulRequiredSize);
                pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                ulDeviceInterfaceDetailDataSize = ulRequiredSize;
                deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                bOk = ::SetupDiGetDeviceInterfaceDetail(hDeviceInfoSet, &deviceInterfaceData, pDeviceInterfaceDetailData, ulDeviceInterfaceDetailDataSize, &ulRequiredSize, &deviceInfoData);
                ulErrorCode = ::GetLastError();
            }

            if (!bOk) {
                // retrieving detailed information about the device failed
                _ASSERT(FALSE);
                ::free(pbyBuffer);
                ::free(pDeviceInterfaceDetailData);
                ::SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
                return disks;
            }
        } else {
            // retrieving detailed information about the device succeeded unexpectedly
            _ASSERT(FALSE);
            ::free(pbyBuffer);
            ::SetupDiDestroyDeviceInfoList(hDeviceInfoSet);
            return disks;
        }

        disks.push_back(pDeviceInterfaceDetailData->DevicePath);

        // free buffer for device interface details
        ::free(pDeviceInterfaceDetailData);

        // free buffer
        ::free(pbyBuffer);
    }

    // destroy device info list
    ::SetupDiDestroyDeviceInfoList(hDeviceInfoSet);

    return disks;
}
#endif
