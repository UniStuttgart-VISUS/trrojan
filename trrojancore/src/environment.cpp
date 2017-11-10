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
