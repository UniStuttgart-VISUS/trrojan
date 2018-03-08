/// <copyright file="os_info.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <cstdlib>
#include <utility>

#include "trrojan/sysinfo/export.h"
#include "trrojan/sysinfo/tdr.h"


namespace trrojan {
namespace sysinfo {

    /// <summary>
    /// Provides information about the operating system the software is
    /// running on.
    /// </summary>
    class TRROJANSNFO_API os_info {

    public:

        /// <summary>
        /// Indicates an invalid TDR timeout etc.
        /// </summary>
        static const size_t invalid_tdr_value;

        /// <summary>
        /// Collects information about the current operating system.
        /// </summary>
        static os_info collect(void);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline os_info(void)
            : _name(nullptr),
            _tdr_ddi_delay(os_info::invalid_tdr_value),
            _tdr_debug_mode(sysinfo::tdr_debug_mode::unknown),
            _tdr_delay(os_info::invalid_tdr_value),
            _tdr_level(sysinfo::tdr_level::unknown),
            _tdr_limit_count(os_info::invalid_tdr_value),
            _tdr_limit_time(os_info::invalid_tdr_value),
            _version(nullptr),
            _word_size(0) { }

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        inline os_info(const os_info& rhs) : _name(nullptr), _version(nullptr) {
            *this = rhs;
        }

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        inline os_info(os_info&& rhs) : _name(nullptr), _version(nullptr) {
            *this = std::move(rhs);
        }

        /// <summary>
        /// Finalise the instance.
        /// </summary>
        ~os_info(void);

        /// <summary>
        /// Gets the name of the operating system.
        /// </summary>
        inline const char *const name(void) const {
            return this->_name;
        }

        /// <summary>
        /// Gets the number of seconds that the operating system allows threads
        /// to leave the driver.
        /// </summary>
        inline size_t tdr_ddi_delay(void) const {
            return this->_tdr_ddi_delay;
        }

        /// <summary>
        /// Gets the number of seconds that the operating system allows threads
        /// to leave the driver.
        /// </summary>
        inline sysinfo::tdr_debug_mode tdr_debug_mode(void) const {
            return this->_tdr_debug_mode;
        }

        /// <summary>
        /// Gets the number of seconds that the GPU can delay the preempt
        /// request from the GPU scheduler.
        /// </summary>
        inline size_t tdr_delay(void) const {
            return this->_tdr_delay;
        }

        /// <summary>
        /// Gets the system behaviour when a GPU timeout is detected.
        /// </summary>
        sysinfo::tdr_level tdr_level(void) const {
            return this->_tdr_level;
        }

        /// <summary>
        /// Gets the default number of TDRs that are allowed during the time
        /// specified by the <see cref="tdr_limit_time" /> without crashing
        /// the computer.
        /// </summary>
        size_t tdr_limit_count(void) const {
            return this->_tdr_limit_count;
        }

        /// <summary>
        /// Gets the default time within which a specific number of TDRs
        /// (specified by <see cref="tdr_limit_count" />) are allowed 
        /// without crashing the computer.
        /// </summary>
        size_t tdr_limit_time(void) const {
            return this->_tdr_limit_time;
        }

        /// <summary>
        /// Gets the version of the operating system.
        /// </summary>
        const char *const version(void) const {
            return this->_version;
        }

        /// <summary>
        /// Gets the word size of the operating system.
        /// </summary>
        inline size_t word_size(void) const {
            return this->_word_size;
        }

        os_info& operator =(const os_info& rhs);

        os_info& operator =(os_info&& rhs);

    private:

        char *_name;
        size_t _tdr_ddi_delay;
        sysinfo::tdr_debug_mode _tdr_debug_mode;
        size_t _tdr_delay;
        sysinfo::tdr_level _tdr_level;
        size_t _tdr_limit_count;
        size_t _tdr_limit_time;
        char *_version;
        size_t _word_size;

    };

} /* end namespace trrojan */
} /* end namespace sysinfo */
