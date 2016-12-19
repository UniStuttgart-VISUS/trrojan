/// <copyright file="environment.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/environment.h"


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
void trrojan::environment_base::on_deactivate(void)  throw() { }


/*
 * trrojan::environment_base::on_finalise
 */
void trrojan::environment_base::on_finalise(void)  throw() { }


/*
 * trrojan::environment_base::on_initialise
 */
void trrojan::environment_base::on_initialise(
    const std::vector<std::string>& cmdLine) { }
