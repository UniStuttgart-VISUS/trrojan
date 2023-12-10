// <copyright file="gdeflate.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#if defined(TRROJAN_WITH_DSTORAGE)
#include "trrojan/d3d12/gdeflate.h"

#include "trrojan/io.h"

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

#include <atlexcept.h>


/*
 * trrojan::d3d12::create_dstorage_codec
 */
winrt::com_ptr<IDStorageCompressionCodec> trrojan::d3d12::create_dstorage_codec(
        const DSTORAGE_COMPRESSION_FORMAT format) {
    auto hr = S_OK;
    winrt::com_ptr<IDStorageCompressionCodec> retval;

    switch (format) {
        case DSTORAGE_COMPRESSION_FORMAT_GDEFLATE:
            hr = ::DStorageCreateCompressionCodec(format, 0,
                IID_PPV_ARGS(retval.put()));
            break;

        default:
            hr = E_INVALIDARG;
            break;
    }

    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::gdeflate_compress
 */
std::vector<std::size_t> trrojan::d3d12::gdeflate_compress(
        const std::uint8_t *data,
        const std::size_t cnt_data,
        const std::size_t block_size,
        const std::wstring& path) {
    // Adapted from https://github.com/microsoft/DirectStorage/blob/main/Samples/GpuDecompressionBenchmark/GpuDecompressionBenchmark.cpp
    if (data == nullptr) {
        throw ATL::CAtlException(E_POINTER);
    }

    const auto cnt_blocks = (cnt_data + block_size - 1) / block_size;
    std::vector<std::vector<std::uint8_t>> blocks;
    std::atomic<std::size_t> cur_block;
    std::vector<std::thread> workers;

    // Reserve output data and worker threads.
    blocks.resize(cnt_blocks);
    workers.reserve(std::thread::hardware_concurrency());

    for (unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        workers.emplace_back([&](void) {
            auto codec = create_dstorage_codec(
                DSTORAGE_COMPRESSION_FORMAT_GDEFLATE);

            while (true) {
                const auto b = cur_block++;
                if (b >= cnt_blocks) {
                    return;
                }

                const auto offset = b * block_size;
                const auto size = (std::min)(cnt_data - offset, block_size);

                // Allocate sufficient memory to hold all compressed data.
                std::size_t compressed = codec->CompressBufferBound(size);
                blocks[b].resize(compressed);

                // Compress the block.
                auto hr = codec->CompressBuffer(data + offset, size,
                    DSTORAGE_COMPRESSION_BEST_RATIO, blocks[b].data(),
                    blocks[b].size(), &compressed);
                if (FAILED(hr)) {
                    throw ATL::CAtlException(hr);
                }

                // Resize above was to upper bound, we need to make sure that we
                // truncate to the actual size now.
                blocks[b].resize(compressed);
            }
        });
    }

    // Wait for all workers ot exit.
    for (auto& w : workers) {
        w.join();
    }

    // Write the output file and compute the prefix sum of the compressed
    // blocks.
    winrt::file_handle file(::CreateFileW(path.c_str(), GENERIC_WRITE,
        FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, NULL));
    if (!file) {
        throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));
    }

    std::vector<std::size_t> retval;
    retval.reserve(blocks.size());

    for (auto& b : blocks) {
        std::size_t sum = 0;
        if (!retval.empty()) {
            sum = retval.back();
        }

        retval.push_back(sum + b.size());
        write_all_bytes(file.get(), b.data(), b.size());
    }

    return retval;
}
#endif /* defined(TRROJAN_WITH_DSTORAGE) */
