// <copyright file="log.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/log.h"


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
    if (file != nullptr) {
        this->_logger = spdlog::basic_logger_mt("file", file);

    } else {
#if defined(TRROJAN_FOR_UWP)
        this->_buffer_sink = std::make_shared<ring_sink_type>(128);
        this->_logger = std::make_shared<spdlog::logger>("console",
            this->_buffer_sink);
#else /* defined(TRROJAN_FOR_UWP) */
        this->_logger = spdlog::stdout_color_mt("console");
#endif /* defined(TRROJAN_FOR_UWP) */
    }

#if (defined(DEBUG) || defined(_DEBUG))
    this->_logger->set_level(spdlog::level::trace);
    spdlog::set_level(spdlog::level::trace);
#endif /* (defined(DEBUG) || defined(_DEBUG)) */
}
