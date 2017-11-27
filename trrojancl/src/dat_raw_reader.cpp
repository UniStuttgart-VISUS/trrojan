/// <copyright file="util.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2017 SFB-TRR 161. Alle Rechte vorbehalten.
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
void trrojan::opencl::dat_raw_reader::read_files(const std::string dat_file_name)
{
    // check file
    if (!dat_file_name.empty())
    {
        _prop.dat_file_name = dat_file_name;
    }
    else
    {
        throw std::invalid_argument("dat file name must not be empty.");
    }

    try
    {
        read_dat(_prop.dat_file_name);
        read_raw(_prop.raw_file_name);
    }
    catch (std::runtime_error e)
    {
        throw e;
    }
}


/*
 * trrojan::opencl::dat_raw_reader::has_data
 */
bool trrojan::opencl::dat_raw_reader::has_data() const
{
    return !(_raw_data.empty());
}


/*
 * trrojan::opencl::dat_raw_reader::data
 */
const std::vector<char> & trrojan::opencl::dat_raw_reader::data() const
{
    if (!has_data())
    {
        throw std::runtime_error("No data available.");
    }
//    return std::move(_raw_data);
    return _raw_data;
}

const trrojan::opencl::Properties &trrojan::opencl::dat_raw_reader::properties() const
{
    if (!has_data())
    {
        throw std::runtime_error("No properties available.");
    }
    return _prop;
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
        throw std::runtime_error("Could not open .dat file " + dat_file_name);
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

    // check that values read from the dat file
    if (_prop.raw_file_name.empty())
    {
        throw std::runtime_error("Missing raw file name declaration in " + dat_file_name);
    }
    if (_prop.volume_res.empty())
    {
        throw std::runtime_error("Missing volume resolution declaration in " + dat_file_name);
    }
    if (_prop.slice_thickness.at(0) == -1.0 || _prop.slice_thickness.at(1) == -1.0
            || _prop.slice_thickness.at(2) == -1.0)
    {
        std::cerr << "WARNING: Missing slice thickness declaration in " << dat_file_name
                  << std::endl;
        std::cerr << "Assuming a slice thickness of 1.0 in each dimension." << std::endl;
        _prop.slice_thickness.fill(1.0);
    }
    if (_prop.format.empty())
    {
        std::cerr << "WARNING: Missing format declaration in " << dat_file_name << std::endl;
        std::cerr << "Trying to calculate the format from raw file size and volume resolution."
                  << std::endl;
    }
}

/*
 * trrojan::opencl::dat_raw_reader::read_raw
 */
void trrojan::opencl::dat_raw_reader::read_raw(const std::string raw_file_name)
{
    if (raw_file_name.empty())
    {
        throw std::invalid_argument("Raw file name must not be empty.");
    }
    assert(!raw_file_name.empty());

    // append .raw file name to .dat file name path
    std::size_t found = _prop.dat_file_name.find_last_of("/\\");
    std::string name_with_path;
    if (found != std::string::npos && _prop.dat_file_name.size() >= found)
    {
        name_with_path = _prop.dat_file_name.substr(0, found + 1) + raw_file_name;
    }
    else
    {
        name_with_path = raw_file_name;
    }

    // use plain old C++ method for file read here that is much faster than iterator
    // based approaches according to:
    // http://insanecoding.blogspot.de/2011/11/how-to-read-in-file-in-c.html
    std::ifstream is(name_with_path, std::ios::in | std::ifstream::binary);
    if (is)
    {
        // get length of file:
        is.seekg(0, is.end);
//#ifdef _WIN32
        // HACK: to support files bigger than 2048 MB on windows
//        _prop.raw_file_size = *(__int64 *)(((char *)&(is.tellg())) + 8);
//#else
        _prop.raw_file_size = is.tellg();
//#endif
        is.seekg( 0, is.beg );

        _raw_data.clear();
        _raw_data.resize(_prop.raw_file_size);

        // read data as a block:
        is.read(_raw_data.data(), _prop.raw_file_size);

        if (!is)
        {
            throw std::runtime_error("Error reading " + raw_file_name);
        }
        is.close();
    }
    else
    {
        throw std::runtime_error("Could no open " + raw_file_name);
    }

    // if format was not specified in .dat file, try to calculate it from
    // file size and volume resolution
    if (_prop.format.empty())
    {
        unsigned int bytes = _raw_data.size() / (static_cast<long long>(_prop.volume_res[0]) *
                                                 static_cast<long long>(_prop.volume_res[1]) *
                                                 static_cast<long long>(_prop.volume_res[2]));
        switch (bytes)
        {
        case 1:
            _prop.format = "UCHAR";
            std::cout << "Format determined as UCHAR." << std::endl;
            break;
        case 2:
            _prop.format = "USHORT";
            std::cout << "Format determined as USHORT." << std::endl;
            break;
        default: throw std::runtime_error("Could not resolve missing format specification.");
        }
    }
}
