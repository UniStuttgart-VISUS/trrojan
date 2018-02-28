/// <copyright file="os_info.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once


namespace trrojan {
namespace sysinfo {

    /// <summary>
    /// Provides information about the operating system the software is
    /// running on.
    /// </summary>
    class os_info {

    public:

        /// <summary>
        /// Answer whether the operating system is 32 bit or 64 bit.
        /// </summary>
        size_t bits(void) const = 0;

        /// <summary>
        /// Gets the name of the operating system.
        /// </summary>
        const char *name(void) const = 0;

        /// <summary>
        /// Gets the version of the operating system.
        /// </summary>
        const char *version(void) const = 0;

    private:

    };

} /* end namespace trrojan */
} /* end namespace sysinfo */
