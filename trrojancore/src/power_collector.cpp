// <copyright file="power_collector.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/power_collector.h"

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
#include <memory>

#include <visus/pwrowg/csv_iomanip.h>
#include <visus/pwrowg/hmc8015_instrument.h>
#include <visus/pwrowg/marker_configuration.h>
#include <visus/pwrowg/rtx_configuration.h>
#include <visus/pwrowg/sensor_array.h>
#include <visus/pwrowg/sensor_filters.h>
#include <visus/pwrowg/thread_local_sink.h>
#include <visus/pwrowg/tinkerforge_configuration.h>
#include <visus/pwrowg/usb_pd_configuration.h>
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */

#include "trrojan/configuration.h"
#include "trrojan/csv_util.h"
#include "trrojan/log.h"
#include "trrojan/power_compatibility_sink.h"


#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
namespace trrojan {
namespace detail {

    typedef visus::pwrowg::thread_local_sink<power_compatibility_sink> pwr_sink;

    /// <summary>
    /// Holds the Power-Overwhelming-related data we want to hide from the
    /// header.
    /// </summary>
    struct power_details final {
        std::vector<visus::pwrowg::hmc8015_instrument> hmc8015;
        visus::pwrowg::marker_controller *markers { };
        visus::pwrowg::tinkerforge_controller *tinkerforge { };
        std::unique_ptr<pwr_sink> sink;
        visus::pwrowg::sensor_array sensors;
    };

    /// <summary>
    /// Starts recording to a new log file on the HMC8015.
    /// </summary>
    void start_hmc8015_sensor(visus::pwrowg::hmc8015_instrument& sensor) {
        // Unfortunately, the HMC8015 only supports 8.3 file names, so we try
        // to build a unique one ...
        std::string file_name;
        {
            std::stringstream file_name_builder;

            auto timestamp = std::chrono::high_resolution_clock::now()
                .time_since_epoch().count();
            timestamp = timestamp & UINT_MAX ^ (timestamp >> 32);
            file_name_builder << std::hex
                << static_cast<std::uint32_t>(timestamp)
                << ".csv";
            file_name = file_name_builder.str();
        }

        sensor.log_file(file_name.c_str(), false, true);

        {
            std::vector<char> actual_name;
            actual_name.resize(sensor.log_file(nullptr, actual_name.size()));
            sensor.log_file(actual_name.data(), actual_name.size());

            log::instance().write_line(log_level::verbose, "HMC8015 is logging "
                "to \"{0}\" (\"{1}\").", file_name, actual_name.data());
        }

        sensor.log(true);
        assert(sensor.is_log());
    }

} /* namespace detail */
} /* namespace trrojan */
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */


/*
 * trrojan::power_collector::get
 */
trrojan::power_collector::pointer trrojan::power_collector::get(
        const configuration& configuration) {
#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
    auto it = configuration.find(power_collector::factor_name);
    if (it != configuration.end()) {
        return it->value().as<power_collector::pointer>();
    }
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */

    return nullptr;
}


/*
 * trrojan::power_collector::delimiter
 */
const char trrojan::power_collector::delimiter = ';';


/*
 * trrojan::power_collector::factor_name
 */
const char *trrojan::power_collector::factor_name = "powerlog";


#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
/*
 * trrojan::power_collector::power_collector
 */
trrojan::power_collector::power_collector(void)
        : _details(std::make_unique<detail::power_details>()) {
    assert(this->_details != nullptr);

    // For backward compatibility, we set up the HMC 8015 separately.
    try {
        this->_details->hmc8015.resize(
            visus::pwrowg::hmc8015_instrument::for_all(nullptr, 0));
        visus::pwrowg::hmc8015_instrument::for_all(
            this->_details->hmc8015.data(),
            this->_details->hmc8015.size());

        for (auto& s : this->_details->hmc8015) {
            s.display("TRRojan. The way you're meant to be trrolled!");
            s.synchronise_clock(true);
            s.log_file("trrojan.csv", true, true);

            // Fix the ranges, because an automatic range switch will ruin the
            // measurements.
            s.voltage_range(visus::pwrowg::hmc8015_instrument_range::explicitly,
                300);
            // If we consume more than 5A 230V, our PSU is probably just before
            // exploding ...
            s.current_range(visus::pwrowg::hmc8015_instrument_range::explicitly,
                5);
            // Tell it to log it until we explicitly stop it.
            s.log_behaviour(std::numeric_limits<float>::lowest(),
                visus::pwrowg::hmc8015_log_mode::unlimited);
        }
    } catch (std::exception& ex) {
        log::instance().write_line(ex);
    }
}


