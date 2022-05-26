/// <copyright file="log.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cstdio>
#include <iostream>
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

#ifdef _UWP
        inline std::string getFullLogString() { return oss.str(); }

        inline std::vector<std::string> getLogStrings(size_t cnt) { return ringbuffer_sink->last_formatted(cnt); }
#endif

    private:

        static std::shared_ptr<spdlog::logger> create_logger(
            const char *file
#ifdef _UWP
            , std::shared_ptr<spdlog::sinks::ostream_sink_st> ostream_sink
            , std::shared_ptr<spdlog::sinks::ringbuffer_sink<std::mutex>> ringbuffer_sink
#endif
        ) {
            if (file != nullptr) {
                return spdlog::basic_logger_mt("file", file);
            } else {
#ifdef _UWP
                std::vector<spdlog::sink_ptr> sinks;
                sinks.push_back(ostream_sink);
                sinks.push_back(ringbuffer_sink);
                return std::make_shared<spdlog::logger>("console", std::begin(sinks), std::end(sinks));
                //return spdlog::create("console", );
                //auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_st>(oss);
                //ostream_logger = std::make_shared<spdlog::logger>("console", ostream_sink);
                //ostream_logger->set_pattern(">%v<");
                //ostream_logger->set_level(spdlog::level::debug);
                //spdlog::set_default_logger(ostream_logger);
#else
                return spdlog::stdout_color_mt("console");
#endif // _UWP
            }
        }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline log(
            const char *file
        ) : 
            oss(),
            ostream_sink(std::make_shared<spdlog::sinks::ostream_sink_st>(oss)),
            ringbuffer_sink(std::make_shared<spdlog::sinks::ringbuffer_sink<std::mutex>>(128)),
            logger(log::create_logger(
                file
#ifdef _UWP
                , ostream_sink
                , ringbuffer_sink
#endif
            ))
        {
            logger->set_level(spdlog::level::trace);
#if (defined(DEBUG) || defined(_DEBUG))
            spdlog::set_level(spdlog::level::trace);
#endif /* (defined(DEBUG) || defined(_DEBUG)) */
        }

#ifdef _UWP
        std::ostringstream oss;
        std::shared_ptr<spdlog::sinks::ostream_sink_st> ostream_sink;
        std::shared_ptr<spdlog::sinks::ringbuffer_sink<std::mutex>> ringbuffer_sink;
#endif

        std::shared_ptr<spdlog::logger> logger;

    };
}
