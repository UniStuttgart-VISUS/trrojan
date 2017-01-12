/// <copyright file="util.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/opencl/dat_raw_reader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <cassert>

/*
 * trrojan::opencl::dat_raw_reader::read_files
 */
void trrojan::opencl::dat_raw_reader::read_files(const std::string dat_file_name,
                                                 const std::vector<char> &raw_data)
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


/*
 * trrojan::opencl::dat_raw_reader::has_data
 */
bool trrojan::opencl::dat_raw_reader::has_data() const
{
    if (_raw_data.empty())
    {
        return false;
    }

    return true;
}


/*
 * trrojan::opencl::dat_raw_reader::data
 */
const std::vector<char> & trrojan::opencl::dat_raw_reader::data() const
{
    if (!has_data())
    {
        throw "No data available";
    }
    return _raw_data;
}


/*
 * trrojan::opencl::dat_raw_reader::size
 */
size_t trrojan::opencl::dat_raw_reader::size()
{
    return _prop.raw_file_size;
}


/*
 * trrojan::opencl::dat_raw_reader::read_dat
 */
void trrojan::opencl::dat_raw_reader::read_dat(const std::string dat_file_name)
{
    std::ifstream dat_file(dat_file_name);

    std::string line;
    std::vector<std::vector<std::string>> lines;

    // read lines from .dat file and split on whitespace
    if (dat_file.is_open())
    {
        while (std::getline(dat_file, line))
        {
            std::istringstream iss(line);
            std::vector<std::string> tokens;
            std::copy(std::istream_iterator<std::string>(iss),
                      std::istream_iterator<std::string>(),
                      std::back_inserter(tokens));
            lines.push_back(tokens);
        }
        dat_file.close();
    }
    else
    {
        throw "Could not open .dat file.";
    }

    for (auto l : lines)
    {
        if (!l.empty())
        {
            std::string name = l.at(0);
            if (name.find("ObjectFileName") != std::string::npos && l.size() > 1u)
            {
                _prop.raw_file_name = l.at(1);
            }
            else if (name.find("Resolution") != std::string::npos && l.size() > 3u)
            {
                for (size_t i = 1; i < l.size(); ++i)
                {
                    _prop.volume_res.at(i - 1) = std::stoul(l.at(i));
                }
            }
            else if (name.find("SliceThickness") != std::string::npos && l.size() > 3u)
            {
                for (size_t i = 1; i < l.size(); ++i)
                {
                    _prop.slice_thickness.at(i - 1) = std::stod(l.at(i));
                }
            }
            if (name.find("Format") != std::string::npos && l.size() > 1u)
            {
                _prop.format = l.at(1);
            }
        }
    }

    if (_prop.raw_file_name.empty() || _prop.format.empty() || _prop.volume_res.empty())
    {
        throw "Missing property in .dat file.";
    }
}

/*
 * trrojan::opencl::dat_raw_reader::read_raw
 */
void trrojan::opencl::dat_raw_reader::read_raw(const std::string raw_file_name)
{
    assert(!raw_file_name.empty());
    // append .raw file name to .dat file name path
    std::size_t found = _prop.dat_file_name.find_last_of("/\\");
    std::string name_with_path = _prop.dat_file_name.substr(0, found) + "/\\" + raw_file_name;

    std::ifstream is(name_with_path, std::ios::in | std::ifstream::binary);
    if (is)
    {
        // get length of file:
        is.seekg(0, is.end);
#ifdef _WIN32
        // HACK: to support files bigger than 2048 MB on windows
        _prop.raw_file_size = *(__int64 *)(((char *)&(is.tellg())) + 8);
#else
        _prop.raw_file_size = is.tellg();
#endif
        is.seekg( 0, is.beg );

        _raw_data.clear();
        _raw_data.resize(_prop.raw_file_size);

        // read data as a block:
        is.read(_raw_data.data(), _prop.raw_file_size);

        if (!is)
        {
            throw "Error reading " + raw_file_name;
        }
        is.close();
    }
    else
    {
        throw "Could no open " + raw_file_name;
    }
}
