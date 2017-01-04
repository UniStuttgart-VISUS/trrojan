/// <copyright file="constants.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <climits>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Provides some useful constants.
    /// </summary>
    template<class T> class constants {

    public:

        /// <summary>
        /// Specifies the type to retrieve the constant as.
        /// </summary>
        typedef T type;

        /// <summary>
        /// Specifies the number of bits in a byte.
        /// </summary>
        static const type bits_per_byte;

        /// <summary>
        /// Specifies the number of bytes in a Gigabyte.
        /// </summary>
        static const type bytes_per_gigabyte;

        /// <summary>
        /// Specifies the number of bytes in a kilobyte.
        /// </summary>
        static const type bytes_per_kilobyte;

        /// <summary>
        /// Specifies the number of bytes in a Megabyte.
        /// <summary>
        static const type bytes_per_megabyte;

        /// <summary>
        /// Specifies the number milliseconds per second.
        /// </summary>
        static const type millis_per_second;

        /// <summary>
        /// Specifies the number seconds per hour.
        /// </summary>
        static const type seconds_per_hour;

        /// <summary>
        /// Specifies the number seconds per minute.
        /// </summary>
        static const type seconds_per_minute;

        constants(void) = delete;

        ~constants(void) = delete;
    };
}

#include "trrojan/constants.inl"
