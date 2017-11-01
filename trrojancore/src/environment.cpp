/// <copyright file="environment.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/environment.h"

#include <sstream>
#include <stdexcept>

#include "trrojan/configuration.h"


/*
 * trrojan::environment_base::~environment_base
 */
trrojan::environment_base::~environment_base(void) { }


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
void trrojan::environment_base::translate_device(configuration& config,
        const std::string& name, const std::string& device) {
    auto fn = config.find(name);
    auto fd = config.find(device);

    if ((fn != config.end()) && (fd == config.end())) {
        std::vector<trrojan::device> devices;
        this->get_devices(devices);

        auto dn = fn->value().as<std::string>();
        auto d = std::find_if(devices.begin(), devices.end(),
            [&dn](const trrojan::device& d) { return d->name() == dn; });
        if (d != devices.end()) {
            config.add(device, *d);

        } else {
            std::stringstream msg;
            msg << "No device named \"" << dn << "\" is available."
                << std::ends;
            throw std::invalid_argument(msg.str());
        }
    }
}
