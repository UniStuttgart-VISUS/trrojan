/// <copyright file="cool_down.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/cool_down.h"

#include <thread>

#include "trrojan/log.h"


/*
 * trrojan::cool_down_evaluator::check
 */
void trrojan::cool_down_evaluator::check(void) {
    if (this->config.enabled()) {
        auto dt = std::chrono::system_clock::now() - last;
        if (dt > this->config.frequency) {
            log::instance().write_line(log_level::information, "A cool-down "
                "period was requested. Pausing for %u seconds ...",
                this->config.duration.count());
            std::this_thread::sleep_for(this->config.duration);
            this->last = std::chrono::system_clock::now();
        }
    }
}
