/// <copyright file="os_info.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Copyright � 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph M�ller</author>

#pragma once

#include <cinttypes>
#include <utility>

#include "trrojan/sysinfo/export.h"


namespace trrojan {
namespace sysinfo {

    /// <summary>
    /// Provides information about the operating system the software is
    /// running on.
    /// </summary>
    class TRROJANSNFO_API os_info {

    public:

        /// <summary>
        /// Collects information about the current operating system.
        /// </summary>
        static os_info collect(void);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline os_info(void)
            : _name(nullptr), _version(nullptr), _word_size(0) { }

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        inline os_info(const os_info& rhs)
                : _name(nullptr), _version(nullptr), _word_size(0) {
            *this = rhs;
        }

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        inline os_info(os_info&& rhs)
                : _name(nullptr), _version(nullptr), _word_size(0) {
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
        char *_version;
        size_t _word_size;

    };

} /* end namespace trrojan */
} /* end namespace sysinfo */
