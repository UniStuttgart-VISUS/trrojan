/// <copyright file="executive.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/executive.h"


/*
 * trrojan::executive::~executive
 */
trrojan::executive::~executive(void) {
    if (this->cur_environment != nullptr) {
        this->cur_environment->on_deactivate();
    }

    for (auto e : this->environments) {
        e.second->on_finalise();
    }
}


/*
 * trrojan::executive::enable_environment
 */
void trrojan::executive::enable_environment(const std::string& name) {
    if (this->cur_environment != nullptr) {
        this->cur_environment->on_deactivate();
    }
    this->cur_environment = this->environments[name];
    this->cur_environment->on_activate();
}
