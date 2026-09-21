// <copyright file="power_collector.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/power_collector.h"

#include <memory>

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
#include <visus/pwrowg/convert_string.h>
#include <visus/pwrowg/csv_iomanip.h>
#include <visus/pwrowg/dump_sensors.h>
#include <visus/pwrowg/hdf5_sink.h>
#include <visus/pwrowg/hmc8015_instrument.h>
#include <visus/pwrowg/marker_configuration.h>
#include <visus/pwrowg/msr_configuration.h>
#include <visus/pwrowg/parquet_sink.h>
#include <visus/pwrowg/pwog_sink.h>
#include <visus/pwrowg/rtx_configuration.h>
#include <visus/pwrowg/rtx_sensor_trigger.h>
#include <visus/pwrowg/sensor_array.h>
#include <visus/pwrowg/sensor_filters.h>
#include <visus/pwrowg/thread_local_sink.h>
#include <visus/pwrowg/tinkerforge_configuration.h>
#include <visus/pwrowg/usb_pd_configuration.h>
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */

#include "trrojan/configuration.h"
#include "trrojan/csv_util.h"
#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/power_compatibility_sink.h"
#include "trrojan/system_factors.h"


#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
namespace trrojan {
namespace detail {

#if defined(USE_HDF5_SINK)
    typedef visus::pwrowg::thread_local_sink<visus::pwrowg::hdf5_sink> pwr_sink;
#elif defined(USE_PWOG_SINK)
    typedef visus::pwrowg::thread_local_sink<visus::pwrowg::pwog_sink>
        pwr_sink;
#elif defined(USE_PARQUET_SINK)
    typedef visus::pwrowg::thread_local_sink<visus::pwrowg::parquet_sink>
        pwr_sink;
#else /* (defined(USE_PARQUET_SINK) */
    typedef visus::pwrowg::thread_local_sink<power_compatibility_sink> pwr_sink;
#endif /* (defined(USE_PARQUET_SINK) */

    /// <summary>
    /// Holds the Power-Overwhelming-related data we want to hide from the
    /// header.
    /// </summary>
    struct power_details final {
        std::vector<visus::pwrowg::hmc8015_instrument> hmc8015;
        visus::pwrowg::marker_controller *markers { };
        visus::pwrowg::rtx_sensor_trigger rtx_trigger;
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
        log::instance().write(log_level::warning, ex);
    }
}


/*
 * trrojan::power_collector::~power_collector
 */
trrojan::power_collector::~power_collector(void) {
    this->stop();
}


/*
 * trrojan::power_collector::acquire_rtx
 */
bool trrojan::power_collector::acquire_rtx(
        const std::function<void(void)>& acquired,
        const std::function<void(bool)>& done) {
    if ((this->_details == nullptr) || !this->_details->rtx_trigger) {
        return false;
    }

    return this->_details->rtx_trigger.acquire(
        [acquired](void) { acquired(); },
        [done](void) { done(true); },
        [done](const std::exception_ptr) { done(false); return true; });
}


/*
 * trrojan::power_collector::configure_rtx
 */
void trrojan::power_collector::configure_rtx(const std::string& file) {
    this->_rtx_config = file;
}


/*
 * trrojan::power_collector::enter_scope
 */
std::uint64_t trrojan::power_collector::enter_scope(void) {
    assert(this->_details != nullptr);
    unsigned int retval = 0;
    this->_details->markers->emit(&retval);

#if (!defined(USE_HDF5_SINK) && !defined(USE_PARQUET_SINK) && !defined(USE_PWOG_SINK))
    if (this->_details->sink) {
        this->_details->sink->power_uid(retval);
    }
#endif /* (!defined(USE_HDF5_SINK) && !defined(USE_PARQUET_SINK) && !defined(USE_PWOG_SINK)) */
    return retval;
}


/*
 * trrojan::power_collector::leave_scope
 */
void trrojan::power_collector::leave_scope(void) {
    assert(this->_details != nullptr);
    this->_details->markers->emit(0u);
#if !defined(USE_PARQUET_SINK)
    this->_details->sink->flush();
#endif /* !defined(USE_PARQUET_SINK) */
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
        const interval_type sampling_interval,
        const cmd_line& cmd_line) {
    using namespace visus::pwrowg;
    assert(this->_details != nullptr);

    if (this->_details->sensors) {
        throw std::runtime_error("The sampler thread of the power_collector is "
            "already running and cannot be restarted.");
    }

    // Parse the optional settings from the command line.
    {
        auto it = trrojan::find_argument("--rtx-configuration",
            cmd_line.begin(), cmd_line.end());
        if (it != cmd_line.end()) {
            this->configure_rtx(*it);
        }
    }

    std::size_t batch_size = 1024;
    {
        auto it = trrojan::find_argument("--power-batch",
            cmd_line.begin(), cmd_line.end());
        if (it != cmd_line.end()) {
            batch_size = trrojan::parse<std::size_t>(it->c_str());
        }
    }

    const auto record_voltage = trrojan::contains_switch(
        "--record-voltage", cmd_line.begin(), cmd_line.end());
    const auto record_current = trrojan::contains_switch(
        "--record-current", cmd_line.begin(), cmd_line.end());


    // Prepare the output sink.
    this->_file = file;
#if defined(USE_HDF5_SINK)
    {
        hdf5_configuration c(this->_file.c_str(), true);
        c.chunk_size(batch_size);
        c.raw(false);

        this->_details->sink.reset(new detail::pwr_sink(batch_size, false,
            this->_file.c_str()));
    }
#elif defined(USE_PWOG_SINK)
    {
        auto file = visus::pwrowg::pwog_file::create(this->_file.c_str(), true);

        try {
            auto it = trrojan::find_argument("--trroll", cmd_line.begin(),
                cmd_line.end());
            file << visus::pwrowg::make_pwog_meta_data("TRRollFile", *it);

            if (it != cmd_line.end()) {
                const auto t = read_text_file(*it);
                file << visus::pwrowg::make_pwog_meta_data("TRRollScript", t);
            }
        } catch (std::exception& ex) {
            log::instance().write_line(log_level::warning, ex);
        }

        try {
            {
                std::stringstream ss;
                ss << system_factors::instance().bios();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorBios", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().computer_name();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorComputerName", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().cpu();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorCPU", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().debug_build();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorDebugBuild", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().installed_memory();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorInstalledMemory", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().logical_cores();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorLogicalCores", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().mainboard();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorMainboard", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().os();
                ss << " ";
                ss << system_factors::instance().os_version();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorOperatingSystem", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().ram();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorRAM", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().system_desc();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorSystemDescription", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().tdr_delay();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorTDRDelay", ss.str());
            }
            {
                std::stringstream ss;
                ss << system_factors::instance().tdr_level();
                file << visus::pwrowg::make_pwog_meta_data(
                    "SystemFactorTDRLevel", ss.str());
            }
        } catch (std::exception& ex) {
            log::instance().write_line(log_level::warning, ex);
        }

        this->_details->sink.reset(new detail::pwr_sink(batch_size, false,
            std::move(file)));
    }
#elif defined(USE_PARQUET_SINK)
    {
        parquet_configuration c(this->_file.c_str());
        c.identity(parquet_identity_column::label);
        c.raw(false);
        this->_details->sink.reset(new detail::pwr_sink(batch_size, c));
    }
#else /* defined(USE_PARQUET_SINK) */
    this->_details->sink.reset(new detail::pwr_sink(batch_size, false,
        this->_file.c_str()));
#endif /* defined(USE_PARQUET_SINK) */

