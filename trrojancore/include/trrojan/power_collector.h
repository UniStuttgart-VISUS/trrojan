// <copyright file="power_collector.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once
#if defined(TRROJAN_WITH_POWER_OVERWHELMING)

#include <array>
#include <atomic>
#include <chrono>
#include <fstream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "trrojan/export.h"


// Forward declarations.
namespace trrojan { class configuration; }
namespace trrojan { namespace detail { struct power_details; } }


namespace trrojan {

    /// <summary>
    /// A utility class for sampling power sensors.
    /// </summary>
    class TRROJANCORE_API power_collector final {

    public:

        /// <summary>
        /// The type to express the sampling intervals in.
        /// </summary>
        typedef std::chrono::milliseconds interval_type;

        /// <summary>
        /// A pointer type by which the collector is referenced.
        /// </summary>
        typedef std::shared_ptr<power_collector> pointer;

        /// <summary>
        /// Gets, if any, the power collector for the given
        /// <pararmef name="configuration" />.
        /// </summary>
        /// <param name="c"></param>
        /// <returns></returns>
        static pointer get(const configuration& configuration);

        /// <summary>
        /// The column delimiter.
        /// </summary>
        static const char delimiter;

        /// <summary>
        /// The default name of an power collection factor, which is
        /// &quot;powerlog&quot;.
        /// </summary>
        static const char *factor_name;

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        power_collector(void);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~power_collector(void);

        /// <summary>
        /// Generate a new unique identifier and set it as the description for
        /// the current measurement.
        /// </summary>
        /// <remarks>
        /// <para>This method is thread-safe.</para>
        /// </remarks>
        /// <returns></returns>
        std::uint64_t enter_scope(void);

        /// <summary>
        /// Gets the name of the log file the collector is writing to.
        /// </summary>
        /// <returns></returns>
        const std::string& file(void) const {
            return this->_file;
        }

        /// <summary>
        /// Clears the description for the current measurement, which will
        /// temporarily prevent samples from being saved.
        /// </summary>
        /// <remarks>
        /// <para>This method is thread-safe.</para>
        /// </remarks>
        void leave_scope(void);

        /// <summary>
        /// Trigger time sync on sensors with internal clock.
        /// </summary>
        void sync_time(void);

        /// <summary>
        /// Start all sensors with the specified sampling interval.
        /// </summary>
        void start(const std::string& file,
            const interval_type sampling_interval);

        /// <summary>
        /// Stop all sensors and wait for all asynchronous processing to end.
        /// </summary>
        void stop(void);

    private:

        std::unique_ptr<detail::power_details> _details;
        std::string _file;
        std::atomic<std::uint64_t> _next_identifier;
#else /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
        power_collector(void) = delete;
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
    };

} /* end namespace trrojan */

#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
