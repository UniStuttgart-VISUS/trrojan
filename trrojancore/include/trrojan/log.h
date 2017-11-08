/// <copyright file="log.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
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
        static inline log& instance(void) {
            static log l;
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
            this->write(level, "%s", ex.what());
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
            this->write(level, "%s\n", ex.what());
        }

        inline void write_line(const std::exception& ex) {
            this->write_line(log_level::error, ex);
        }

    private:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline log(void) : logger(spdlog::stdout_color_mt("console")) {
#if (defined(DEBUG) || defined(_DEBUG))
            spdlog::set_level(spdlog::level::trace);
#endif /* (defined(DEBUG) || defined(_DEBUG)) */
        }

        std::shared_ptr<spdlog::logger> logger;

    };
}
