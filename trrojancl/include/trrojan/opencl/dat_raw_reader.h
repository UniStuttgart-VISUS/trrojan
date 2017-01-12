/// <copyright file="util.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#pragma once

#include "trrojan/opencl/export.h"

#include <vector>
#include <string>
#include <array>

namespace trrojan {
namespace opencl {

    /// <summary>
    /// The Properties struct that can hold .dat and .raw file information.
    /// </summary>
    struct Properties
    {
        std::string dat_file_name;
        std::string raw_file_name;
        size_t raw_file_size;

        std::array<uint, 3> volume_res;
        std::array<uint, 3> slice_thickness;
        std::string format;                     // UCHAR, USHORT,...
    };

    /// <summary>
    /// Dat-raw file reader.
    /// </summary>
    class TRROJANCL_API dat_raw_reader
    {

    public:

        void read_files(const std::string dat_file_name,
                        const std::vector<unsigned char> &raw_data);

        bool has_data();

        const std::vector<unsigned char> & data();

    private:

        void read_dat(const std::string dat_file_name);

        void read_raw(const std::string raw_file_name);

        Properties _prop;

        std::vector<unsigned char> _raw_data;
    };
}
}
