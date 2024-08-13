// <copyright file="dstorage_configuration.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/dstorage_configuration.h"


#include "trrojan/com_error_category.h"
#include "trrojan/contains.h"
#include "trrojan/io.h"


#define _DSTOR_DEFINE_FACTOR(f)                                                \
const char *trrojan::d3d12::dstorage_configuration::factor_##f = #f

_DSTOR_DEFINE_FACTOR(queue_depth);
_DSTOR_DEFINE_FACTOR(queue_priority);
_DSTOR_DEFINE_FACTOR(staging_buffer_size);
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

    configs.add_factor(factor::from_manifestations(factor_staging_buffer_size,
        static_cast<std::uint32_t>(32 * 1204 * 1024)));
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
        _DSTOR_INIT_FACTOR(staging_buffer_size),
        _DSTOR_INIT_FACTOR(staging_directory) {
    if (this->_queue_depth < DSTORAGE_MIN_QUEUE_CAPACITY) {
        this->_queue_depth = DSTORAGE_MIN_QUEUE_CAPACITY;
    } else if (this->_queue_depth >= DSTORAGE_MAX_QUEUE_CAPACITY) {
        this->_queue_depth = DSTORAGE_MAX_QUEUE_CAPACITY;
    }
}


/*
 * trrojan::d3d12::dstorage_configuration::apply
 */
void trrojan::d3d12::dstorage_configuration::apply(
        winrt::com_ptr<IDStorageFactory> factory,
        const std::vector<std::string>& changed) const {
    if (factory == nullptr) {
        throw ATL::CAtlException(E_POINTER);
    }

    // Set the size of the staging buffer.
    if (changed.empty() || contains(changed, factor_staging_buffer_size)) {
        auto hr = factory->SetStagingBufferSize(this->_staging_buffer_size);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }
}


/*
 * trrojan::d3d12::dstorage_configuration::create_factory
 */
winrt::com_ptr<IDStorageFactory>
trrojan::d3d12::dstorage_configuration::create_factory(void) {
    winrt::com_ptr<IDStorageFactory> retval;

    {
        auto hr = ::DStorageGetFactory(IID_PPV_ARGS(retval.put()));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    this->apply(retval, { });

    return retval;
}

#undef _DSTOR_INIT_FACTOR

#endif /* defined(TRROJAN_WITH_DSTORAGE) */
