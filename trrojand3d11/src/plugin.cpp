// <copyright file="plugin.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2016 - 2022 Visualisierungsinstitut der Universit�t Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph M�ller</author>

#include "trrojan/d3d11/plugin.h"

#include "trrojan/io.h"

#include "trrojan/d3d11/cs_volume_benchmark.h"
#include "trrojan/d3d11/environment.h"
#include "trrojan/d3d11/sphere_benchmark.h"
#include "trrojan/d3d11/two_pass_volume_benchmark.h"


/// <summary>
/// Handle of the plugin DLL.
/// </summary>
static HINSTANCE hTrrojanDll = NULL;


/// <summary>
/// Entry point of the DLL.
/// </summary>
BOOL WINAPI DllMain(HINSTANCE hDll, DWORD reason, LPVOID reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            ::DisableThreadLibraryCalls(hDll);
            ::hTrrojanDll = hDll;
            break;

        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}


/// <summary>
/// Gets a new instance of the plugin descriptor.
/// </summary>
extern "C" TRROJAND3D11_API trrojan::plugin_base *get_trrojan_plugin(void) {
    return new trrojan::d3d11::plugin();
}


/*
 * trrojan::d3d11::plugin::get_directory
 */
std::string trrojan::d3d11::plugin::get_directory(void) {
    auto retval = get_location();
    auto it = std::find(retval.rbegin(), retval.rend(),
        directory_separator_char);
    return std::string(retval.begin(), it.base());
}


/*
 * trrojan::d3d11::plugin::get_location
 */
std::string trrojan::d3d11::plugin::get_location(void) {
    std::vector<char> retval(MAX_PATH);

    auto len = ::GetModuleFileNameA(::hTrrojanDll, retval.data(),
        static_cast<DWORD>(retval.size()));

    while (len == retval.size()) {
        assert(retval.size() > 1);
        retval.resize(retval.size() + retval.size() / 2);
        len = ::GetModuleFileNameA(::hTrrojanDll, retval.data(),
            static_cast<DWORD>(retval.size()));
    }

    retval[len] = 0;

    return retval.data();
}


#if !defined(TRROJAN_FOR_UWP)
/*
 * trrojan::d3d11::plugin::load_resource
 */
std::vector<std::uint8_t> trrojan::d3d11::plugin::load_resource(LPCTSTR name,
        LPCTSTR type) {
    auto hRes = ::FindResource(::hTrrojanDll, name, type);
    if (hRes == NULL) {
        std::error_code ec(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to find a resource.");
    }

    auto hGlobal = ::LoadResource(::hTrrojanDll, hRes);
    if (hGlobal == NULL) {
        std::error_code ec(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to load a resource.");
    }
    // Note: 'hGlobal' must not be freed. See
    // https://msdn.microsoft.com/de-de/library/windows/desktop/ms648046(v=vs.85).aspx

    auto hLock = ::LockResource(hGlobal);
    if (hLock == NULL) {
        std::error_code ec(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to lock a resource.");
    }


    auto retval = std::vector<std::uint8_t>(::SizeofResource(::hTrrojanDll,
        hRes));
    ::memcpy(retval.data(), hLock, retval.size());

    UnlockResource(hLock);

    return retval;
}
#endif /* !defined(TRROJAN_FOR_UWP) */



/*
 * trrojan::d3d12::plugin::load_shader_asset
 */
std::vector<std::uint8_t> trrojan::d3d11::plugin::load_shader_asset(
        const std::string& file) {
    const auto path = combine_path(plugin::get_directory(),
        "trrojand3d11", "d3d11", file);
    return read_binary_file(path);
}



/*
 * trrojan::d3d11::plugin::~plugin
 */
trrojan::d3d11::plugin::~plugin(void) { }


/*
 * trrojan::d3d11::plugin::create_benchmarks
 */
size_t trrojan::d3d11::plugin::create_benchmarks(benchmark_list& dst) const {
    auto retval = dst.size();

    dst.emplace_back(std::make_shared<sphere_benchmark>());
    dst.emplace_back(std::make_shared<cs_volume_benchmark>());
#if !defined(TRROJAN_FOR_UWP)
    dst.emplace_back(std::make_shared<two_pass_volume_benchmark>());
#endif /*!defined(TRROJAN_FOR_UWP) */

    retval = dst.size() - retval;
    return retval;
}


/*
 * trrojan::d3d11::plugin::create_environments
 */
size_t trrojan::d3d11::plugin::create_environments(
        environment_list& dst) const {
    //return 0;
    dst.emplace_back(std::make_shared<environment>());
    return 1;
}
