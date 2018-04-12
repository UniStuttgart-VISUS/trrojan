/// <copyright file="hardware_info.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/sysinfo/hardware_info.h"

#include "win32_hardware_info.h"


/*
 * trrojan::sysinfo::hardware_info::collect
 */
trrojan::sysinfo::hardware_info trrojan::sysinfo::hardware_info::collect(void) {
    hardware_info retval;
    return retval;
}


/*
 * trrojan::sysinfo::hardware_info::~hardware_info
 */
trrojan::sysinfo::hardware_info::~hardware_info(void) {
    delete this->impl;
}


/*
 * trrojan::sysinfo::hardware_info::operator =
 */
trrojan::sysinfo::hardware_info& trrojan::sysinfo::hardware_info::operator =(
        const hardware_info & rhs) {
    if (this != std::addressof(rhs)) {

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