    // Configure the sensors.
    sensor_array_configuration config;
    config.exclude<usb_pd_configuration>()
        .sample_every(sampling_interval)
        .deliver_to(detail::pwr_sink::sample_callback)
        .deliver_context(this->_details->sink.get())
        .configure<tinkerforge_configuration>(
                [](tinkerforge_configuration& c) {
            typedef visus::pwrowg::tinkerforge_sample_averaging avg;
            typedef visus::pwrowg::tinkerforge_conversion_time conv;
            c.averaging(avg::average_of_4);
            c.current_conversion_time(conv::milliseconds_2_116);
            c.voltage_conversion_time(conv::milliseconds_2_116);
        })
        .configure<msr_configuration>([](msr_configuration& c) {
            c.first_core(true);
        });

    // Enable the oscilloscope if a configuration was provided.
    if (!this->_rtx_config.empty()) {
        config.configure<rtx_configuration>(
            [this](rtx_configuration& c) {
                c = rtx_configuration::load(this->_rtx_config.c_str());
                this->_details->rtx_trigger = c.trigger();
            });
    } else {
        config.exclude<rtx_configuration>();
    }

    if (record_voltage && record_current) {
        this->_details->sensors = visus::pwrowg::sensor_array::for_matches(
            std::move(config), visus::pwrowg::is_any_of<
                visus::pwrowg::is_power_sensor,
                visus::pwrowg::is_marker_sensor,
                visus::pwrowg::is_voltage_sensor,
                visus::pwrowg::is_current_sensor>);
    } else if (record_voltage) {
        this->_details->sensors = visus::pwrowg::sensor_array::for_matches(
            std::move(config), visus::pwrowg::is_any_of<
                visus::pwrowg::is_power_sensor,
                visus::pwrowg::is_marker_sensor,
                visus::pwrowg::is_voltage_sensor>);
    } else if (record_current) {
        this->_details->sensors = visus::pwrowg::sensor_array::for_matches(
            std::move(config), visus::pwrowg::is_any_of<
                visus::pwrowg::is_power_sensor,
                visus::pwrowg::is_marker_sensor,
                visus::pwrowg::is_current_sensor>);
    } else {
        this->_details->sensors = visus::pwrowg::sensor_array::for_matches(
            std::move(config), visus::pwrowg::is_any_of<
                visus::pwrowg::is_power_sensor,
                visus::pwrowg::is_marker_sensor>);
    }

    {
        auto it = trrojan::find_argument("--dump-power-sensors",
            cmd_line.begin(), cmd_line.end());
        if (it != cmd_line.end()) {
            log::instance().write_line(log_level::verbose, "Logging power "
                "sensors  to \"{0}\".", it->c_str());
            dump_sensors(this->_details->sensors, *it);
        }
    }

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
    log::instance().write_line(log_level::information, "Stopping power data "
        "collection.");

    // Stop sampling power data.
    if (this->_details->sensors) {
        this->_details->sensors.stop();
    }

    // Finalise the output file.
    this->_details->sink.reset();

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
}

#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
