/// <copyright file="os_info.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/sysinfo/os_info.h"

#include <cassert>
#include <limits>
#include <memory>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#include <lmcons.h>
#else /* _WIN32 */
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#endif /* _WIN32 */

#include "os_info_impl.h"
#include "utilities.h"


/*
 * trrojan::sysinfo::os_info::collect
 */
trrojan::sysinfo::os_info trrojan::sysinfo::os_info::collect(void) {
    os_info retval;
    retval.impl = new detail::os_info_impl();
    retval.impl->update();
    return retval;
}


/*
 * trrojan::sysinfo::os_info::name
 */
const char *const trrojan::sysinfo::os_info::name(void) const {
    return (*this) ? this->impl->name.c_str() : nullptr;
}


/*
 * trrojan::sysinfo::os_info::tdr_ddi_delay
 */
size_t trrojan::sysinfo::os_info::tdr_ddi_delay(void) const {
    return (*this) ? this->impl->tdr_ddi_delay : invalid_tdr_value;
}


/*
 * trrojan::sysinfo::os_info::tdr_debug_mode
 */
trrojan::sysinfo::tdr_debug_mode trrojan::sysinfo::os_info::tdr_debug_mode(
        void) const {
    return (*this) ? this->impl->tdr_debug_mode : sysinfo::tdr_debug_mode::unknown;
}


/*
 * trrojan::sysinfo::os_info::tdr_delay
 */
size_t trrojan::sysinfo::os_info::tdr_delay(void) const {
    return (*this) ? this->impl->tdr_delay : invalid_tdr_value;
}


/*
 * trrojan::sysinfo::os_info::tdr_level
 */
trrojan::sysinfo::tdr_level trrojan::sysinfo::os_info::tdr_level(void) const {
    return (*this) ? this->impl->tdr_level : sysinfo::tdr_level::unknown;
}

/*
 * trrojan::sysinfo::os_info::tdr_limit_time
 */
size_t trrojan::sysinfo::os_info::tdr_limit_count(void) const {
    return (*this) ? this->impl->tdr_limit_count : invalid_tdr_value;
}


/*
 * trrojan::sysinfo::os_info::tdr_limit_time
 */
size_t trrojan::sysinfo::os_info::tdr_limit_time(void) const {
    return (*this) ? this->impl->tdr_limit_time : invalid_tdr_value;
}


/*
 * trrojan::sysinfo::os_info::version
 */
const char * const trrojan::sysinfo::os_info::version(void) const {
    return (*this) ? this->impl->version.c_str() : nullptr;
}


/*
 * trrojan::sysinfo::os_info::word_size
 */
size_t trrojan::sysinfo::os_info::word_size(void) const{
    return (*this) ? this->impl->word_size : 0;
}


/*
 * trrojan::sysinfo::os_info::invalid_tdr_value
 */
const size_t trrojan::sysinfo::os_info::invalid_tdr_value
    = (std::numeric_limits<size_t>::max)();


/*
 * trrojan::sysinfo::os_info::~os_info
 */
trrojan::sysinfo::os_info::~os_info(void) {
    delete this->impl;
}


/*
 * trrojan::sysinfo::os_info::operator =
 */
trrojan::sysinfo::os_info& trrojan::sysinfo::os_info::operator =(
        const os_info& rhs) {
    if (this != std::addressof(rhs)) {
        delete this->impl;
        this->impl = rhs ? new detail::os_info_impl(*rhs.impl) : nullptr;
    }

    return *this;
}


/*
 * trrojan::sysinfo::os_info::operator =
 */
trrojan::sysinfo::os_info& trrojan::sysinfo::os_info::operator =(
        os_info&& rhs) {
    if (this != std::addressof(rhs)) {
        this->impl = rhs.impl;
        rhs.impl = nullptr;
    }

    return *this;
}
