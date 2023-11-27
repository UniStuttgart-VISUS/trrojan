// <copyright file="dstorage_configuration.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <string>
#include <type_traits>
#include <vector>

#if defined(TRROJAN_WITH_DSTORAGE)
#include <dstorage.h>
#endif /* defined(TRROJAN_WITH_DSTORAGE) */

#include <winrt/base.h>

#include "trrojan/configuration_set.h"

#include "trrojan/d3d12/export.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Manages factors that are shared by DirectStorage implementations.
    /// </summary>
    class TRROJAND3D12_API dstorage_configuration final {

    public:

        /// <summary>
        /// Adds the defaults for the factors controlled by this class to the
        /// given configuration set.
        /// </summary>
        /// <param name="configs"></param>
        static void add_defaults(configuration_set& configs);

        /// <summary>
        /// Specifies the factor that determines the capacity of the
        /// DirectStorage queue.
        /// </summary>
        static const char *factor_queue_depth;

        /// <summary>
        /// Specifies the priority of the queue used to stream the data.
        /// </summary>
        /// <remarks>
        /// This is a numeric factor that receives the values of the
        /// <see cref="DSTORAGE_PRIORITY" /> enumeration. If not specified,
        /// the default value is <see cref="DSTORAGE_PRIORITY_NORMAL" />.
        /// </remarks>
        static const char *factor_queue_priority;

        /// <summary>
        /// Specifies the size of the staging buffer in bytes.
        /// </summary>
        static const char *factor_staging_buffer_size;

        /// <summary>
        /// Determines the directory where the input data are staged.
        /// </summary>
        /// <remarks>
        /// The benchmark will copy the raw data to this directory before
        /// streaming from there, which allows us to test different kinds of
        /// disks.
        /// </remarks>
        static const char *factor_staging_directory;

#if defined(TRROJAN_WITH_DSTORAGE)
        dstorage_configuration(const configuration& config);

        inline void apply(DSTORAGE_QUEUE_DESC& desc) const noexcept {
            desc.Capacity = this->queue_depth();
            desc.Priority = this->queue_priority();
        }

        void apply(winrt::com_ptr<IDStorageFactory> factory,
            const std::vector<std::string>& changed) const;

        /// <summary>
        /// Creates a DirectStorage factory and applies all configuration that
        /// is applicable to the factory itself.
        /// </summary>
        /// <returns></returns>
        /// <exception cref="ATL::CAtlException"></exception>
        winrt::com_ptr<IDStorageFactory> create_factory(void);

        inline std::uint16_t queue_depth(void) const noexcept {
            return this->_queue_depth;
        }

        inline DSTORAGE_PRIORITY queue_priority(void) const noexcept {
            return static_cast<DSTORAGE_PRIORITY>(this->_queue_priority);
        }

        inline const std::string& staging_directory(void) const noexcept {
            return this->_staging_directory;
        }

    private:

        std::uint16_t _queue_depth;
        std::underlying_type<DSTORAGE_PRIORITY>::type _queue_priority;
        std::uint32_t _staging_buffer_size;
        std::string _staging_directory;

#else /* defined(TRROJAN_WITH_DSTORAGE) */
        dstorage_configuration(void) = delete;
#endif /* defined(TRROJAN_WITH_DSTORAGE) */
    };

} /* end namespace d3d12 */
} /* end namespace trrojan */
