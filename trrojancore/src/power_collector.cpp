// <copyright file="power_collector.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
#include "trrojan/power_collector.h"

#include "trrojan/log.h"
#include "trrojan/text.h"


/*
 * trrojan::power_collector::power_collector
 */
trrojan::power_collector::power_collector(void) : _is_running(false) { }


/*
 * trrojan::power_collector::~power_collector
 */
trrojan::power_collector::~power_collector(void) {
    this->stop();
}


/*
 * trrojan::power_collector::start
 */
void trrojan::power_collector::start(const interval_type sampling_interval) {
    using namespace visus::power_overwhelming;
    const auto si = std::chrono::duration_cast<std::chrono::microseconds>(
        sampling_interval).count();

    auto expected = false;
    if (!this->_is_running.compare_exchange_strong(expected, true)) {
        throw std::runtime_error("The sampler thread of the power_collector is "
            "already running and cannot be restarted.");
    }

    // Start the ADL sensor, but do not register it in the PO sampler, because
    // we need to sample NVIDIA manually anyway and can share the thread.
    for (auto& s : this->_adl_sensors) {
        s.start(si);
    }

    // HMC sensors log to USB and must just be started.
    for (auto& s : this->_hmc8015_sensors) {
        start_hmc8015_sensor(s);
    }

    // NVIDIA does not need to be started, because we sample it manually.

    // Tinkerforge is the only one that is really asynchronous, so we need to
    // register a callback here.
    for (auto& s : this->_tinkerforge_sensors) {
        s.sample(on_measurement, tinkerforge_sensor_source::power, si, this);
    }

    this->_sampler = std::thread(&power_collector::sample, this,
        sampling_interval);
}


/*
 * trrojan::power_collector::stop
 */
void trrojan::power_collector::stop(void) {
    using namespace visus::power_overwhelming;

    // Tell the thread to exit.
    this->_is_running = false;

    // Stop all ADL sensors., because these are collecting asynchronously
    // although we collect the data manually.
    for (auto& s : this->_adl_sensors) {
        try {
            s.stop();
        } catch (std::exception& ex) {
            log::instance().write_line(ex);
        }
    }

    // NVIDIA does not need to be stopped, because it never ran on its own in
    // the first place ...

    // Stop logging on HMC.
    for (auto& s : this->_hmc8015_sensors) {
        try {
            s.log(false);
        } catch (std::exception& ex) {
            log::instance().write_line(ex);
        }
    }

    // Stop Tinkerforge sensors by unregistering the callbacks.
    for (auto &s : this->_tinkerforge_sensors) {
        try {
            s.sample(nullptr, tinkerforge_sensor_source::all);
        } catch (std::exception& ex) {
            log::instance().write_line(ex);
        }
    }

    // Wait for the thread to exit.
    if (this->_sampler.joinable()) {
        this->_sampler.join();
    }
}


/*
 * trrojan::power_collector::on_measurement
 */
void trrojan::power_collector::on_measurement(
        const visus::power_overwhelming::measurement& m,
        void *context) {
    auto that = static_cast<power_collector *>(context);
}


/*
 * trrojan::power_collector::start_hmc8015_sensor
 */
void trrojan::power_collector::start_hmc8015_sensor(
        visus::power_overwhelming::hmc8015_sensor& sensor) {
    std::string file_name = "trrojan";
    file_name += to_string<char>(std::chrono::system_clock::now(), true);
    file_name += ".csv";
    log::instance().write_line(log_level::verbose, "HMC8015 is logging to "
        "\"{0}\".", file_name);

    sensor.log_file(file_name.c_str(), false, true);
    sensor.log(true);
    assert(sensor.is_log());
}


/*
 * trrojan::power_collector::sample
 */
void trrojan::power_collector::sample(const interval_type sampling_interval) {
    static constexpr auto timestamp_resolution
        = visus::power_overwhelming::timestamp_resolution::milliseconds;

    for (auto& s : this->_adl_sensors) {
        s.sample(timestamp_resolution);
    }

    for (auto& s : this->_nvml_sensors) {
        s.sample(timestamp_resolution);
    }
}


/*
 * trrojan::power_collector::setup_adl_sensors
 */
void trrojan::power_collector::setup_adl_sensors(void) {
    using visus::power_overwhelming::adl_sensor;

    try {
        this->_adl_sensors.resize(adl_sensor::for_all(nullptr, 0));
        adl_sensor::for_all(this->_adl_sensors.data(),
            this->_adl_sensors.size());
    } catch (std::exception& ex) {
        log::instance().write_line(ex);
    }
}


/*
 * trrojan::power_collector::setup_hmc8015_sensors
 */
void trrojan::power_collector::setup_hmc8015_sensors(void) {
    using visus::power_overwhelming::hmc8015_sensor;
    using visus::power_overwhelming::instrument_range;
    using visus::power_overwhelming::log_mode;

    try {
        this->_hmc8015_sensors.resize(hmc8015_sensor::for_all(nullptr, 0));
        hmc8015_sensor::for_all(this->_hmc8015_sensors.data(),
            this->_hmc8015_sensors.size());

        for (auto& s : this->_hmc8015_sensors) {
            s.display("They way you're meant to be trrolled!");
            s.synchronise_clock();
            s.log_file("trrojan.csv", true, true);

            // Fix the ranges, because an automatic range switch will ruin the
            // measurements.
            s.voltage_range(instrument_range::explicitly, 300);
            // If we consume more than 5A 230V, our PSU is probably just before
            // exploding ...
            s.current_range(instrument_range::explicitly, 5);

            // Tell it to log it until we explicitly stop it.
            s.log_behaviour(std::numeric_limits<float>::lowest(),
                log_mode::unlimited);
        }

    } catch (std::exception& ex) {
        log::instance().write_line(ex);
    }
}


/*
 * trrojan::power_collector::setup_nvml_sensors
 */
void trrojan::power_collector::setup_nvml_sensors(void) {
    using visus::power_overwhelming::nvml_sensor;

    try {
        this->_nvml_sensors.resize(nvml_sensor::for_all(nullptr, 0));
        nvml_sensor::for_all(this->_nvml_sensors.data(),
            this->_nvml_sensors.size());
    } catch (std::exception& ex) {
        log::instance().write_line(ex);
    }
}


/*
 * trrojan::power_collector::setup_tinkerforge_sensors
 */
void trrojan::power_collector::setup_tinkerforge_sensors(void) {
    using visus::power_overwhelming::tinkerforge_sensor;
    using visus::power_overwhelming::tinkerforge_sensor_definiton;

    try {
        std::vector<tinkerforge_sensor_definiton> descs;
        descs.resize(tinkerforge_sensor::get_definitions(nullptr, 0));
        auto cnt = tinkerforge_sensor::get_definitions(descs.data(),
            descs.size());

        if (cnt < descs.size()) {
            // This is necessary because a bricklet might have been
            // hot-unplugged.
            descs.resize(cnt);
        }

        this->_tinkerforge_sensors.reserve(cnt);
        for (auto& d : descs) {
            this->_tinkerforge_sensors.emplace_back(d);
        }

    } catch (std::exception& ex) {
        log::instance().write_line(ex);
    }
}

#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
