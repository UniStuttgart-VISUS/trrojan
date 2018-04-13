/// <copyright file="os_info_impl.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "os_info_impl.h"

#include <cassert>
#include <limits>
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

#include "trrojan/sysinfo/os_info.h"

#include "utilities.h"


namespace trrojan {
namespace sysinfo {
namespace detail {

#if defined(_WIN32)
    /// <summary>
    /// Reads a DWORD value from the TDR registry path.
    /// </summary>
    template<class T> T get_tdr_registry_value(const char *name,
            const T default, const T error) {
        assert(name != nullptr);

        HKEY hKey = reinterpret_cast<HKEY>(INVALID_HANDLE_VALUE);
        auto retval = error;

        auto status = ::RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "System\\CurrentControlSet\\Control\\GraphicsDrivers", 0,
            KEY_READ, &hKey);
        if (status == ERROR_SUCCESS) {
            DWORD type = 0;
            std::vector<std::uint8_t> value;

            status = detail::read_reg_value(value, type, hKey, name);
            if (status == ERROR_SUCCESS) {
                // The key was found, convert the data.
                if ((type == REG_DWORD) && (value.size() == sizeof(DWORD))) {
                    auto v = *reinterpret_cast<DWORD *>(value.data());
                    retval = static_cast<T>(v);
                }
            } else if (status == ERROR_FILE_NOT_FOUND) {
                // The key was not found, but everything else succeeded, which
                // indicates default behaviour.
                retval = default;
            }
        }

        ::RegCloseKey(hKey);

        return retval;
    }
#endif /* defined(_WIN32) */

} /* end namespace detail */
} /* end namespace trrojan */
} /* end namespace sysinfo */


/*
 * trrojan::sysinfo::detail::os_info_impl::os_info_impl
 */
trrojan::sysinfo::detail::os_info_impl::os_info_impl(void)
    : tdr_ddi_delay(os_info::invalid_tdr_value),
    tdr_debug_mode(sysinfo::tdr_debug_mode::unknown),
    tdr_delay(os_info::invalid_tdr_value),
    tdr_level(sysinfo::tdr_level::unknown),
    tdr_limit_count(os_info::invalid_tdr_value),
    tdr_limit_time(os_info::invalid_tdr_value),
    word_size(0) { }


/*
 * trrojan::sysinfo::detail::os_info_impl::update
 */
void trrojan::sysinfo::detail::os_info_impl::update(void) {
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
            this->version = str.str();
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

        this->name = (vi.wProductType == VER_NT_WORKSTATION)
            ? "Windows"
            : "Windows Server";

        if (this->version.empty()) {
            std::stringstream str;
            str << vi.dwMajorVersion << "."
                << vi.dwMinorVersion << "."
                << vi.dwBuildNumber;
            this->version = str.str();
        }
    }

    // Determine the word size of the OS from the processor architecture.
    {
        SYSTEM_INFO si;
        ::GetNativeSystemInfo(&si);

        switch (si.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_INTEL:
                this->word_size = 32;
                break;

            case PROCESSOR_ARCHITECTURE_IA64:
                /* Falls through. */
            case PROCESSOR_ARCHITECTURE_AMD64:
                this->word_size = 64;
                break;

            case PROCESSOR_ARCHITECTURE_UNKNOWN:
                /* Falls through. */
            default:
                throw std::runtime_error("Failed to determine operating word "
                    "size.");
        }
    }

    // Determine the TDR information.
    this->tdr_ddi_delay = detail::get_tdr_registry_value("TdrDdiDelay",
        static_cast<size_t>(5), os_info::invalid_tdr_value);
    this->tdr_debug_mode = detail::get_tdr_registry_value("TdrDebugMode",
        sysinfo::tdr_debug_mode::default_behaviour,
        sysinfo::tdr_debug_mode::unknown);
    this->tdr_delay = detail::get_tdr_registry_value("TdrDelay",
        static_cast<size_t>(2), os_info::invalid_tdr_value);
    this->tdr_level = detail::get_tdr_registry_value("TdrLevel",
        sysinfo::tdr_level::default_behaviour, sysinfo::tdr_level::unknown);
    this->tdr_limit_count = detail::get_tdr_registry_value("TdrLimitCount",
        static_cast<size_t>(5), os_info::invalid_tdr_value);
    this->tdr_limit_time = detail::get_tdr_registry_value("TdrLimitTime",
        static_cast<size_t>(60), os_info::invalid_tdr_value);

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
}
