/// <copyright file="image_helper.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Steffen Frey</author>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

#include <array>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <atlbase.h>
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")
#endif /* _WIN32 */

#ifdef TRROJAN_WITH_CIMG
#include "CImg.h"
#endif /* TRROJAN_WITH_CIMG */

#include "trrojan/export.h"


namespace trrojan
{

    /// <summary>
    /// std::array initializing helper.
    /// </summary>
    template <size_t N, class T>
    std::array<T, N> make_array(const T &v)
    {
        std::array<T, N> ret;
        ret.fill(v);
        return ret;
    }


#ifdef TRROJAN_WITH_CIMG
    /// <summary>
    /// Write an image to file <paramref name="filename" />.
    /// </summary>
    /// <param name="filename">The file name.</param>
    /// <param name="data">Raw data to write as an image.</param>
    /// <param name="dim">Input dimensions in x, y and z directions.</param>
    /// <param name="channels">Number of 'vector' channels, e.g. 3 for R, G, B.</param>
    /// <param name="flip">Optional bit indicator to flip the resulting image at target axis.</param>
    /// <param name="out_dim">The optional output dimension.</param>
    /// <tparam name="I3">3D dimension type.</tparam>
    template<typename I3>
    void cimg_write(std::string filename,
                    const unsigned char* data,
                    const std::array<I3, 3>& dim,
                    int channels,
                    unsigned char flip = 0,
                    const std::array<I3, 3>& out_dim = make_array<3>(0))
    {
        const int default_val = 0;

        cimg_library::CImg<unsigned char> out(dim.at(0), dim.at(1), dim.at(2), channels, default_val);
        for (size_t z = 0; z < dim.at(2); ++z)
        {
            for (size_t y = 0; y < dim.at(1); ++y)
            {
                for (size_t x = 0; x < dim.at(0); ++x)
                {
                    for (int c = 0; c < channels; ++c)
                    {
                        size_t vx = x;
                        if(flip & 1)
                            vx = (dim.at(0)-x-1);
                        size_t vy = y;
                        if(flip & 2)
                            vy = (dim.at(1)-y-1);
                        size_t vz = z;
                        if(flip & 4)
                            vz = (dim.at(2)-z-1);

                        out.data()[c*dim.at(0)*dim.at(1)*dim.at(2) + vz*dim.at(0)*dim.at(1) + vy*dim.at(0) + vx] =
                                ((unsigned char*) &data[0])[c + channels*(x + dim.at(0)*(y + dim.at(1)*z))];
                    }
                }
            }
        }
        if((out_dim.at(0) != 0 && out_dim.at(1) != 0)
                && (dim.at(0) != out_dim.at(0) || dim.at(1) != out_dim.at(1)))
        {
            out.resize(out_dim.at(0), out_dim.at(1), out_dim.at(2), channels, 5);
        }

        out.save(filename.c_str());
    }


    ///
    ///
    ///
    template<typename T, typename I3>
    void cimg_write(std::string filename,
                    const T* data,
                    const std::array<I3, 3> &dim,
                    int channels,
                    unsigned char flip = 0,
                    const std::array<I3, 3> &out_dim = make_array<3>((size_t)0))
    {
        std::vector<unsigned char> d(dim.at(0)*dim.at(1)*dim.at(2)*channels);
        for (size_t i = 0; i < dim.at(0)*dim.at(1)*dim.at(2)*channels; ++i)
        {
            const unsigned char v = (std::max)(0.0, (std::min)(data[i] + 0.5, 255.0));
            d.at(i) = v;
        }
        cimg_write(filename, &d[0], dim, channels, flip, out_dim);
    }


    ///
    ///
    ///
    template<typename T, typename I3>
    void cimg_write(std::string filename,
                    std::vector<T> data,
                    const std::array<I3, 3> &dim,
                    int channels,
                    unsigned char flip = 0,
                    const std::array<I3, 3> &out_dim = make_array<3>(0))
    {
        cimg_write(filename, &data[0], dim, channels, flip, out_dim);
    }


    ///
    ///
    ///
    template<typename T, typename I3>
    void cimg_read(std::vector<T>& data,
                   std::array<I3, 3>& vol_dim,
                   int& channels,
                   const std::string& filename,
                   bool force_grey = false,
                   bool force_dim = false)
    {  
        cimg_library::CImg<T> img(filename.c_str());

        if(force_dim && (vol_dim.at(0) != img.width() || vol_dim.at(1) != img.height()))
            img.resize(vol_dim.at(0), vol_dim.at(1), vol_dim.at(2), img.spectrum(), 5);

        vol_dim.at(0) = img.width();
        vol_dim.at(1) = img.height();
        vol_dim.at(2) = img.depth();
        channels = img.spectrum();

        assert(vol_dim.at(2) == 1);
        //std::cout << filename << ", there are #channels: " << channels << std::endl;
        if(force_grey)
            channels = 1;
        data.resize(vol_dim.at(0)*vol_dim.at(1)*vol_dim.at(2)*channels);

        size_t idx = 0;
        for(size_t z = 0; z < vol_dim.at(2); ++z)
        {
            for(size_t y = 0; y < vol_dim.at(1); ++y)
            {
                for(size_t x = 0; x < vol_dim.at(0); ++x)
                {
                    for(size_t c = 0; c < channels; ++c)
                    {
                        data[idx] = img(x, y, z, c);
                        bool all_channels_equal = true;
                        if(img.spectrum() > 1)
                            all_channels_equal = all_channels_equal
                                    && (img(x, y, z, 0) == img(x, y, z, 1));
                        if(img.spectrum() > 2)
                            all_channels_equal = all_channels_equal
                                    && (img(x, y, z, 0) == img(x, y, z, 2));
                        if(img.spectrum() > 3)
                            all_channels_equal = all_channels_equal
                                    && (img(x, y, z, 0) == img(x, y, z, 3));
                        assert(!force_grey || all_channels_equal);
                        idx++;
                    }
                }
            }
        }
    }
#endif /* TRROJAN_WITH_CIMG */


namespace detail {

