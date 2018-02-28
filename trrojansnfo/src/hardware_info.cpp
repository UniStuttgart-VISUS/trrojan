/// <copyright file="hardware_info.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/sysinfo/hardware_info.h"


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
        LPCWSTR pszHardwareId = (LPCWSTR) pbyBuffer;

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
