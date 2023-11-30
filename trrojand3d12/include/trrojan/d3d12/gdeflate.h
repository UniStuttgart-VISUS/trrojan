// <copyright file="gdeflate.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#if defined(TRROJAN_WITH_DSTORAGE)
#include <cinttypes>
#include <string>

#include <dstorage.h>

#include <winrt/base.h>


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Creates the specified <see cref="IDStorageCompressionCodec" />.
    /// </summary>
    /// <param name="format"></param>
    /// <returns></returns>
    winrt::com_ptr<IDStorageCompressionCodec> create_dstorage_codec(
        const DSTORAGE_COMPRESSION_FORMAT format);

    /// <summary>
    /// Compresses the given data into <paramref name="path" />.
    /// </summary>
    /// <param name="data"></param>
    /// <param name="cnt_data"></param>
    /// <param name="block_size"></param>
    /// <param name="path"></param>
    /// <returns></returns>
    std::vector<std::size_t> gdeflate_compress(
        const std::uint8_t *data,
        const std::size_t cnt_data,
        const std::size_t block_size,
        const std::string& path);

} /* end namespace d3d12 */
} /* end namespace trrojan */

#endif /* defined(TRROJAN_WITH_DSTORAGE) */
