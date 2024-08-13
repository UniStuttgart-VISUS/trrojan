/// <copyright file="os_info.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <cstdlib>
#include <utility>

#include "trrojan/sysinfo/export.h"
#include "trrojan/sysinfo/tdr.h"


namespace trrojan {
namespace sysinfo {

    /* Forward declarations. */
    namespace detail { struct os_info_impl; }

    /// <summary>
    /// Provides information about the operating system the software is
    /// running on.
    /// </summary>
    class TRROJANSNFO_API os_info {

    public:

        /// <summary>
        /// Indicates an invalid TDR timeout etc.
        /// </summary>
        static const size_t invalid_tdr_value;

        /// <summary>
        /// Collects information about the current operating system.
        /// </summary>
        static os_info collect(void);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline os_info(void) : impl(nullptr) { }

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        inline os_info(const os_info& rhs) : impl(nullptr) {
            *this = rhs;
        }

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        inline os_info(os_info&& rhs) : impl(nullptr) {
            *this = std::move(rhs);
        }

        /// <summary>
        /// Finalise the instance.
        /// </summary>
        ~os_info(void);

        /// <summary>
        /// Gets the name of the operating system.
        /// </summary>
        const char *const name(void) const;

        /// <summary>
        /// Gets the number of seconds that the operating system allows threads
        /// to leave the driver.
        /// </summary>
        size_t tdr_ddi_delay(void) const;

        /// <summary>
        /// Gets the number of seconds that the operating system allows threads
        /// to leave the driver.
        /// </summary>
        sysinfo::tdr_debug_mode tdr_debug_mode(void) const;

        /// <summary>
        /// Gets the number of seconds that the GPU can delay the preempt
        /// request from the GPU scheduler.
        /// </summary>
        size_t tdr_delay(void) const;

        /// <summary>
        /// Gets the system behaviour when a GPU timeout is detected.
        /// </summary>
        sysinfo::tdr_level tdr_level(void) const;

        /// <summary>
        /// Gets the default number of TDRs that are allowed during the time
        /// specified by the <see cref="tdr_limit_time" /> without crashing
        /// the computer.
        /// </summary>
        size_t tdr_limit_count(void) const;

        /// <summary>
        /// Gets the default time within which a specific number of TDRs
        /// (specified by <see cref="tdr_limit_count" />) are allowed 
        /// without crashing the computer.
        /// </summary>
        size_t tdr_limit_time(void) const;

        /// <summary>
        /// Gets the version of the operating system.
        /// </summary>
        const char *const version(void) const;

        /// <summary>
        /// Gets the word size of the operating system.
        /// </summary>
        size_t word_size(void) const;

        os_info& operator =(const os_info& rhs);

        os_info& operator =(os_info&& rhs);

        /// <summary>
        /// Answer whether the OS information is valid.
        /// </summary>
        inline operator bool(void) const {
            return (this->impl != nullptr);
        }

    private:

        detail::os_info_impl *impl;

    };

} /* end namespace trrojan */
} /* end namespace sysinfo */
