// <copyright file="direct_storage_file.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/direct_storage_file.h"

#if defined(TRROJAN_WITH_DSTORAGE)
#include <cassert>
#include <stdexcept>


/*
 * trrojan::direct_storage_file::direct_storage_file
 */
trrojan::direct_storage_file::direct_storage_file(
        direct_storage_scope& scope, const wchar_t *path) {
    if (!scope) {
        throw std::invalid_argument("A valid direct_storage_scope is required "
            "to create a new direct_storage_file");
    }

    auto hr = scope->OpenFile(path, IID_PPV_ARGS(&this->_file.p));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }
}


/*
 * trrojan::direct_storage_file::get_file_information
 */
BY_HANDLE_FILE_INFORMATION trrojan::direct_storage_file::get_file_information(
        void) const {
    BY_HANDLE_FILE_INFORMATION retval;

    auto hr = this->_file->GetFileInformation(&retval);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::direct_storage_file::queue_request
 */
void trrojan::direct_storage_file::queue_request(IDStorageQueue *queue,
        const std::size_t srcOffset, const std::size_t count,
        ID3D12Resource *dst, const std::size_t dstOffset) {
    assert(this->_file != nullptr);
    assert(queue != nullptr);
    assert(dst != nullptr);

    DSTORAGE_REQUEST request;
    ::ZeroMemory(&request, sizeof(request));
    request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
    request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_BUFFER;

    request.Source.File.Source = this->_file.p;
    request.Source.File.Offset = srcOffset;
    request.Source.File.Size = count;

    request.UncompressedSize = count;

    request.Destination.Buffer.Resource = dst;
    request.Destination.Buffer.Offset = dstOffset;
    request.Destination.Buffer.Size = count;

    queue->EnqueueRequest(&request);
}


#endif /* defined(TRROJAN_WITH_DSTORAGE) */
