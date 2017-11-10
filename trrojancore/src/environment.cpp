/// <copyright file="environment.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/environment.h"

#include <sstream>
#include <stdexcept>

#include "trrojan/configuration.h"
#include "trrojan/factor.h"
#include "trrojan/log.h"
#include "trrojan/text.h"


/*
 * trrojan::environment_base::factor_name
 */
const char *trrojan::environment_base::factor_name = "environment";


/*
 * trrojan::environment_base::none_name
 */
const char *trrojan::environment_base::none_name = "none";


/*
 * trrojan::environment_base::~environment_base
 */
trrojan::environment_base::~environment_base(void) { }


/*
 * trrojan::environment_base::get_device
 */
trrojan::device trrojan::environment_base::get_device(const std::string& name) {
    device_list devices;
    this->get_devices(devices);

    for (auto d : devices) {
        if ((d != nullptr) && (d->name() == name)) {
            return d;
        }
    }

    return nullptr;
}


/*
 * trrojan::environment_base::on_activate
 */
void trrojan::environment_base::on_activate(void) { }


/*
 * trrojan::environment_base::on_deactivate
 */
void trrojan::environment_base::on_deactivate(void)  noexcept { }


/*
 * trrojan::environment_base::on_finalise
 */
void trrojan::environment_base::on_finalise(void)  noexcept { }


/*
 * trrojan::environment_base::on_initialise
 */
void trrojan::environment_base::on_initialise(const cmd_line& cmdLine) { }


/*
 * trrojan::environment_base::translate_device
 */
bool trrojan::environment_base::translate_device(configuration& config,
        const std::string& name) {
    device dev = nullptr;
    auto factor = config.find(name);

    if (factor != config.end()) {
        auto& f = factor->value();

        switch (f.type()) {
            case variant_type::string:
                dev = this->get_device(f.as<std::string>());
                break;

            case variant_type::wstring:
                dev = this->get_device(to_utf8(f.as<std::wstring>()));
                break;

            default:
                dev = nullptr;
                break;
        }

        if (dev != nullptr) {
            config.replace(name, variant(std::move(dev)));
            return true;

        } else {
            log::instance().write_line(log_level::warning, "No device name to "
                "be translated into a device object was found in the given "
                "configuration (as factor \"%s\". Note that this might be "
                "perfectly OK if no device restriction was specified or if the "
                "device has already been applied to the configuration.",
                name.c_str());
            return false;
        }
    }
}
