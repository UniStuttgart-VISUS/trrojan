/// <copyright file="timer.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/export.h"

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
#endif /* _WIN32 */


namespace trrojan {

    /// <summary>
    /// A utility class for measuring wall clock times.
    /// </summary>
    class TRROJANCORE_API timer {

    public:

        /// <summary>
        /// Represents the difference between two points in time.
        /// </summary>
#if (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK))
        typedef std::int64_t difference_type;
#else /* (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK)) */
        typedef std::chrono::high_resolution_clock::duration difference_type;
#endif /* (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK)) */

        /// <summary>
        /// The type to represent milliseconds.
        /// </summary>
        typedef double millis_type;

        /// <summary>
        /// Represents a point in time.
        /// </summary>
#if (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK))
        typedef std::uint64_t value_type;
#else /* (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK)) */
        typedef std::chrono::high_resolution_clock::time_point value_type;
#endif /* (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK)) */

        /// <summary>
        /// Answer the times's current native value.
        /// </summary>
        /// <returns>The current timer value.</returns>
        static inline value_type now(void) {
#if (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK))
            LARGE_INTEGER retval;
            if (::QueryPerformanceCounter(&retval)) {
                return retval.QuadPart;
            } else {
                std::error_code ec(::GetLastError(), std::system_category());
                throw std::system_error(ec, "Failed to query performance "
                    "counter.");
            }
#else /* (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK)) */
            return std::chrono::high_resolution_clock::now();
#endif /* (defined(_WIN32) && !defined(TRROJAN_FORCE_STL_CLOCK)) */
        }

        /// <summary>
        /// Converts a point in time to milliseconds since the epoch.
        /// </summary>
        /// <param name="value">The value to be converted.</param>
        /// <returns>The value as milliseconds</returns>
        static millis_type millis_since_epoch(const value_type value);

        /// <summary>
        /// Converts a time difference to milliseconds.
        /// </summary>
        /// <param name="value">The value to be converted.</param>
        /// <returns>The value as milliseconds</returns>
        static millis_type to_millis(const difference_type value);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline timer(void) { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        inline ~timer(void) { }

        /// <summary>
        /// Answer the elapsed time since the last start mark.
        /// </summary>
        /// <returns>The elapsed time.</returns>
        inline difference_type elapsed(void) const {
            return (timer::now() - this->mark);
        }

        /// <summary>
        /// Answer the elapsed time since the last start mark in milliseconds.
        /// </summary>
        /// <returns>The elapsed time.</returns>
        inline millis_type elapsed_millis(void) const {
            return timer::to_millis(this->elapsed());
        }

        /// <summary>
        /// Sets a new start mark.
        /// </summary>
        /// <returns>The value of the start mark.</returns>
        inline value_type start(void) {
            this->mark = timer::now();
            return this->mark;
        }

    private:

        /// <summary>
        /// The mark, i.e. the clock value when the last measurement was
        /// started.
        /// </summary>
        value_type mark;

    };

}
