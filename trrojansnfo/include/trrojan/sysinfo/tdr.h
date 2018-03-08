/// <copyright file="tdr.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <limits>

#if defined(_WIN32)
#include <Windows.h>
#endif /* defined(_WIN32) */


namespace trrojan {
namespace sysinfo {

namespace detail {

#if defined(_WIN32)
    typedef DWORD tdr_underlying_type;
#else /* defined(_WIN32) */
    typedef std::uint32_t tdr_underlying_type;
#endif /* defined(_WIN32) */

} /* end namespace detail */


    /// <summary>
    /// Lists debugging-related behaviours of the TDR process.
    /// </summary>
    enum class tdr_debug_mode : detail::tdr_underlying_type {

        /// <summary>
        /// The debugging-related behaviour is unknown.
        /// </summary>
        unknown = (std::numeric_limits<detail::tdr_underlying_type>::max)(),

        /// <summary>
        /// Break to kernel debugger before the recovery to allow investigation
        /// of the timeout.
        /// </summary>
        off = static_cast<detail::tdr_underlying_type>(0),

        /// <summary>
        /// Ignore any timeout.
        /// </summary>
        ignore_timeout = static_cast<detail::tdr_underlying_type>(1),

        /// <summary>
        /// Recover without breaking into the debugger.
        /// </summary>
        recover_no_prompt = static_cast<detail::tdr_underlying_type>(2),

        /// <summary>
        /// Recover even if some recovery conditions are not met (eg recover on
        /// consecutive timeouts).
        /// </summary>
        recover_unconditional = static_cast<detail::tdr_underlying_type>(3),

        /// <summary>
        /// The default behaviour according to
        /// https://docs.microsoft.com/en-us/windows-hardware/drivers/display/tdr-registry-keys
        /// </summary>
        default_behaviour = static_cast<detail::tdr_underlying_type>(2)
    };

    /// <summary>
    /// Lists possible GPU timeout detection and recovery levels.
    /// </summary>
    enum class tdr_level : detail::tdr_underlying_type {

        /// <summary>
        /// The state of timeout detection is unknown.
        /// </summary>
        unknown = (std::numeric_limits<detail::tdr_underlying_type>::max)(),

        /// <summary>
        /// Timeout detection is disabled.
        /// </summary>
        off = static_cast<detail::tdr_underlying_type>(0),

        /// <summary>
        /// The system bug checks (bluescreens) on timeout.
        /// </summary>
        bug_check = static_cast<detail::tdr_underlying_type>(1),

        /// <summary>
        /// Recover to VGA on timeout.
        /// </summary>
        recover_vga = static_cast<detail::tdr_underlying_type>(2),

        /// <summary>
        /// Fully recover on timeout.
        /// </summary>
        recover = static_cast<detail::tdr_underlying_type>(3),

        /// <summary>
        /// The default level according to
        /// https://docs.microsoft.com/en-us/windows-hardware/drivers/display/tdr-registry-keys
        /// </summary>
        default_behaviour = static_cast<detail::tdr_underlying_type>(3)
    };

} /* end namespace trrojan */
} /* end namespace sysinfo */
