/// <copyright file="timer.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/timer.h"

#include "trrojan/constants.h"


/*
 * trrojan::timer::millis_since_epoch
 */
trrojan::timer::millis_type trrojan::timer::millis_since_epoch(
        const value_type value) {
#if (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK))
    return timer::to_millis(value);
#else /* (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK)) */
    return timer::to_millis(value.time_since_epoch());
#endif /* (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK)) */
}


/*
 * trrojan::timer::to_millis
 */
trrojan::timer::millis_type trrojan::timer::to_millis(
        const difference_type value) {
#if (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK))
    LARGE_INTEGER frequency;

    if (::QueryPerformanceFrequency(&frequency)) {
        auto v = static_cast<millis_type>(value);
        auto s = trrojan::constants<millis_type>::millis_per_second;
        auto f = static_cast<millis_type>(frequency.QuadPart);
        return ((v * s) / f);

    } else {
        std::error_code ec(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to query performance "
            "counter frequency.");
    }

#else /* (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK)) */
    std::chrono::duration<millis_type, std::milli> v(value);
    return v.count();
#endif /* (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK)) */
}
