// <copyright file="log.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/log.h"

#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>


/*
 * trrojan::log::~log
 */
trrojan::log::~log(void) { }


/*
 * trrojan::log::write
 */
void trrojan::log::write(const log_level level, const char *str) {
    this->_logger->log(static_cast<spdlog::level::level_enum>(level), str);
}


/*
 * trrojan::log::log
 */
trrojan::log::log(const char *file) {
    std::vector<spdlog::sink_ptr> sinks;

    if (file != nullptr) {
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file);
        sinks.push_back(sink);
    }

#if defined(TRROJAN_FOR_UWP)
    this->_buffer_sink = std::make_shared<ring_sink_type>(128);
    sinks.push_back(this->_buffer_sink);
#else /* defined(TRROJAN_FOR_UWP) */
    sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
#endif /* defined(TRROJAN_FOR_UWP) */

#if (defined(_WIN32) && (defined(DEBUG) || defined(_DEBUG)))
    sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>());
#endif /* (defined(_WIN32) && (defined(DEBUG) || defined(_DEBUG))) */

    this->_logger = std::make_shared<spdlog::logger>("logger",
        sinks.begin(), sinks.end());

#if (defined(DEBUG) || defined(_DEBUG))
    this->_logger->set_level(spdlog::level::trace);
    spdlog::set_level(spdlog::level::trace);
#endif /* (defined(DEBUG) || defined(_DEBUG)) */
}