/*
 * trrojan::power_collector::~power_collector
 */
trrojan::power_collector::~power_collector(void) {
    this->stop();
}


/*
 * trrojan::power_collector::enter_scope
 */
std::uint64_t trrojan::power_collector::enter_scope(void) {
    assert(this->_details != nullptr);
    unsigned int retval = 0;
    this->_details->markers->emit(&retval);

    if (this->_details->sink) {
        this->_details->sink->power_uid(retval);
    }

    return retval;
}


/*
 * trrojan::power_collector::leave_scope
 */
void trrojan::power_collector::leave_scope(void) {
    assert(this->_details != nullptr);
    this->_details->markers->emit(0u);
    //if (this->_details->sink) {
    //    this->_details->sink->power_uid(0);
    //}
}


/*
 * trrojan::power_collector::sync_time
 */
void trrojan::power_collector::sync_time(void) {
    assert(this->_details != nullptr);
    if (this->_details->tinkerforge) {
        this->_details->tinkerforge->resync_clock();
    }
}


/*
 * trrojan::power_collector::start
 */
void trrojan::power_collector::start(
        const std::string& file,
        const interval_type sampling_interval) {
    assert(this->_details != nullptr);

    if (this->_details->sensors) {
        throw std::runtime_error("The sampler thread of the power_collector is "
            "already running and cannot be restarted.");
    }

    // Prepare the output sink.
    this->_file = file;
    this->_details->sink.reset(new detail::pwr_sink(1024, this->_file.c_str()));

    // Configure the sensors.
    visus::pwrowg::sensor_array_configuration config;
    config.exclude<visus::pwrowg::rtx_configuration>()
        .exclude<visus::pwrowg::usb_pd_configuration>()
        .sample_every(5)
        .deliver_to(detail::pwr_sink::sample_callback)
        .deliver_context(this->_details->sink.get())
        .configure<visus::pwrowg::tinkerforge_configuration>(
                [](visus::pwrowg::tinkerforge_configuration& c) {
            typedef visus::pwrowg::tinkerforge_sample_averaging avg;
            typedef visus::pwrowg::tinkerforge_conversion_time conv;
            c.averaging(avg::average_of_4);
            c.current_conversion_time(conv::microseconds_588);
            c.voltage_conversion_time(conv::microseconds_588);
        });

    this->_details->sensors = visus::pwrowg::sensor_array::for_matches(
        std::move(config), visus::pwrowg::is_any_of<
        visus::pwrowg::is_power_sensor, visus::pwrowg::is_marker_sensor>);

    this->_details->markers = this->_details->sensors.controller<
        visus::pwrowg::marker_configuration>();
    this->_details->tinkerforge = this->_details->sensors.controller<
        visus::pwrowg::tinkerforge_configuration>();

    log::instance().write_line(log_level::information, "Logging power usage to "
        "\"{0}\" at an {1} ms interval.", this->_file.c_str(),
        sampling_interval.count());
    this->_details->sensors.start();
    assert(this->_details->markers != nullptr);
    this->_details->markers->emit();
}


/*
 * trrojan::power_collector::stop
 */
void trrojan::power_collector::stop(void) {
    assert(this->_details != nullptr);

    // Stop sampling power data.
    if (this->_details->sensors) {
        this->_details->sensors.stop();
    }

    // Dispose the array, which serves as guard whether we are running or not.
    this->_details->sensors = visus::pwrowg::sensor_array();

    // Stop logging on HMC.
    for (auto& s : this->_details->hmc8015) {
        try {
            s.log(false);
            s.display("Do not forget to retrieve your measurements from the "
                "USB port!");
        } catch (std::exception& ex) {
            log::instance().write_line(ex);
        }
    }

    // Finalise the output file.
    this->_details->sink.reset();
}

#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
