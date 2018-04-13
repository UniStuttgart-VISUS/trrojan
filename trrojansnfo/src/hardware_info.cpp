/// <copyright file="hardware_info.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/sysinfo/hardware_info.h"

#include <algorithm>

#include "hardware_info_impl.h"


/*
 * trrojan::sysinfo::hardware_info::collect
 */
trrojan::sysinfo::hardware_info trrojan::sysinfo::hardware_info::collect(void) {
    hardware_info retval;
    retval.impl = new detail::hardware_info_impl();
    retval.impl->update();
    return retval;
}


/*
 * trrojan::sysinfo::hardware_info::~hardware_info
 */
trrojan::sysinfo::hardware_info::~hardware_info(void) {
    delete this->impl;
}


/*
 * trrojan::sysinfo::hardware_info::gpus
 */
bool trrojan::sysinfo::hardware_info::gpus(
        const device_info **outDevices, size_t& inOutCntDevices) const {
    if (this->impl != nullptr) {
        auto cnt = this->impl->gpus.size();
        auto retval = ((outDevices != nullptr) && (inOutCntDevices >= cnt));
        inOutCntDevices = cnt;

        if (retval) {
            std::transform(this->impl->gpus.begin(),
                this->impl->gpus.end(),
                outDevices,
                [](const detail::device_info_impl& d) { return &d; });
        }

        return retval;
    } else {
        // There is nothing to return, so this is a success.
        inOutCntDevices = 0;
        return true;
    }
}


/*
 * trrojan::sysinfo::hardware_info::operator =
 */
trrojan::sysinfo::hardware_info& trrojan::sysinfo::hardware_info::operator =(
        const hardware_info & rhs) {
    if (this != std::addressof(rhs)) {
        delete this->impl;
        this->impl = rhs ? new detail::hardware_info_impl(*rhs.impl) : nullptr;
    }

    return *this;
}


/*
 * trrojan::sysinfo::hardware_info::operator =
 */
trrojan::sysinfo::hardware_info& trrojan::sysinfo::hardware_info::operator=(
        hardware_info && rhs) {
    if (this != std::addressof(rhs)) {
        this->impl = rhs.impl;
        rhs.impl = nullptr;
    }

    return *this;
}
