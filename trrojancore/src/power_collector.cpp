// <copyright file="power_collector.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
#include "trrojan/power_collector.h"

#include "power_overwhelming/csv_iomanip.h"

#include "trrojan/configuration.h"
#include "trrojan/csv_util.h"
#include "trrojan/log.h"
#include "trrojan/text.h"


/*
 * trrojan::power_collector::delimiter
 */
const char trrojan::power_collector::delimiter = ';';


/*
 * trrojan::power_collector::factor_name
 */
const char *trrojan::power_collector::factor_name = "powerlog";


/*
 * trrojan::power_collector::power_collector
 */
trrojan::power_collector::power_collector(void)
        : _is_collecting(false), _is_running(false) {
    this->setup_adl_sensors();
    this->setup_hmc8015_sensors();
    this->setup_nvml_sensors();
    this->setup_tinkerforge_sensors();
}


/*
 * trrojan::power_collector::~power_collector
 */
trrojan::power_collector::~power_collector(void) {
    this->stop();
}


/*
 * trrojan::power_collector::set_description
 */
void trrojan::power_collector::set_description(std::string&& description) {
    // Start/stop/continue logging based on whether we have a valid description.
    this->_is_collecting.store(!description.empty(),
        std::memory_order::memory_order_release);

    std::lock_guard<decltype(this->_lock)> l(this->_lock);
    // Once the data are locked, flush all data with the old description.
    this->flush_buffer();

    // Store the new description for the next call.
    this->_description = std::move(description);
}


/*
 * trrojan::power_collector::set_description
 */
void trrojan::power_collector::set_description(const configuration& config,
        const std::string& phase) {
    std::stringstream ss;

    for (auto &f : config) {
        print_csv_value(ss, f, true) << delimiter;
    }

    ss << "\"" << phase << "\"";

    this->set_description(ss.str());
}


/*
 * trrojan::power_collector::set_header
 */
void trrojan::power_collector::set_header(const configuration& config,
        const std::string& phase) {
    std::stringstream ss;

    for (auto& f : config) {
        print_csv_header(ss, f, true) << delimiter;
    }

    ss << "\"" << phase << "\"";

    this->_header = ss.str();
}


/*
 * trrojan::power_collector::start
 */
void trrojan::power_collector::start(const std::string& file,
        const interval_type sampling_interval) {
    using namespace visus::power_overwhelming;
    const auto si = std::chrono::duration_cast<std::chrono::microseconds>(
        sampling_interval).count();

    auto expected = false;
    if (!this->_is_running.compare_exchange_strong(expected, true)) {
        throw std::runtime_error("The sampler thread of the power_collector is "
            "already running and cannot be restarted.");
    }

    // Prepare the output file.
    this->_file = file;
    this->_stream = std::ofstream(this->_file, std::ios::trunc);
    if (!this->_stream.is_open()) {
        throw std::invalid_argument("Failed to open output stream.");
    }

    this->_stream << visus::power_overwhelming::setcsvdelimiter(delimiter);
    this->_stream << visus::power_overwhelming::csvquote;


    // Start the ADL sensor, but do not register it in the PO sampler, because
    // we need to sample NVIDIA manually anyway and can share the thread.
    for (auto& s : this->_adl_sensors) {
        s.start(si);
        log::instance().write_line(log_level::information, "Power sensor "
            "\"{0}\" started.", convert_string<char>(s.name()));
    }

    // HMC sensors log to USB and must just be started.
    for (auto& s : this->_hmc8015_sensors) {
        start_hmc8015_sensor(s);
        log::instance().write_line(log_level::information, "Power sensor "
            "\"{0}\" started.", convert_string<char>(s.name()));
    }

    // NVIDIA does not need to be started, because we sample it manually.
    for (auto& s : this->_nvml_sensors) {
        log::instance().write_line(log_level::information, "Power sensor "
            "\"{0}\" started.", convert_string<char>(s.name()));
    }

    // Tinkerforge is the only one that is really asynchronous, so we need to
    // register a callback here.
    for (auto& s : this->_tinkerforge_sensors) {
        s.sample(on_measurement, tinkerforge_sensor_source::power, si, this);
        log::instance().write_line(log_level::information, "Power sensor "
            "\"{0}\" started.", convert_string<char>(s.name()));
    }

    log::instance().write_line(log_level::information, "Logging power usage to "
        "\"{0}\" at an {1} ms interval.", this->_file,
        sampling_interval.count());
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
            s.display("Do not forget to retrieve your measurements from the "
                "USB port!");
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

    // Log all remaining data.
    this->flush_buffer();
    this->_stream.close();
}


/*
 * trrojan::power_collector::on_measurement
 */
void trrojan::power_collector::on_measurement(
        const visus::power_overwhelming::measurement& m,
        void *context) {
    auto that = static_cast<power_collector *>(context);
    if (that->_is_collecting.load(std::memory_order::memory_order_acquire)) {
        std::lock_guard<decltype(that->_lock)> l(that->_lock);
        that->_buffer.push_back(m);
    }
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
 * trrojan::power_collector::flush_buffer
 */
void trrojan::power_collector::flush_buffer(void) {
    // We assume that the caller already holds the lock.
    if ((this->_stream.tellp() == 0) && !this->_buffer.empty()) {
        // If this is the first line, print the CSV header.
        this->_stream << visus::power_overwhelming::csvheader
            << this->_buffer.front() << delimiter
            << this->_header
            << std::endl
            << visus::power_overwhelming::csvdata;
    }

    for (auto& m : this->_buffer) {
        this->_stream
            << m << delimiter
            << this->_description
            << std::endl;
    }

    this->_buffer.clear();
    this->_stream.flush();
}


/*
 * trrojan::power_collector::sample
 */
void trrojan::power_collector::sample(const interval_type sampling_interval) {
    static constexpr auto timestamp_resolution
        = visus::power_overwhelming::timestamp_resolution::milliseconds;
    while (this->_is_running.load()) {
        auto now = std::chrono::high_resolution_clock::now();

        if (this->_is_collecting.load(std::memory_order::memory_order_acquire)) {
            // We sample the sensors only if we have a valid description such
            // that we know the situation for which we sample.
            std::lock_guard<decltype(this->_lock)> l(this->_lock);

             {
                // Sample ADL: The sensor asynchronously provisions the samples in a
                // buffer, but we still need to copy them.
                for (auto &s : this->_adl_sensors) {
                    this->_buffer.push_back(s.sample(timestamp_resolution));
                }

                // Sample NVML: NVIDIA is synchronous, so we need to get the stuff
                // manually.
                for (auto &s : this->_nvml_sensors) {
                    this->_buffer.push_back(s.sample(timestamp_resolution));
                }
            }
        }
        // Note: we must not hold '_lock' while sleeping!

        std::this_thread::sleep_until(now + sampling_interval);
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
    using namespace visus::power_overwhelming;

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
            this->_tinkerforge_sensors.back().configure(
                sample_averaging::average_of_4,
                conversion_time::microseconds_588,
                conversion_time::microseconds_588);
        }

    } catch (std::exception& ex) {
        log::instance().write_line(ex);
    }
}

#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
