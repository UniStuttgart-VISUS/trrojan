// <copyright file="power_collector.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/power_collector.h"

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
#include <visus/pwrowg/csv_iomanip.h>
#include <visus/pwrowg/hmc8015_instrument.h>
#include <visus/pwrowg/rtx_configuration.h>
#include <visus/pwrowg/sensor_array.h>
#include <visus/pwrowg/sensor_filters.h>
#include <visus/pwrowg/tinkerforge_configuration.h>
#include <visus/pwrowg/usb_pd_configuration.h>
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */

#include "trrojan/configuration.h"
#include "trrojan/csv_util.h"
#include "trrojan/log.h"
#include "trrojan/text.h"


#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
namespace trrojan {
namespace detail {

    /// <summary>
    /// Holds the Power-Overwhelming-related data we want to hide from the header.
    /// </summary>
    struct power_details final {
        std::vector<visus::pwrowg::hmc8015_instrument> hmc8015;
        std::vector<std::string> ids;
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
        : _current_identifier(0),
        _details(std::make_unique<detail::power_details>()),
        _next_identifier(1) {
    assert(this->_details != nullptr);

    // For backward compatibiliy, we set up the HMC 8015 separately.
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
    std::lock_guard<decltype(this->_lock)> l(this->_lock);
    auto retval = this->_current_identifier = this->_next_identifier++;
    return retval;
}


/*
 * trrojan::power_collector::leave_scope
 */
void trrojan::power_collector::leave_scope(void) {
    std::lock_guard<decltype(this->_lock)> l(this->_lock);
    this->_current_identifier = 0;
}


/*
 * trrojan::power_collector::sync_time
 */
void trrojan::power_collector::sync_time(void) {
    //for (auto& s : this->_details->tinkerforge) {
    //    s.resync_internal_clock();
    //}
}


/*
 * trrojan::power_collector::start
 */
void trrojan::power_collector::start(
        const std::string& file,
        const interval_type sampling_interval) {
    assert(this->_details != nullptr);

    if (!this->_details->sensors) {
        throw std::runtime_error("The sampler thread of the power_collector is "
            "already running and cannot be restarted.");
    }

    visus::pwrowg::sensor_array_configuration config;
    config.exclude<visus::pwrowg::rtx_configuration>()
        .exclude<visus::pwrowg::usb_pd_configuration>()
        .sample_every(5)
        .deliver_to([](const visus::pwrowg::sample *samples,
                const std::size_t cnt,
                const visus::pwrowg::sensor_description *descs,
                void *context) {
            auto that = static_cast<power_collector *>(context);
            //if (that->_is_collecting.load(
            //        std::memory_order::memory_order_acquire)) {
            std::lock_guard<decltype(that->_lock)> l(that->_lock);
            for (std::size_t i = 0; i < cnt; ++i) {
                that->_stream
                    << "\"" << that->_details->ids[i] << "\"" << that->delimiter
                    << samples[i].timestamp.value() << that->delimiter
                    << 1 << that->delimiter
                    << that->delimiter
                    << that->delimiter
                    << samples[i].reading.floating_point << that->delimiter
                    << that->_current_identifier
                    << std::endl;
            }
        })
        .deliver_context(this)
        .configure<visus::pwrowg::tinkerforge_configuration>(
                [](visus::pwrowg::tinkerforge_configuration& c) {
            typedef visus::pwrowg::tinkerforge_sample_averaging avg;
            typedef visus::pwrowg::tinkerforge_conversion_time conv;
            c.averaging(avg::average_of_4);
            c.current_conversion_time(conv::microseconds_588);
            c.voltage_conversion_time(conv::microseconds_588);
        });

    this->_details->sensors = visus::pwrowg::sensor_array::for_matches(
        std::move(config), visus::pwrowg::is_power_sensor);

    std::vector<visus::pwrowg::sensor_description> descriptions(
        this->_details->sensors.descriptions(nullptr, 0));
    descriptions.resize(this->_details->sensors.descriptions(
        descriptions.data(), descriptions.size()));
    this->_details->ids.reserve(descriptions.size());
    for (const auto& d : descriptions) {
        this->_details->ids.emplace_back(to_utf8(d.id()));
    }

//"sensor";"timestamp";"valid";"voltage";"current";"power";"power_uid"
//"Tinkerforge/localhost:4223/Ufm";13301178422235;1;-3.40282e+38;-3.40282e+38;inf;1
//"Tinkerforge/localhost:4223/UgC";13301178422235;1;-3.40282e+38;-3.40282e+38;44.698;1
//"Tinkerforge/localhost:4223/UeW";13301178422235;1;-3.40282e+38;-3.40282e+38;inf;1
//"ADL/ASIC/AMD Radeon PRO W6800/0";13301185622222;1;-3.40282e+38;-3.40282e+38;96;1

    // Prepare the output file.
    this->_file = file;
    this->_stream = std::ofstream(this->_file, std::ios::trunc);
    if (!this->_stream.is_open()) {
        throw std::invalid_argument("Failed to open output stream.");
    }

    this->_stream
        << "\"sensor\"" << delimiter
        << "\"timestamp\"" << delimiter
        << "\"valid\"" << delimiter
        << "\"voltage\"" << delimiter
        << "\"current\"" << delimiter
        << "\"power\"" << delimiter
        << "\"power_uid\"" << delimiter
        << std::endl;

    log::instance().write_line(log_level::information, "Logging power usage to "
        "\"{0}\" at an {1} ms interval.", this->_file,
        sampling_interval.count());
    this->_details->sensors.start();
}


/*
 * trrojan::power_collector::stop
 */
void trrojan::power_collector::stop(void) {
    assert(this->_details != nullptr);

    // Stop sampling power data.
    this->_details->sensors.stop();

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
    this->_stream.close();
}

#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
