// <copyright file="direct_storage_file.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#if defined(TRROJAN_WITH_DSTORAGE)
#include "trrojan/direct_storage_scope.h"


namespace trrojan {

    /// <summary>
    /// A RAII container for a DirectStorage file.
    /// </summary>
    class TRROJANCORE_API direct_storage_file {

    public:

        typedef UINT32 size_type;

        direct_storage_file(direct_storage_scope& scope,
            const wchar_t *path);

        direct_storage_file(const direct_storage_file&) = delete;

        BY_HANDLE_FILE_INFORMATION get_file_information(void) const;

        void queue_request(IDStorageQueue *queue,
            const size_type srcOffset, const size_type count,
            ID3D12Resource *dst, const size_type dstOffset = 0);

        direct_storage_file operator =(const direct_storage_file&) = delete;

    private:

        ATL::CComPtr<IDStorageFile> _file;
    };

}
#endif /* defined(TRROJAN_WITH_DSTORAGE) */
