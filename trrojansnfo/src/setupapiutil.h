/// <copyright file="setupapiutil.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#if defined(WIN32)
#include <cinttypes>
#include <functional>
#include <vector>

#include <Windows.h>
#include <SetupAPI.h>


namespace trrojan {
namespace sysinfo {
namespace detail {

    typedef std::function<bool(HDEVINFO, SP_DEVINFO_DATA&)>
        enum_class_devices_cb;

    typedef std::function<bool(HDEVINFO, SP_DEVINFO_DATA&,
        SP_DEVICE_INTERFACE_DATA&)> enum_device_interfaces_cb;

    /// <summary>
    /// The GUID of the display devices class, which is not defined in the
    /// public header of SetupAPI.
    /// </summary>
    extern const GUID display_device_class;

    /// <summary>
    /// Invoke <paramref name="cb" /> for all devices of the given class.
    /// </summary>
    /// <remarks>
    /// <para>The function continues enumerating devices as long as there are
    /// more devices and <paramref cref="cb" /> returns <c>true</c>.</para>
    /// <para><paramref name="cb" /> should not throw exceptions. Failing to
    /// fulfil this requirement might result in a memory leak.</para>
    /// </remarks>
    /// <param name="class_guid">A pointer to the class GUID to enumerate or
    /// <c>nullptr</c> to enumerate all classes in combination with the
    /// <c>DIGCF_ALLCLASSES</c> flag.</param>
    /// <param name="cb">The callback to be invoked for all results.</param>
    /// <param name="flags">The enumeration flagsas described on
    /// https://msdn.microsoft.com/en-us/library/windows/hardware/ff551069(v=vs.85).aspx
    /// The parameter defaults to <c>DIGCF_PRESENT</c>.</param>
    /// <returns>The number of elements that have been enumerated.</returns>
    size_t enum_class_devices(const GUID *class_guid, const enum_class_devices_cb& cb,
        const DWORD flags = DIGCF_PRESENT);

    /// <summary>
    /// Invokes a callback for all device interfaces in a device information
    /// set.
    /// </summary>
    /// <param name="hDevInfo"></param>
    /// <param name="devInfo"></param>
    /// <param name="interfaceGuid"></param>
    /// <param name="cb"></param>
    /// <returns></returns>
    size_t enum_device_interfaces(HDEVINFO hDevInfo, SP_DEVINFO_DATA& devInfo,
        const GUID& interfaceGuid, const enum_device_interfaces_cb& cb);

    std::vector<std::uint8_t> get_device_interface_detail(HDEVINFO hDevInfo,
        SP_DEVICE_INTERFACE_DATA& devIfData,
        SP_DEVINFO_DATA *outDevInfo = nullptr);

    /// <summary>
    /// Retrieves a registry-stored property of the device identified by
    /// <paramref name="hDevInfo" /> and <paramref name="devInfo" />.
    /// </summary>
    /// <param name="hDevInfo">A handle to a device information set that
    /// contains a device information element that represents the device for
    /// which to retrieve a property.</param>
    /// <param name="devInfo">A pointer to an <c>SP_DEVINFO_DATA</c> structure
    /// from <paramref name="hDevInfo" /> which specifies the device information
    /// element.</param>
    /// <param name="property">The property to be retrieved. Valid values can be
    /// found at
    /// https://msdn.microsoft.com/en-us/library/windows/hardware/ff551967(v=vs.85).aspx
    /// </param>
    /// <param name="outRegDataType">If not <c>nullptr</c>, retrieves the registry
    /// type of the data returned.</param>
    /// <returns></returns>
    std::vector<std::uint8_t> get_device_registry_property(HDEVINFO hDevInfo,
        SP_DEVINFO_DATA& devInfo, const DWORD property,
        DWORD *outRegDataType = nullptr);

} /* end detail */
} /* end sysinfo */
} /* end trrojan */

#endif /* defined(WIN32) */
