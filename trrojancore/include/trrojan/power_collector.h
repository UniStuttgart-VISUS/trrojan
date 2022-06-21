// <copyright file="power_collector.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "trrojan/export.h"

#include "power_overwhelming/collector.h"


namespace trrojan {

    /// <summary>
    /// A utility class for sampling power sensors.
    /// </summary>
    class TRROJANCORE_API power_collector final {

    public:

        typedef std::chrono::milliseconds interval_type;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        power_collector(void);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~power_collector(void);

        /// <summary>
        /// Start all sensors with the specified sampling interval.
        /// </summary>
        void start(const interval_type sampling_interval);

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

        void sample(const interval_type sampling_interval);

        void setup_adl_sensors(void);

        void setup_hmc8015_sensors(void);

        void setup_nvml_sensors(void);

        void setup_tinkerforge_sensors(void);

        std::vector<visus::power_overwhelming::adl_sensor> _adl_sensors;
        std::vector<visus::power_overwhelming::hmc8015_sensor> _hmc8015_sensors;
        std::atomic<bool> _is_running;
        std::vector<visus::power_overwhelming::nvml_sensor> _nvml_sensors;
        std::thread _sampler;
        std::vector<visus::power_overwhelming::tinkerforge_sensor> _tinkerforge_sensors;

    };

} /* end namespace trrojan */

#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
