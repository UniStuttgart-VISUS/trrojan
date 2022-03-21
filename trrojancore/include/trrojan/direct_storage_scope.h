// <copyright file="direct_storage_scope.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#if defined(TRROJAN_WITH_DSTORAGE)
#include <winsdkver.h>
#include <Windows.h>

#include <atlbase.h>
#include <d3d12.h>
#include <dstorage.h>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// A RAII scope for the DirectStorage factory.
    /// </summary>
    class TRROJANCORE_API direct_storage_scope {

    public:

        /// <summary>
        /// Initialises a new instance by obtaining a new
        /// <see cref="IDStorageFactory" />.
        /// </summary>
        /// <param name=""></param>
        direct_storage_scope(void);

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs">The object to be cloned.</param>
        inline direct_storage_scope(const direct_storage_scope& rhs)
            : _factory(rhs._factory) { }

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs">The object to be moved.</param>
        direct_storage_scope(direct_storage_scope&& rhs) noexcept;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~direct_storage_scope(void) = default;

        /// <summary>
        /// Creates a DirectStorage queue for the factory wrapped by the scope.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="capacity"></param>
        /// <param name="priority"></param>
        /// <param name="type"></param>
        /// <returns></returns>
        ATL::CComPtr<IDStorageQueue> create_storage_queue(
            ID3D12Device *device = nullptr,
            const UINT16 capacity = DSTORAGE_MAX_QUEUE_CAPACITY,
            const DSTORAGE_PRIORITY priority = DSTORAGE_PRIORITY_NORMAL,
            const DSTORAGE_REQUEST_SOURCE_TYPE type = DSTORAGE_REQUEST_SOURCE_FILE);

        /// <summary>
        /// Assignment.
        /// </summary>
        /// <param name="rhs">The object to be copied to this one.</param>
        /// <returns><c>*this</c>.</returns>
        direct_storage_scope& operator =(const direct_storage_scope& rhs);

        /// <summary>
        /// Move assignment.
        /// </summary>
        /// <param name="rhs">The object to be moved to this one.</param>
        /// <returns><c>*this</c>.</returns>
        direct_storage_scope& operator =(direct_storage_scope&& rhs) noexcept;

        /// <summary>
        /// Exposes the factory wrapped in the scope.
        /// </summary>
        /// <returns></returns>
        inline operator IDStorageFactory *(void) {
            return this->_factory.p;
        }

        /// <summary>
        /// Exposes the factory wrapped in the scope.
        /// </summary>
        /// <returns></returns>
        inline IDStorageFactory *operator ->(void) {
            return this->_factory.p;
        }

        /// <summary>
        /// Answer whether the scope is valid.
        /// </summary>
        /// <returns></returns>
        inline operator bool(void) const {
            return (this->_factory != nullptr);
        }

    private:

        ATL::CComPtr<IDStorageFactory> _factory;
    };

}
#endif /* defined(TRROJAN_WITH_DSTORAGE) */
