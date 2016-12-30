/// <copyright file="environment.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/stream/environment.h"


/*
 * trrojan::stream::environment::~environment
 */
trrojan::stream::environment::~environment(void) { }


/*
 * trrojan::stream::environment::get_devices
 */
size_t trrojan::stream::environment::get_devices(device_list& dst) {
    return 0;
}


/*
 * trrojan::stream::environment::on_initialise
 */
void trrojan::stream::environment::on_initialise(const trrojan::cmd_line& cmdLine) {
}
