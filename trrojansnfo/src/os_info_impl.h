/// <copyright file="os_info_impl.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include <cinttypes>
#include <string>

#include "trrojan/sysinfo/tdr.h"


namespace trrojan {
namespace sysinfo {
namespace detail {

    /// <summary>
    /// Opaque implementation of <see cref="os_info" />.
    /// </summary>
    struct os_info_impl {

        os_info_impl(void);

        void update(void);

        std::string name;
        size_t tdr_ddi_delay;
        sysinfo::tdr_debug_mode tdr_debug_mode;
        size_t tdr_delay;
        sysinfo::tdr_level tdr_level;
        size_t tdr_limit_count;
        size_t tdr_limit_time;
        std::string version;
        size_t word_size;
    };

} /* end namespace detail */
} /* end namespace trrojan */
} /* end namespace sysinfo */
