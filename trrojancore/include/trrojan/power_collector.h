// <copyright file="power_collector.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once


#include <array>
#include <atomic>
#include <chrono>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
#include "power_overwhelming/collector.h"
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */

#include "trrojan/export.h"


namespace trrojan {

    /* Forward declarations. */
    class configuration;

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
        /// A pointer type by which the collector is referecned.
        /// </summary>
        typedef std::shared_ptr<power_collector> pointer;

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
        /// <summary>
        /// The column delimiter.
        /// </summary>
        static const char delimiter;

        /// <summary>
        /// The default name of an power collection factor, which is
        /// &quot;powerlog&quot;.
        /// </summary>
        static const char *factor_name;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        power_collector(void);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~power_collector(void);

        /// <summary>
        /// Gets the name of the log file the collector is writing to.
        /// </summary>
        /// <returns></returns>
        const std::string& file(void) const {
            return this->_file;
        }

        /// <summary>
        /// Create and return the next unique benchmark identifier.
        /// </summary>
        /// <returns>The next identifier.</returns>
        std::string next_unique_identifier(void);

        /// <summary>
        /// Updates the description of what is currently measured.
        /// </summary>
        /// <remarks>
        /// <para>Setting a new description flushes all data that have been
        /// collected for the previous description to disk.</para>
        /// <para>Setting an empty descriptions will disable the collection
        /// of data until a new non-empty string is set. The sensors will
        /// still run, but all samples will be discarded.</para>
        /// </remarks>
        /// <param name="description"></param>
        void set_description(const std::string& description);

        /// <summary>
        /// Updates the description of what is currently measured to the
        /// given configuration.
        /// </summary>
        /// <remarks>
        /// <para>Setting a new description flushes all data that have been
        /// collected for the previous description to disk.</para>
        /// </remarks>
        /// <param name="config"></param>
        /// <param name="phase"></param>
        void set_description(const configuration& config,
            const std::string& phase);

        /// <summary>
        /// Sets the header text to be written in the first line of the file.
        /// </summary>
        /// <param name="config">The configuration to be included in each line-
        /// </param>
        /// <param name="phase">The name of the phase column, which defaults to
        /// &quot;phase&quot;.</param>
        void set_header(const configuration& config,
            const std::string &phase = "phase");

        /// <summary>
        /// Sets the header text to be written in the first line of the file.
        /// </summary>
        /// <param name="uid">The name of the column of the unique power ID,
        /// which defaults to &quot;power_uid&quot;.</param>
        void set_header(const std::string& uid = "power_uid");

        /// <summary>
        /// Generate a new unique identifier and set it as the description for
        /// the current measurement.
        /// </summary>
        /// <returns></returns>
        inline std::string set_next_unique_description(void) {
            auto retval = this->next_unique_identifier();
            this->set_description(retval);
            return retval;
        }

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

        static void on_measurement(
            const visus::power_overwhelming::measurement& m,
            void *context);

        static void start_hmc8015_sensor(
            visus::power_overwhelming::hmc8015_sensor& sensor);

        void flush_buffer(void);

        void sample(const interval_type sampling_interval);

        void setup_adl_sensors(void);

        void setup_hmc8015_sensors(void);

        void setup_nvml_sensors(void);

        void setup_tinkerforge_sensors(void);

        std::vector<visus::power_overwhelming::adl_sensor> _adl_sensors;
        std::vector<visus::power_overwhelming::measurement> _buffer;
        std::string _description;
        std::string _file;
        std::string _header;
        std::vector<visus::power_overwhelming::hmc8015_sensor> _hmc8015_sensors;
        std::atomic<bool> _is_collecting;
        std::atomic<bool> _is_running;
        std::mutex _lock;
        std::vector<visus::power_overwhelming::nvml_sensor> _nvml_sensors;
        std::thread _sampler;
        std::ofstream _stream;
        std::vector<visus::power_overwhelming::tinkerforge_sensor> _tinkerforge_sensors;
        std::atomic<std::uint64_t> _unique_identifier;
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
    };

} /* end namespace trrojan */
