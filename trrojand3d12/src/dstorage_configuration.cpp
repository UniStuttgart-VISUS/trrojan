// <copyright file="dstorage_configuration.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


#include "trrojan/d3d12/dstorage_configuration.h"

#include "trrojan/d3d12/utilities.h"


#define _DSTOR_DEFINE_FACTOR(f)                                                \
const char *trrojan::d3d12::dstorage_configuration::factor_##f = #f

_DSTOR_DEFINE_FACTOR(queue_depth);
_DSTOR_DEFINE_FACTOR(queue_priority);
_DSTOR_DEFINE_FACTOR(staging_directory);

#undef _DSTOR_DEFINE_FACTOR


/*
 * trrojan::d3d12::dstorage_configuration::add_defaults
 */
void trrojan::d3d12::dstorage_configuration::add_defaults(
        configuration_set& configs) {
#if defined(TRROJAN_WITH_DSTORAGE)
    configs.add_factor(factor::from_manifestations(factor_queue_depth,
        DSTORAGE_MAX_QUEUE_CAPACITY));
    configs.add_factor(factor::from_manifestations(factor_queue_priority,
        static_cast<std::uint32_t>(DSTORAGE_PRIORITY_NORMAL)));
#else /* defined(TRROJAN_WITH_DSTORAGE) */
    configs.add_factor(factor::from_manifestations(factor_queue_depth,
        static_cast<std::uint8_t>(0)));
    configs.add_factor(factor::from_manifestations(factor_queue_priority, 0u));
#endif /* defined(TRROJAN_WITH_DSTORAGE) */

    configs.add_factor(factor::from_manifestations(factor_staging_directory,
        get_temp_folder()));
}


#if defined(TRROJAN_WITH_DSTORAGE)
#define _DSTOR_INIT_FACTOR(f) _##f(config.get<decltype(_##f)>(factor_##f))

/*
 * trrojan::d3d12::dstorage_configuration::dstorage_configuration
 */
trrojan::d3d12::dstorage_configuration::dstorage_configuration(
        const configuration& config)
    : _DSTOR_INIT_FACTOR(queue_depth),
        _DSTOR_INIT_FACTOR(queue_priority),
        _DSTOR_INIT_FACTOR(staging_directory) {
    if (this->_queue_depth < DSTORAGE_MIN_QUEUE_CAPACITY) {
        this->_queue_depth = DSTORAGE_MIN_QUEUE_CAPACITY;
    } else if (this->_queue_depth >= DSTORAGE_MAX_QUEUE_CAPACITY) {
        this->_queue_depth = DSTORAGE_MAX_QUEUE_CAPACITY;
    }
}

#undef _DSTOR_INIT_FACTOR

#endif /* defined(TRROJAN_WITH_DSTORAGE) */
