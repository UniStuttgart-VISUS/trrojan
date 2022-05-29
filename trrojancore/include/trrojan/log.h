/// <copyright file="log.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cstdio>
#include <iostream>

#include <spdlog/spdlog.h>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Defines possible log levels which can be filtered.
    /// </summary>
    enum class log_level : std::underlying_type<
            spdlog::level::level_enum>::type {
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

        inline void write(const log_level level, const char *str) {
            this->logger->log(static_cast<spdlog::level::level_enum>(level),
                str);
        }

        template<class... P>
        inline void write(const log_level level, const char *fmt,
                P... params) {
            this->logger->log(static_cast<spdlog::level::level_enum>(level),
                fmt, params...);
        }

        inline void write(const log_level level, const std::exception& ex) {
            this->write(level, "{}", ex.what());
        }

        inline void write(const std::exception& ex) {
            this->write(log_level::error, ex);
        }

        template<class... P>
        inline void write_line(const log_level level, const char *fmt,
                P... params) {
            std::string f(fmt);
            f += "\n";
            this->logger->log(static_cast<spdlog::level::level_enum>(level),
                f.c_str(), params...);
        }

        template<class... P>
        inline void write_line(const log_level level, std::string fmt,
                P... params) {
            fmt += "\n";
            this->logger->log(static_cast<spdlog::level::level_enum>(level),
                fmt.c_str(), params...);
        }

        inline void write_line(const log_level level,
                const std::exception& ex) {
            this->write(level, "{}\n", ex.what());
        }

        inline void write_line(const std::exception& ex) {
            this->write_line(log_level::error, ex);
        }

    private:

        static std::shared_ptr<spdlog::logger> create_logger(const char *file) {
            if (file != nullptr) {
                return spdlog::basic_logger_mt("file", file);
            } else {
                return spdlog::stdout_color_mt("console");
            }
        }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline log(const char *file) : logger(log::create_logger(file)) {
#if (defined(DEBUG) || defined(_DEBUG))
            spdlog::set_level(spdlog::level::trace);
#endif /* (defined(DEBUG) || defined(_DEBUG)) */
        }

        std::shared_ptr<spdlog::logger> logger;

    };
}
