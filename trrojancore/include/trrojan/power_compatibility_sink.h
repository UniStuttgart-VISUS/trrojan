// <copyright file="power_compatibility_sink.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for more information.
// </copyright>
// <author>Christoph Müller</author>

#if !defined(_TRROJAN_POWER_COMPATIBILITY_SINK_H)
#define _TRROJAN_POWER_COMPATIBILITY_SINK_H
#pragma once

#include <atomic>
#include <cassert>
#include <fstream>
#include <stdexcept>

#include <visus/pwrowg/sensor_description.h>

#include "trrojan/export.h"
#include "trrojan/text.h"


namespace trrojan {

    /// <summary>
    /// A sink generating CSV output compatible with the output format of
    /// <see cref="power_collector" /> for Power Overwhelming 1.x.
    /// </summary>
    class TRROJANCORE_API power_compatibility_sink {

    public:

        explicit power_compatibility_sink(_In_z_ const char *path);

        /// <summary>
        /// Sets the unique identifier of the current measurement series.
        /// </summary>
        /// <param name="uid"></param>
        inline void power_uid(_In_ const std::uint64_t uid) {
            this->_power_uid.store(uid, std::memory_order_release);
        }

    protected:

        /// <summary>
        /// Write the given range of samples to the <see cref="_stream" />.
        /// </summary>
        template<class TIterator>
        void write_samples(_In_ const TIterator begin,
            _In_ const TIterator end,
            _In_ const visus::pwrowg::sensor_description *sensors);

    private:

        char _delimiter;
        bool _first;
        std::atomic<std::uint64_t> _power_uid;
        std::ofstream _stream;
    };

} /* namespace trrojan */

#include "trrojan/power_compatibility_sink.inl"

#endif /* !defined(_TRROJAN_POWER_COMPATIBILITY_SINK_H) */
