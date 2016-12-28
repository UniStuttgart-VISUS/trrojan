/// <copyright file="log.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cstdio>
#include <iostream>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Defines possible log levels which can be filtered.
    /// </summary>
    enum class log_level {
        debug,
        verbose,
        information,
        warning,
        error
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
            ::printf("%s", str);  // TODO: preliminary implementation
        }

        template<class... P>
        inline void write(const log_level level, const char *fmt, P... params) {
            ::printf(fmt, params...);   // TODO: preliminary implementation
        }

        inline void write(const log_level level, const std::exception& ex) {
            this->write(level, "%s", ex.what());
        }

        inline void write(const std::exception& ex) {
            this->write(log_level::error, ex);
        }

        inline void write_line(const log_level level, const char *str) {
            ::printf("%s\n", str);  // TODO: preliminary implementation
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
        inline log(void) { }

    };
}
