/// <copyright file="util.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/opencl/dat_raw_reader.h"


void trrojan::opencl::dat_raw_reader::read_files(const std::string dat_file_name,
                                                 const std::vector<unsigned char> &raw_data)
{
    // check file
    if (!dat_file_name.empty())
    {
        _prop.dat_file_name = dat_file_name;
    }
    else
    {
        throw "Empty .dat file name.";
    }

    try
    {
        read_dat(_prop.dat_file_name);
    }
    catch (...)
    {
        throw "Error reading .dat file.";
    }

    try
    {
        read_raw(_prop.raw_file_name);
    }
    catch (...)
    {
        throw "Error reading .raw file.";
    }

}

bool trrojan::opencl::dat_raw_reader::has_data()
{
    if (_raw_data.empty())
    {
        return false;
    }

    return true;
}


const std::vector<unsigned char> & trrojan::opencl::dat_raw_reader::data()
{
    if (!has_data())
    {
        throw "No data available";
    }
    return _raw_data;
}


void trrojan::opencl::dat_raw_reader::read_dat(const std::string dat_file_name)
{

}

void trrojan::opencl::dat_raw_reader::read_raw(const std::string raw_file_name)
{
}
