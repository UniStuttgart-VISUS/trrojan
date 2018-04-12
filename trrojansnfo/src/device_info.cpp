/// <copyright file="device_info.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/sysinfo/device_info.h"

#include <stdexcept>

#include "win32_device_info.h"



#if defined(_WIN32)
/// <summary>
/// Initialises a new instance.
/// </summary>
trrojan::sysinfo::device_info::device_info(HDEVINFO hDev,
        SP_DEVINFO_DATA& data)
    : impl(new detail::win32_device_info(hDev, data)) { }
#endif /* defined(_WIN32) */


/*
 * trrojan::sysinfo::device_info::~device_info
 */
trrojan::sysinfo::device_info::~device_info(void) {
    delete this->impl;
}


/*
 * trrojan::sysinfo::device_info::operator =
 */
trrojan::sysinfo::device_info& trrojan::sysinfo::device_info::operator =(
        const device_info & rhs) {
    if (this != std::addressof(rhs)) {
        delete this->impl;

#if defined(_WIN32)
        this->impl = new detail::win32_device_info(rhs.impl);
#else /* defined(_WIN32) */
        throw std::runtime_error("TODO: IMPLEMENTATION MISSING!");
#endif /* defined(_WIN32) */
    }

    return *this;
}


/*
 * trrojan::sysinfo::device_info::operator =
 */
trrojan::sysinfo::device_info& trrojan::sysinfo::device_info::operator=(
        device_info && rhs) {
    if (this != std::addressof(rhs)) {
        this->impl = rhs.impl;
        rhs.impl = nullptr;
    }

    return *this;
 }