    /// <summary>
    /// A traits class which allows deriving the pixel format from a pointer
    /// to pixels.
    /// </summary>
    template<class T> struct wic_format_traits { };

#ifdef _WIN32
    template<> struct TRROJANCORE_API wic_format_traits<unsigned char> {
        static const GUID id(const size_t cntChannels, const bool bgr = false);
    };

    template<> struct TRROJANCORE_API wic_format_traits<unsigned short> {
        static const GUID id(const size_t cntChannels, const bool bgr = false);
    };

    template<> struct TRROJANCORE_API wic_format_traits<float> {
        static const GUID id(const size_t cntChannels, const bool bgr = false);
    };

    template<> struct TRROJANCORE_API wic_format_traits<char> {
        static inline const GUID id(const size_t cntChannels, 
                const bool bgr = false) {
            return wic_format_traits<unsigned char>::id(cntChannels, bgr);
        }
    };

    template<> struct TRROJANCORE_API wic_format_traits<short> {
        static inline const GUID id(const size_t cntChannels,
                const bool bgr = false) {
            return wic_format_traits<unsigned short>::id(cntChannels, bgr);
        }
    };
#endif /* _WIN32 */
} /* end namespace detail */


#ifdef _WIN32
    /// <summary>
    /// Get a WIC bitmap for the specified raw data.
    /// </summary>
    ATL::CComPtr<IWICBitmapSource> TRROJANCORE_API get_wic_bitmap(
        IWICImagingFactory *wic, const void *data, const size_t width,
        const size_t height, const GUID& fmtData, size_t stride = 0,
        GUID fmtBitmap = ::GUID_NULL);
#endif /* _WIN32*/


#ifdef _WIN32
    /// <summary>
    /// Gets the bits per pixel for the given format.
    /// </summary>
    /// <param name="wic"></param>
    /// <param name="format"></param>
    /// <returns></returns>
    /// <exception cref="std::invalid_argument"></exception>
    /// <exception cref="std::runtime_error"></exception>
    size_t TRROJANCORE_API get_wic_bpp(IWICImagingFactory *wic,
        const GUID& format);
#endif /* _WIN32 */


#ifdef _WIN32
    /// <summary>
    /// Determines the WIC codec fromt he extension of the given file name.
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    /// <exception cref="std::invalid_argument">If the codec could not be
    /// determined from <see cref="fileName" />.</exception>
    GUID TRROJANCORE_API get_wic_codec(const std::string& fileName);
#endif /* _WIN32 */


#ifdef _WIN32
    /// <summary>
    /// Instantiates a new <see cref="IWICImagingFactory" />.
    /// </summary>
    /// <returns></returns>
    /// <exception cref=""></exception>
    ATL::CComPtr<IWICImagingFactory> TRROJANCORE_API get_wic_factory(void);
#endif /* _WIN32 */


#ifdef _WIN32
    /// <summary>
    /// Saves an image using WIC.
    /// </summary>
    /// <param name="wic">The <see cref="IWICImagingFactory" /> used to perform
    /// the operation.</param>
    /// <param name="data">Pointer to the data to be saved.</param>
    /// <param name="width">Width of the image in pixels.</param>
    /// <param name="height">Height of the image in pixels.</param>
    /// <param name="stride">The size of a single image line in bytes. If this
    /// parameter is zero, it will be computed from <paramref name="width" />
    /// and <paramref name="fmtData" />.</param>
    /// <param name="fmtData">The format of <paramref name="data" />.</param>
    /// <param name="fileName">The name of the output file.</param>
    /// <param name="fmtFile">The codec ID for the file. If this parameter
    /// is <see cref="GUID_NULL" />, which is the default, the codec is
    /// determined from the file name extension.</param>
    /// <exception cref=""></exception>
    void TRROJANCORE_API wic_save(IWICImagingFactory *wic, const void *data,
        const size_t width, const size_t height, size_t stride,
        const GUID& fmtData, const std::string& fileName, GUID fmtFile);
#endif /* _WIN32 */

    /// <summary>
    /// Saves bitmap data to the specified file.
    /// </summary>
    template<class T>
    void save_image(const std::string& fileName, const T *data,
        const size_t width, const size_t height, const size_t channels);

} /* end namespace trrojan */

#include "trrojan/image_helper.inl"
