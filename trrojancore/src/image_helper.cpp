// <copyright file="image_helper.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/image_helper.h"

#include <cassert>
#include <climits>
#include <sstream>
#include <stdexcept>

#include "trrojan/io.h"
#include "trrojan/text.h"


#if defined(_WIN32)
/*
 * trrojan::detail::wic_format_traits<unsigned char>::id
 */
const GUID trrojan::detail::wic_format_traits<unsigned char>::id(
        const size_t cntChannels, const bool bgr) {
    switch (cntChannels) {
        case 1: return GUID_WICPixelFormat8bppGray;
        case 3: return bgr ? GUID_WICPixelFormat24bppBGR : GUID_WICPixelFormat24bppRGB;
        case 4: return bgr ? GUID_WICPixelFormat32bppBGRA : GUID_WICPixelFormat32bppRGBA;
        default: throw std::invalid_argument("Cannot derive pixel format.");
    }
}


/*
 * trrojan::detail::wic_format_traits<unsigned short>::id
 */
const GUID trrojan::detail::wic_format_traits<unsigned short>::id(
        const size_t cntChannels, const bool bgr) {
    switch (cntChannels) {
        case 1: return GUID_WICPixelFormat16bppGray;
        case 3: return bgr ? GUID_WICPixelFormat48bppBGR : GUID_WICPixelFormat48bppBGR;
        case 4: return bgr ? GUID_WICPixelFormat64bppBGRA : GUID_WICPixelFormat64bppRGBA;
        default: throw std::invalid_argument("Cannot derive pixel format.");
    }
}


/*
 * trrojan::detail::wic_format_traits<float>::id
 */
const GUID trrojan::detail::wic_format_traits<float>::id(
        const size_t cntChannels, const bool bgr) {
    if (bgr) {
        throw std::invalid_argument("Cannot derive pixel format.");
    }

    switch (cntChannels) {
        case 1: return GUID_WICPixelFormat32bppGrayFloat;
        case 3: return GUID_WICPixelFormat96bppRGBFloat;
        case 4: return GUID_WICPixelFormat128bppRGBAFloat;
        default: throw std::invalid_argument("Cannot derive pixel format.");
    }
}
#endif /* defined(_WIN32) */


#if defined(_WIN32)
/*
 * trrojan::get_wic_bitmap
 */
winrt::com_ptr<IWICBitmapSource> TRROJANCORE_API trrojan::get_wic_bitmap(
        IWICImagingFactory *wic, const void *data, const size_t width,
        const size_t height, const GUID& fmtData, size_t stride,
        GUID fmtBitmap) {
    winrt::com_ptr<IWICBitmap> bmp;
    winrt::com_ptr<IWICFormatConverter> conv;
    BYTE *dst = nullptr;
    UINT dstSize = 0;
    UINT dstStride = 0;
    HRESULT hr = S_OK;
    winrt::com_ptr<IWICBitmapLock> lock;

    if (wic == nullptr) {
        throw std::invalid_argument("'wic' must not be nullptr.");
    }

    /* Compute stride if not specified. */
    if (stride == 0) {
        stride = trrojan::get_wic_bpp(wic, fmtData);
        assert((stride % CHAR_BIT) == 0);
        stride /= CHAR_BIT;
        stride = (width * stride + 3) & ~3;
    }

    /* Assume no conversion if no target format was given. */
    if (::IsEqualGUID(fmtBitmap, ::GUID_NULL)) {
        fmtBitmap = fmtData;
    }

    /* Create the bitmap */
    assert(width <= UINT_MAX);
    assert(height <= UINT_MAX);
    hr = wic->CreateBitmap(static_cast<UINT>(width), static_cast<UINT>(height),
        fmtData, WICBitmapCacheOnDemand, bmp.put());
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to instantiate WIC bitmap converter with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    /* Update the data. */
    hr = bmp->Lock(nullptr, WICBitmapLockWrite, lock.put());
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to lock bitmap with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    hr = lock->GetStride(&dstStride);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to obtain data stride for bitmap with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    hr = lock->GetDataPointer(&dstSize, &dst);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to obtain data pointer for bitmap with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    for (size_t y = 0; y < height; ++y) {
        auto s = static_cast<const BYTE *>(data) + y * stride;
        ::memcpy(dst + y * dstStride, s, stride);
    }

    lock = nullptr;

    /* Add conversion as requested. */
    if (!::IsEqualGUID(fmtBitmap, fmtData)) {
        hr = wic->CreateFormatConverter(conv.put());
        if (FAILED(hr)) {
            std::stringstream msg;
            msg << "Failed to instantiate WIC format converter with error code "
                << hr << "." << std::ends;
            throw std::runtime_error(msg.str());
        }

        hr = conv->Initialize(bmp.get(), fmtBitmap, WICBitmapDitherTypeNone,
            nullptr, 0.0, WICBitmapPaletteTypeCustom);
        if (FAILED(hr)) {
            std::stringstream msg;
            msg << "Failed to initialise WIC format converter with error code "
                << hr << "." << std::ends;
            throw std::runtime_error(msg.str());
        }

        return conv;
    } else {
        return bmp;
    }
}
#endif /* defined(_WIN32) */


#if defined(_WIN32)
/*
 * trrojan::get_wic_bpp
 */
