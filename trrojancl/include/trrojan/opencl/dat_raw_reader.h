/// <copyright file="util.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2017 SFB-TRR 161. Alle Rechte vorbehalten.
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
        size_t raw_file_size = 0;

        std::array<unsigned int, 3> volume_res;
        std::array<double, 3> slice_thickness = {{1.0, 1.0, 1.0}};
        std::string format;                     // UCHAR, USHORT,...
    };

    /// <summary>
    /// Dat-raw volume data file reader.
    /// Based on a description in a text file ".dat", raw voxel data is read from a
    /// binary file ".raw". The dat-file should contain information on the file name of the
    /// raw-file, the resolution of the volume, the data format of the scalar data and possibly
    /// the slice thickness (default is 1.0 in each dimension).
    /// The raw data is stored in a vector of chars.
    /// </summary>
    class TRROJANCL_API dat_raw_reader
    {

    public:

        /// <summary>
        /// Read the dat file of the given name and based on the content, the raw data.
        /// Saves volume data set properties and scalar data in member variables.
        /// </summary>
        /// <param name="dat_file_name">Name and full path of the dat file</param>
        /// <param name="raw_data">Reference to a vector where the read raw data
        /// is stored in.</param>
        /// <throws>If one of the files could not be read.</throws
        void read_files(const std::string dat_file_name);

        /// <summary>
        /// Get the read status of hte objects.
        /// <summary>
        /// <returns><c>true</c> if raw data has been read, <c>false</c> otherwise.</returns>
        bool has_data() const;

        /// <summary>
        /// Get the raw data read. This method movs the data.
        /// </summary>
        /// <throws>If no raw data has been read.</throws>
        const std::vector<char> &data() const;

    private:

        /// <summary>
        /// Read the dat textfile.
        /// <summary>
        /// <param name="dat_file_name"> Name and full path of the dat text file.</param>
        void read_dat(const std::string dat_file_name);

        /// <summary>
        /// Read scalar voxel data from a given raw file.
        /// <summary>
        /// <remarks>This method does not check for a valid file name except for an assertion
        /// that it is not empty.</remarks>
        /// <param name="raw_file_name"> Name of the raw data file without the path.</param>
        /// <throws>If the given file could not be opened or read.</throws>
        void read_raw(const std::string raw_file_name);

        /// <summary>
        /// Properties of the volume data set.
        /// <summary>
        Properties _prop;

        /// <summary>
        /// The raw voxel data.
        /// <summary>
        std::vector<char> _raw_data;
    };
}
}