/// <copyright file="process.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/process.h"

#include <cstdio>
#include <cstdlib>
#include <system_error>
#include <vector>


/*
 * trrojan::get_module_file_name
 */
std::string TRROJANCORE_API trrojan::get_module_file_name(void) {
    typedef decltype(get_module_file_name()) string_type;
#ifdef _WIN32
    std::vector<string_type::value_type> retval;
    DWORD error = ERROR_SUCCESS;
    DWORD bufLen = MAX_PATH + 1;
    DWORD strLen = 0;

    do {
        retval.resize(bufLen);
        strLen = ::GetModuleFileNameA(NULL, retval.data(), bufLen);
        error = ::GetLastError();
        if (strLen == 0) {
            std::error_code ec(::GetLastError(), std::system_category());
            throw std::system_error(ec, "GetModuleFileName failed.");
        }
        bufLen *= 2;
    } while (error == ERROR_INSUFFICIENT_BUFFER);

    return std::move(string_type(retval.data(), strLen));

#else /* _WIN32 */
    string_type::value_type *arg = nullptr;
    string_type retval;
    size_t size = 0;

    auto fp = ::fopen("/proc/self/cmdline", "rb");
    if (fp != nullptr) {
        while (::getdelim(&arg, &size, 0, fp) != -1) {
            retval.append(arg, size);
            retval.append(" ");
        }
        ::free(arg);
        ::fclose(fp);
    }

    return std::move(retval);
#endif /* _WIN32 */
}


/*
 * trrojan::get_process_id
 */
trrojan::process_id TRROJANCORE_API trrojan::get_process_id(void) {
#ifdef _WIN32
    return ::GetCurrentProcessId();
#else /* _WIN32 */
    return ::getpid();
#endif /* _WIN32 */
}