size_t TRROJANCORE_API trrojan::get_wic_bpp(IWICImagingFactory *wic,
        const GUID& format) {
    winrt::com_ptr<IWICComponentInfo> ci;
    HRESULT hr = S_OK;
    winrt::com_ptr<IWICPixelFormatInfo> pfi;
    UINT retval = 0;

    if (wic == nullptr) {
        throw std::invalid_argument("'wic' must not be nullptr.");
    }

    hr = wic->CreateComponentInfo(format, ci.put());
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create a WIC component info with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    if (!ci.try_as(pfi)) {
        std::stringstream msg;
        msg << "Failed to retrieve WIC pixel format info with error code "
            << hr << ". Please note that only pixel formats are allowed "
            "for the parameter 'format'," << std::ends;
        throw std::runtime_error(msg.str());
    }

    hr = pfi->GetBitsPerPixel(&retval);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to query bits per pixel with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    return static_cast<size_t>(retval);
}
#endif /* defined(_WIN32) */


#if defined(_WIN32)
/*
 * trrojan::get_wic_codec
 */
GUID TRROJANCORE_API trrojan::get_wic_codec(const std::string& fileName) {
    if (fileName.empty()) {
        throw std::invalid_argument("'fileName' must not be empty.");
    }

    auto ext = trrojan::tolower(trrojan::get_extension(fileName));

    if ((ext == ".bmp")) {
        return ::GUID_ContainerFormatBmp;

    } else if ((ext == ".png")) {
        return ::GUID_ContainerFormatPng;

    } else if ((ext == ".ico")) {
        return ::GUID_ContainerFormatIco;

    } else if ((ext == ".jpg") || (ext == ".jpeg") || (ext == ".jpe")
            || (ext == ".jfif")) {
        return ::GUID_ContainerFormatJpeg;

    } else if ((ext == ".tif") || (ext == ".tiff")) {
        return ::GUID_ContainerFormatTiff;

    } else if ((ext == ".gif")) {
        return ::GUID_ContainerFormatGif;

    } else if ((ext == ".wmp")) {
        return ::GUID_ContainerFormatWmp;

    } else  if ((ext == ".dds")) {
        return ::GUID_ContainerFormatDds;

    } else {
        std::stringstream msg;
        msg << "Unable to determine WIC image format from file name extension "
            "\"" << ext << "\"" << std::ends;
        throw std::invalid_argument(msg.str());
    }
}
#endif /* defined(_WIN32) */


#if defined(_WIN32)
/*
 * trrojan::get_wic_factory
 */
winrt::com_ptr<IWICImagingFactory> TRROJANCORE_API trrojan::get_wic_factory(void) {
    winrt::com_ptr<IWICImagingFactory> retval;

    auto hr = ::CoCreateInstance(CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), retval.put_void());
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "CoCreateInstance failed with error code " << hr
            << "for CLSID_WICImagingFactory." << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}
#endif /* defined(_WIN32) */


#if defined(_WIN32)
/*
 * trrojan::wic_save
 */
void TRROJANCORE_API trrojan::wic_save(IWICImagingFactory *wic,
        const void *data, const size_t width, const size_t height,
        size_t stride, const GUID& fmtData, const std::string& fileName,
        GUID fmtFile) {
    winrt::com_ptr<IWICBitmapEncoder> encoder;
    GUID fmtFrame = fmtData;
    winrt::com_ptr<IWICBitmapFrameEncode> frame;
    HRESULT hr = S_OK;
    winrt::com_ptr<IWICStream> stream;

    if (wic == nullptr) {
        throw std::invalid_argument("'wic' must not be nullptr.");
    }
    if (data == nullptr) {
        throw std::invalid_argument("'data' must not be nullptr.");
    }

    /* Determine codec from file name if not specified. */
    if (::IsEqualGUID(fmtFile, ::GUID_NULL) != FALSE) {
        fmtFile = get_wic_codec(fileName);
    }

    /* Compute stride if not specified. */
    if (stride == 0) {
        stride = trrojan::get_wic_bpp(wic, fmtData);
        assert((stride % CHAR_BIT) == 0);
        stride /= CHAR_BIT;
        stride = (width * stride + 3) & ~3;
    }

    /* Create the stream for the output file. */
    hr = wic->CreateStream(stream.put());
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create WIC stream with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    auto fn = from_utf8(fileName);
    hr = stream->InitializeFromFilename(fn.c_str(), GENERIC_WRITE);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to initialise WIC stream from file name \"" << fileName
            << "\" with error code " << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    /* Create the encoder for the codec and initialise it. */
    hr = wic->CreateEncoder(fmtFile, nullptr, encoder.put());
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create WIC encoder with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    hr = encoder->Initialize(stream.get(), WICBitmapEncoderNoCache);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to initialise WIC encoder with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    /* Create frame and write the bitmap into it. */
    hr = encoder->CreateNewFrame(frame.put(), nullptr);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to add a new frame with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    hr = frame->Initialize(nullptr);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to initialise frame with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    hr = frame->SetSize(static_cast<UINT>(width), static_cast<UINT>(height));
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to set size of frame with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    hr = frame->SetPixelFormat(&fmtFrame);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to set format of frame with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    if (fmtFrame == fmtData) {
        // Encoder supports input format, can write directly.
        hr = frame->WritePixels(static_cast<UINT>(height),
            static_cast<UINT>(stride),
            static_cast<UINT>(height * stride),
            const_cast<BYTE *>(static_cast<const BYTE *>(data)));
    } else {
        // We need a conversion step.
        auto bmp = trrojan::get_wic_bitmap(wic, data, width, height, fmtData,
            stride, fmtFrame);
        hr = frame->WriteSource(bmp.get(), nullptr);
    }
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to write to frame with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    hr = frame->Commit();
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to commit frame with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }

    hr = encoder->Commit();
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to commit image with error code "
            << hr << "." << std::ends;
        throw std::runtime_error(msg.str());
    }
}
#endif /* _WIN32 */
