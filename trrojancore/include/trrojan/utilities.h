/// <copyright file="utilities.h" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>

#pragma once

#ifdef _UWP

#include <sstream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <atlbase.h>
#include <Windows.h>
#include <d3d11.h>
#include <d3d12.h>

#include "trrojan/result.h"

namespace trrojan {
    // Copyright (c) Microsoft Corporation.
    // Licensed under the MIT License.
    //
    // Modified to use std::filesystem
    inline std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path) {
        bool fileExists = false;
        try {
            std::ifstream file;
            file.exceptions(std::ios::failbit | std::ios::badbit);
            file.open(path, std::ios::binary | std::ios::ate);
            fileExists = true;
            // If tellg fails then it will throw an exception instead of returning -1.
            std::vector<uint8_t> data(static_cast<size_t>(file.tellg()));
            file.seekg(0, std::ios::beg);
            file.read(reinterpret_cast<char*>(data.data()), data.size());
            return data;
        }
        catch (const std::ios::failure&) {
            // The exception only knows that the failbit was set so it doesn't contain anything useful.
            //throw std::runtime_error(std::format("Failed to {} file: {}", fileExists ? "read" : "open", path.string()));
            throw std::runtime_error(std::string("Failed to " + std::string(fileExists ? "read" : "open") + " file: " + path.string()));
        }
    }

    // Copyright (c) Microsoft Corporation.
    // Licensed under the MIT License.
    //
    // Modified to use only UWP version
    inline std::filesystem::path GetAppFolder() {
        HMODULE thisModule;
        thisModule = nullptr;

        wchar_t moduleFilename[MAX_PATH];
        ::GetModuleFileName(thisModule, moduleFilename, (DWORD)std::size(moduleFilename));
        std::filesystem::path fullPath(moduleFilename);
        return fullPath.remove_filename();
    }

    // Copyright (c) Microsoft Corporation.
    // Licensed under the MIT License.
    inline std::filesystem::path GetPathInAppFolder(const std::filesystem::path& filename) {
        return GetAppFolder() / filename;
    }

    inline void result_to_string (std::ostringstream& oss, const basic_result& result) {

        auto print_name = [](std::ostringstream& oss, const std::string& str) {
            const auto len = str.length();
            oss << str;
            for (size_t i = len; i < 0/*padding*/; ++i) {
                oss << ' ';
            }
            oss << ": ";
        };


        for (size_t i = 0; i < result.measurements(); ++i) {
            for (auto& c : result.configuration()) {
                print_name(oss, c.name());
                oss << c.value() << std::endl;
            }
            for (size_t j = 0; j < result.values_per_measurement(); ++j) {
                print_name(oss, result.result_names()[j]);
                oss << result.raw_result(i, j) << std::endl;
            }
            oss << std::endl;
        }
        oss << std::endl;
    };
}
#endif // _UWP