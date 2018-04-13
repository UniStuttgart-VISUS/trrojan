/// <copyright file="os_info_impl.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
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
