// <copyright file="cool_down.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

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
                "period was requested. Pausing for {} seconds ...",
                this->config.duration.count());
            std::this_thread::sleep_for(this->config.duration);
            this->last = std::chrono::system_clock::now();
        }
    }
}
