/// <copyright file="io.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <vector>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Read a whole binary file at the location designated by
    /// <see cref="path" />.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    std::vector<char> read_binary_file(const char *path);

    /// <summary>
    /// Read a whole binary file at the location designated by
    /// <see cref="path" />.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    inline std::vector<char> read_binary_file(const std::string& path) {
        return read_binary_file(path.c_str());
    }

}
