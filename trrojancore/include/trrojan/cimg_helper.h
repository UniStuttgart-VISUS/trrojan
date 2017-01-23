/// <copyright file="configuration.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Steffen Frey</author>
/// <author>Valentin Bruder</author>

#pragma once

#include <cstdlib>
#define cimg_display 0
#define cimg_use_png
#include <CImg.h>
#include <string>
#include <vector>
#include <cassert>

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
                const std::array<I3, 3> &out_dim = make_array<3>(0))
{
    std::vector<unsigned char> d(dim.at(0)*dim.at(1)*dim.at(2)*channels);
    for (int i = 0; i < dim.at(0)*dim.at(1)*dim.at(2)*channels; ++i)
    {
        const unsigned char v = std::max(0.0, std::min(data[i] + 0.5, 255.0));
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

}
