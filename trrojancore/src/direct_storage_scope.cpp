// <copyright file="direct_storage_scope.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/direct_storage_scope.h"

#if defined(TRROJAN_WITH_DSTORAGE)
#include <stdexcept>


/*
 * trrojan::direct_storage_scope::direct_storage_scope
 */
trrojan::direct_storage_scope::direct_storage_scope(void) {
    auto hr = ::DStorageGetFactory(IID_PPV_ARGS(&this->_factory.p));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }
}


/*
 * trrojan::direct_storage_scope::direct_storage_scope
 */
trrojan::direct_storage_scope::direct_storage_scope(
        direct_storage_scope&& rhs) noexcept
        : _factory(rhs._factory) {
    rhs._factory.Release();
}


/*
 * trrojan::direct_storage_scope::create_storage_queue
 */
ATL::CComPtr<IDStorageQueue> trrojan::direct_storage_scope::create_storage_queue(
        ID3D12Device *device,
        const UINT16 capacity,
        const DSTORAGE_PRIORITY priority,
        const DSTORAGE_REQUEST_SOURCE_TYPE type) {
    if (this->_factory == nullptr) {
        throw std::logic_error("A storage queue cannot be created unless the "
            "scope is valid.");
    }

    DSTORAGE_QUEUE_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.Capacity = capacity;
    desc.Priority = priority;
    desc.SourceType = type;
    desc.Device = device;

    ATL::CComPtr<IDStorageQueue> retval;
    auto hr = this->_factory->CreateQueue(&desc, IID_PPV_ARGS(&retval.p));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::direct_storage_scope::operator =
 */
trrojan::direct_storage_scope& trrojan::direct_storage_scope::operator =(
        const direct_storage_scope& rhs) {
    if (this != std::addressof(rhs)) {
        this->_factory = rhs._factory;
    }

    return *this;
}


/*
 * trrojan::direct_storage_scope::operator =
 */
trrojan::direct_storage_scope& trrojan::direct_storage_scope::operator =(
        direct_storage_scope&& rhs) noexcept {
    if (this != std::addressof(rhs)) {
        this->_factory = rhs._factory;
        rhs._factory.Release();
    }

    return *this;
}
#endif /* defined(TRROJAN_WITH_DSTORAGE) */
