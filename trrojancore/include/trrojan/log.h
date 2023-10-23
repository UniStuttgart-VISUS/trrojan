// <copyright file="log.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cstdio>
#include <iostream>
#include <mutex>
#include <sstream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Defines possible log levels which can be filtered.
    /// </summary>
    enum class log_level : std::underlying_type<spdlog::level::level_enum>::type {
        debug = spdlog::level::trace,
        verbose = spdlog::level::debug,
        information = spdlog::level::info,
        warning = spdlog::level::warn,
        error = spdlog::level::err
    };


    /// <summary>
    /// Implements a central logging facility for the TRRojan.
    /// </summary>
    class TRROJANCORE_API log {

    public:

        /// <summary>
        /// Answer the only instance of the <see cref="trrojan::log" />.
        /// </summary>
        /// <param name="file">The path to a log file, which is only honoured in
        /// the very first call to the method. If the parameter is
        /// <c>nullptr</c>, which is the default, the console will be used for
        /// logging.</param>
        /// <returns>An instance of the logger.</returns>
        static inline log& instance(const char *file = nullptr) {
            static log l(file);
            return l;
        }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~log(void);

        /// <summary>
        /// Get the <paramref name="cnt" /> last log entries on UWP.
        /// </summary>
        /// <param name="cnt"></param>
        /// <returns></returns>
        std::vector<std::string> last(const std::size_t cnt) const {
#if defined(TRROJAN_FOR_UWP)
            if (this->_buffer_sink != nullptr) {
                return this->_buffer_sink->last_formatted(cnt);
            }
#endif /* defined(TRROJAN_FOR_UWP) */

            return std::vector<std::string>();
        }

        void write(const log_level level, const char *str);

        template<class... TParams>
        inline void write(const log_level level, const char *fmt,
                TParams&&... params) {
            this->_logger->log(static_cast<spdlog::level::level_enum>(level),
                fmt, std::forward<TParams>(params)...);
        }

        inline void write(const log_level level, const std::exception& ex) {
            this->write(level, "{}", ex.what());
        }

        inline void write(const std::exception& ex) {
            this->write(log_level::error, ex);
        }

        template<class... TParams>
        inline void write_line(const log_level level, const char *fmt,
                TParams&&... params) {
            std::string f(fmt);
            f += "\n";
            // TODO: calls deprecated spdlog function
            // this is only a problem for d3d12, but why?
            this->_logger->log(static_cast<spdlog::level::level_enum>(level),
                f.c_str(), std::forward<TParams>(params)...);
        }

        template<class... TParams>
        inline void write_line(const log_level level, std::string fmt,
                TParams&&... params) {
            fmt += "\n";
            this->_logger->log(static_cast<spdlog::level::level_enum>(level),
                fmt.c_str(), std::forward< TParams>(params)...);
        }

        inline void write_line(const log_level level, std::string fmt) {
            fmt += "\n";
            this->write(level, fmt.c_str());
        }

        inline void write_line(const log_level level, const char* fmt) {
            std::string tmp = fmt + '\n';
            this->write(level, tmp.c_str());
        }

        inline void write_line(const log_level level,
                const std::exception& ex) {
            this->write(level, "{}\n", ex.what());
        }

        inline void write_line(const std::exception& ex) {
            this->write_line(log_level::error, ex);
        }

    private:

        typedef spdlog::sinks::ringbuffer_sink<std::mutex> ring_sink_type;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        log(const char *file);

#if defined(TRROJAN_FOR_UWP)
        std::shared_ptr<ring_sink_type> _buffer_sink;
#endif /* defined(TRROJAN_FOR_UWP) */

        std::shared_ptr<spdlog::logger> _logger;
    };
}
