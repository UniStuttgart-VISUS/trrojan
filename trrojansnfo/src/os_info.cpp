/// <copyright file="os_info.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/sysinfo/os_info.h"

#include <memory>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#include <lmcons.h>
#else /* _WIN32 */
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#endif /* _WIN32 */

#include "utilities.h"



/*
 * trrojan::sysinfo::os_info::collect
 */
trrojan::sysinfo::os_info trrojan::sysinfo::os_info::collect(void) {
    os_info retval;

#if defined(_WIN32)
    typedef NTSTATUS(WINAPI *GetVersionFunc)(PRTL_OSVERSIONINFOW);
    GetVersionFunc getVersion = nullptr;

    auto hModule = ::GetModuleHandleW(L"ntdll.dll");
    if (hModule) {
        getVersion = reinterpret_cast<GetVersionFunc>(::GetProcAddress(
            hModule, "RtlGetVersion"));
    }

    // First, try calling the runtime library directly, which will give us the
    // real OS version rather than the SDK version we use.
    if (getVersion != nullptr) {
        RTL_OSVERSIONINFOW vi = { 0 };
        ::ZeroMemory(&vi, sizeof(vi));
        vi.dwOSVersionInfoSize = sizeof(vi);

        if (getVersion(&vi) == 0) {
            std::stringstream str;
            str << vi.dwMajorVersion << "."
                << vi.dwMinorVersion << "."
                << vi.dwBuildNumber;
            retval._version = detail::clone_string(str.str().c_str());
        }
    }

    // Use public API (returns SDK version on Windows 10). Furthermore, the
    // public API tells us whether we are on a server or workstation.
    {
        OSVERSIONINFOEXA vi;
        ::ZeroMemory(&vi, sizeof(vi));
        vi.dwOSVersionInfoSize = sizeof(vi);

#pragma warning(push)
#pragma warning(disable: 4996)
        if (!::GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&vi))) {
            std::error_code ec(::GetLastError(), std::system_category());
            throw std::system_error(ec, "Failed to get operating system "
                "version.");
        }
#pragma warning(pop)

        retval._name = detail::clone_string(
            (vi.wProductType == VER_NT_WORKSTATION)
            ? "Windows"
            : "Windows Server");

        if (retval._version == nullptr) {
            std::stringstream str;
            str << vi.dwMajorVersion << "."
                << vi.dwMinorVersion << "."
                << vi.dwBuildNumber;
            retval._version = detail::clone_string(str.str().c_str());
        }
    }

    // Determine the word size of the OS from the processor architecture.
    {
        SYSTEM_INFO si;
        ::GetNativeSystemInfo(&si);

        switch (si.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_INTEL:
                retval._word_size = 32;
                break;

            case PROCESSOR_ARCHITECTURE_IA64:
                /* Falls through. */
            case PROCESSOR_ARCHITECTURE_AMD64:
                retval._word_size = 64;
                break;

            case PROCESSOR_ARCHITECTURE_UNKNOWN:
                /* Falls through. */
            default:
                throw std::runtime_error("Failed to determine operating word "
                    "size.");
        }
    }

#else /* defined(_WIN32) */
    utsname vi;

    // http://stackoverflow.com/questions/6315666/c-get-linux-distribution-name-version
    if (::uname(&vi) < 0) {
        std::error_code ec(errno, std::system_category());
        throw std::system_error(ec, "Failed to get operating system "
            "information.");
    }

    retval._name = detail::clone_string(vi.sysname);
    retval._version = detail::clone_string(vi.release);
    retval._word_size = (::strstr(vi.machine, "64") == nullptr) ? 32 : 64;

#endif /* defined(_WIN32) */

    return retval;
}


/*
 * trrojan::sysinfo::os_info::~os_info
 */
trrojan::sysinfo::os_info::~os_info(void) {
    delete[] this->_name;
    delete[] this->_version;
}


/*
 * trrojan::sysinfo::os_info::operator =
 */
trrojan::sysinfo::os_info& trrojan::sysinfo::os_info::operator =(
        const os_info& rhs) {
    if (this != std::addressof(rhs)) {
        this->_name = detail::clone_string(rhs._name);
        this->_version = detail::clone_string(rhs._version);
        this->_word_size = rhs._word_size;
    }

    return *this;
}


/*
* trrojan::sysinfo::os_info::operator =
 */
trrojan::sysinfo::os_info& trrojan::sysinfo::os_info::operator =(
        os_info&& rhs) {
    if (this != std::addressof(rhs)) {
        this->_name = rhs._name;
        rhs._name = nullptr;
        this->_version = rhs._version;
        rhs._version = nullptr;
        this->_word_size = rhs._word_size;
        rhs._word_size = 0;
    }

    return *this;
}
