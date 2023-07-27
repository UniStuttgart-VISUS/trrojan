/// <copyright file="system_factors.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/system_factors.h"

#include <cerrno>
#include <cinttypes>
#include <ctime>
#include <fstream>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>

#ifdef _UWP
#include <winrt/windows.security.exchangeactivesyncprovisioning.h>
#include <winrt/windows.system.profile.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.system.h>
#include <Windows.h>
#if defined(NTDDI_WIN10_RS3) && (NTDDI_VERSION >= NTDDI_WIN10_RS3)
#include "Gamingdeviceinformation.h"
#endif
#endif
#ifdef _WIN32
#include <Windows.h>
#include <lmcons.h>
#else /* _WIN32 */
#include <pwd.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#endif /* _WIN32 */

#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/text.h"


/// <summary>
/// Enables retrieval of a system factor by its name.
/// </summary>
/// <remarks>
/// This must be a static in the .cpp in order to ensure that it is initialised
/// before <see cref="trrojan::system_factors" />.
/// </remarks>
static std::unordered_map<std::string, trrojan::system_factors::retriever_type>
retrievers;


/// <summary>
/// Registers a system factor for retrieval by name.
/// </summary>
static const std::string& register_retriever(const std::string& name,
        trrojan::system_factors::retriever_type retriever) {
    ::retrievers[name] = retriever;
    return name;
}


/*
 * trrojan::system_factors::is_system_factor
 */
bool trrojan::system_factors::is_system_factor(const std::string& factor) {
    for (auto& r : ::retrievers) {
        if (r.first == factor) {
            return true;
        }
    }
    /* No match found at this point. */

    return false;
}


#define __TRROJAN_DEFINE_FACTOR(n)\
const std::string trrojan::system_factors::factor_##n                          \
= ::register_retriever(#n, &system_factors::n)

__TRROJAN_DEFINE_FACTOR(bios);
__TRROJAN_DEFINE_FACTOR(computer_name);
__TRROJAN_DEFINE_FACTOR(cpu);
__TRROJAN_DEFINE_FACTOR(installed_memory);
__TRROJAN_DEFINE_FACTOR(logical_cores);
__TRROJAN_DEFINE_FACTOR(mainboard);
__TRROJAN_DEFINE_FACTOR(os);
__TRROJAN_DEFINE_FACTOR(os_version);
__TRROJAN_DEFINE_FACTOR(process_elevated);
__TRROJAN_DEFINE_FACTOR(ram);
__TRROJAN_DEFINE_FACTOR(system_desc);
__TRROJAN_DEFINE_FACTOR(tdr_delay);
__TRROJAN_DEFINE_FACTOR(tdr_level);
__TRROJAN_DEFINE_FACTOR(timestamp);
__TRROJAN_DEFINE_FACTOR(user_name);

#undef __TRROJAN_DEFINE_FACTOR


/*
 * trrojan::system_factors::bios
 */
trrojan::variant trrojan::system_factors::bios(void) const {
#ifndef _UWP
    typedef sysinfo::smbios_information::bios_information_type entry_type;
    std::vector<const entry_type *> entries;
    smbios.entries_by_type<entry_type>(std::back_inserter(entries));

    if (entries.empty()) {
        log::instance().write(log_level::warning, "No information about the "
            "BIOS could be retrieved from SMBIOS.");
        return variant();

    } else {
        auto e = entries.front();
        std::stringstream value;
        value << e->get_vendor() << " "
            << e->get_version() << " ("
            << e->get_release_date() << ")";
        return variant(value.str());
    }
#else
    log::instance().write(log_level::warning, "No information about the "
        "BIOS could be retrieved from SMBIOS.");
    return variant();
#endif
}


/*
* trrojan::system_factors::computer_name
*/
trrojan::variant trrojan::system_factors::computer_name(void) const {
#ifdef _UWP
    winrt::Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation easinfo;
    return winrt::to_string(easinfo.FriendlyName());
#elif defined (_WIN32)

    std::vector<char> buffer;
    buffer.resize(UNLEN + 1);
    auto oldBufSize = static_cast<DWORD>(buffer.size());
    auto bufSize = oldBufSize;

    while (!::GetComputerNameA(buffer.data(), &bufSize)) {
        DWORD le = ::GetLastError();
        if ((le == ERROR_INSUFFICIENT_BUFFER) && (oldBufSize != bufSize)) {
            oldBufSize = bufSize;
            buffer.resize(bufSize);

        } else {
            std::error_code ec(::GetLastError(), std::system_category());
            throw std::system_error(ec, "Failed to get computer name.");
        }
    }

    return std::string(buffer.data());
#else /* _WIN32 */
    utsname names;

    if (::uname(&names) < 0) {
        std::error_code ec(errno, std::system_category());
        throw std::system_error(ec, "Failed to get computer name.");
    }

    return std::string(names.nodename);

#endif /* _UWP _WIN32 */
}


/*
 * trrojan::system_factors::cpu
 */
trrojan::variant trrojan::system_factors::cpu(void) const {
#ifdef  _UWP
    SYSTEM_INFO info = {};
    GetNativeSystemInfo(&info);

    std::string arch = "UNKNOWN";
    switch (info.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:  arch = "AMD64"; break;
    case PROCESSOR_ARCHITECTURE_ARM:    arch = "ARM"; break;
    case PROCESSOR_ARCHITECTURE_ARM64:  arch = "ARM64"; break;
    case PROCESSOR_ARCHITECTURE_INTEL:  arch = "INTEL"; break;
    }

    return arch;
#else
    typedef sysinfo::smbios_information::processor_information_type entry_type;
    std::vector<const entry_type *> entries;
    smbios.entries_by_type<entry_type>(std::back_inserter(entries));

    if (entries.empty()) {
#ifndef _WIN32
        /* Try /proc/cpuinfo as fallback (eg if not running as root). */
        try {
            std::ifstream pf("/proc/cpuinfo", std::ios::in);
            bool isFirst = true;
            std::string line;
            std::stringstream value;

            while (std::getline(pf, line)) {
                // TODO: filter sockets using physical id
#if (defined(__GNUC__) && ((__GNUC__ > 4)\
    || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 9))))
                static const std::regex RXM("model\\s+name\\s*:\\s*([^\\n]+)");
                //static const std::regex RXP("physical\\s+id\\s*:\\s*([0-9]+)");
                std::smatch match;
                if (std::regex_match(line, match, RXM)) {
                    if (isFirst) {
                        isFirst = false;
                    } else {
                        value << ", ";
                    }
                    value << trim(match[1].str());
                }
#else /* (defined(__GNUC__) && ((__GNUC__ > 4) ... */
                auto kp = line.find("model name");
                auto sp = line.find(": ");
                if ((kp != std::string::npos) && (sp != std::string::npos)) {
                    if (isFirst) {
                        isFirst = false;
                    } else {
                        value << ", ";
                    }
                    value << trim(line.replace(0, sp + 1, ""));
                }
#endif /* (defined(__GNUC__) && ((__GNUC__ > 4) ... */
            }

            if (value.tellp() > 0) {
                return variant(value.str());
            }
        } catch (const std::exception& ex) {
            log::instance().write_line(log_level::verbose, ex);
            /* Fall through to error handling. */
        }
#endif /* !_WIN32 */

        log::instance().write(log_level::warning, "No information about the "
            "installed CPUs could be retrieved from SMBIOS.");
        return variant();

    } else {
        bool isFirst = true;
        std::stringstream value;

        for (auto e : entries) {
            if (isFirst) {
                isFirst = false;
            } else {
                value << ", ";
            }
            value << e->get_socket_designation() << " = "
                << e->get_manufacturer() << " "
                << e->get_version();
        }

        return variant(value.str());
    }
#endif

    //{
    //    typedef smbios_information::processor_information_type entry_type;
    //    std::vector<const smbios_information::header_type *> entries;
    //    smbios.entries_by_type<entry_type>(std::back_inserter(entries));
    //    for (auto h : entries) {
    //        auto e = reinterpret_cast<const entry_type *>(h);
    //        std::cout << e->get_manufacturer() << std::endl;
    //        std::cout << e->get_version() << std::endl;
    //        std::cout << e->get_socket_designation() << std::endl;
    //        std::cout << ((e->core_count == 0xFF)
    //            ? e->core_count2 : e->core_count) << std::endl;
    //        std::cout << ((e->thread_count == 0xFF)
    //            ? e->thread_count2 : e->thread_count) << std::endl;
    //        std::cout << static_cast<double>(e->current_speed) << std::endl;
    //    }
    //}
}


/*
 * trrojan::system_factors::get
 */
trrojan::variant trrojan::system_factors::get(const std::string& factor) const {
    auto it = ::retrievers.find(factor);
    if (it != ::retrievers.end()) {
        return (this->*(it->second))();
    } else {
        return variant();
    }
}


/*
 * trrojan::system_factors::installed_memory
 */
trrojan::variant trrojan::system_factors::installed_memory(void) const {
    typedef std::uint64_t memory_size_type;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    ::ZeroMemory(&memoryStatus, sizeof(memoryStatus));
    memoryStatus.dwLength = sizeof(memoryStatus);

    if (::GlobalMemoryStatusEx(&memoryStatus)) {
        return static_cast<memory_size_type>(memoryStatus.ullTotalPhys);

    } else {
        auto err = ::GetLastError();
        log::instance().write(log_level::warning, "Failed to retrieve amount "
            "of installed physical memory with error code {:x}.", err);
    }

#else /* _WIN32 */
    struct sysinfo info;

    if (::sysinfo(&info) == 0) {
        if (sizeof(info._f) != (sizeof(char) * (20 - 2 * sizeof(long)
            - sizeof(int)))) {
            /* a fucking old kernel is used */
            return static_cast<memory_size_type>(info.freeram);
        } else {
            return static_cast<memory_size_type>(info.freeram)
                * static_cast<memory_size_type>(info.mem_unit);
        }

    } else {
        auto err = errno;
        log::instance().write(log_level::warning, "Failed to retrieve amount "
            "of installed physical memory with error code {:x}.", err);
    }

#endif /* _WIN32 */

    return variant();   // At this point, an error occurred ...
}


/*
 * trrojan::system_factors::logical_cores
 */
trrojan::variant trrojan::system_factors::logical_cores(void) const {
    return static_cast<std::uint32_t>(std::thread::hardware_concurrency());
}


/*
 * trrojan::system_factors::mainboard
 */
trrojan::variant trrojan::system_factors::mainboard(void) const {
#ifndef _UWP
    typedef sysinfo::smbios_information::baseboard_information_type entry_type;
    std::vector<const entry_type *> entries;
    smbios.entries_by_type<entry_type>(std::back_inserter(entries));

    if (entries.empty()) {
        log::instance().write(log_level::warning, "No information about the "
            "mainboard could be retrieved from SMBIOS.");
        return variant();

    } else {
        auto e = entries.front();
        std::stringstream value;
        value << e->get_manufacturer() << " "
            << e->get_product_name() << " ("
            << e->get_version() << ")";
        return variant(value.str());
    }
#else
    log::instance().write(log_level::warning, "No information about the "
        "mainboard could be retrieved from SMBIOS.");
    return variant();
#endif
}


/*
 * trrojan::system_factors::os
 */
trrojan::variant trrojan::system_factors::os(void) const {
#ifdef _UWP
    auto versionInfo = winrt::Windows::System::Profile::AnalyticsInfo::VersionInfo();
    return winrt::to_string(versionInfo.DeviceFamily());
#else
    return std::string(this->osinfo.name());
#endif
}


/*
 * trrojan::system_factors::os_version
 */
trrojan::variant trrojan::system_factors::os_version(void) const {
#ifdef _UWP
    auto versionInfo = winrt::Windows::System::Profile::AnalyticsInfo::VersionInfo();

    // From SystemInfo UWP example:
    // For real-world use just log it as an opaque string, and do the decode in the reader instead
    LARGE_INTEGER li;
    li.QuadPart = _wtoi64(versionInfo.DeviceFamilyVersion().c_str());
    wchar_t buff[128] = {};
    swprintf_s(buff, L"%u.%u.%u.%u", HIWORD(li.HighPart), LOWORD(li.HighPart), HIWORD(li.LowPart), LOWORD(li.LowPart));

    return winrt::to_string(buff);
#else
    return std::string(this->osinfo.version());
#endif
}


/*
 * trrojan::system_factors::process_elevated
 */
trrojan::variant trrojan::system_factors::process_elevated(void) const {
#ifdef _WIN32
    HANDLE hToken = NULL;
    DWORD size = 0;
    TOKEN_ELEVATION_TYPE tet;

    if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        std::error_code ec(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to get process token.");
    }

    if (::GetTokenInformation(hToken, TokenElevationType, &tet,
            sizeof(tet), &size)) {
        assert(size == sizeof(tet));
        return (tet == TokenElevationTypeFull);

    } else {
        std::error_code ec(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to obtain token information.");
    }

#else  /* _WIN32*/
    auto uid = ::getuid();
    auto euid = ::geteuid();

    // TODO
    // http://stackoverflow.com/questions/4159910/check-if-user-is-root-in-c:
    //if ((uid < 0) || (uid != euid)) {
    //    // We might have elevated privileges beyond that of the user who invoked
    //    // the program, due to suid bit. Be very careful about trusting any
    //    // data!
    //    return true;

    //} else {
    //    /* Anything goes. */
    //}

    return (euid == 0);
#endif /* _WIN32 */
}


/*
 * trrojan::system_factors::ram
 */
trrojan::variant trrojan::system_factors::ram(void) const {
#ifdef _UWP
    log::instance().write(log_level::warning, "No information about the "
        "ram version could be retrieved from SMBIOS.");
    return variant();
#else
    typedef sysinfo::smbios_information::memory_device_type entry_type;
    std::vector<const entry_type *> entries;
    smbios.entries_by_type<entry_type>(std::back_inserter(entries));

    if (entries.empty()) {
        log::instance().write(log_level::warning, "No information about the "
            "installed RAM modules could be retrieved from SMBIOS.");
        return variant();

    } else {
        bool isFirst = true;
        std::stringstream value;

        for (auto e : entries) {
            if (e->size != 0) {
                if (isFirst) {
                    isFirst = false;
                } else {
                    value << ", ";
                }
                value << e->get_bank_locator() << " = "
                    << e->get_manufacturer() << " "
                    << e->get_part_number();
            }
        }

        return variant(value.str());
    }
#endif
    //for (auto h : entries) {
    //    if (e->size != 0) {
    //        std::cout << e->get_manufacturer() << std::endl;
    //        std::cout << e->get_part_number() << std::endl;
    //        std::cout << smbios_information::decode_memory_device_type(e->type) << std::endl;
    //        std::cout << smbios_information
    //            ::decode_memory_device_type_detail(e->type_detail) << std::endl;
    //        std::cout << smbios_information
    //            ::decode_memory_device_form_factor(e->form_factor) << std::endl;
    //        std::cout << e->get_device_locator() << std::endl;
    //        std::cout << e->get_bank_locator() << std::endl;
    //        std::cout << ((e->size == 0x7FFF)
    //            ? e->extended_size : e->size) << std::endl;
    //        std::cout << static_cast<double>(e->speed) << std::endl;
    //    }
    //}
}


/*
 * trrojan::system_factors::system_desc
 */
trrojan::variant trrojan::system_factors::system_desc(void) const {
#ifdef _UWP
    winrt::Windows::Security::ExchangeActiveSyncProvisioning::EasClientDeviceInformation easinfo;
    return winrt::to_string(easinfo.SystemProductName());
#else
    typedef sysinfo::smbios_information::system_information_type entry_type;
    std::vector<const entry_type *> entries;
    smbios.entries_by_type<entry_type>(std::back_inserter(entries));

    if (entries.empty()) {
        log::instance().write(log_level::warning, "No information about the "
            "system could be retrieved from SMBIOS.");
        return variant();

    } else {
        auto e = entries.front();
        std::stringstream value;
        value << e->get_manufacturer() << " "
            << e->get_product_name() << " ("
            << e->get_version() << ")";
        return variant(value.str());
    }
#endif
}


/*
 * trrojan::system_factors::timestamp
 */
trrojan::variant trrojan::system_factors::timestamp(void) const {
    return to_string<char>(std::chrono::system_clock::now());
}


/*
 * trrojan::system_factors::user_name
 */
trrojan::variant trrojan::system_factors::user_name(void) const {
#ifdef _WIN32
#ifdef _UWP
    // Fairly hacky solution. Needs App permission set in OS to work

    GAMING_DEVICE_MODEL_INFORMATION info = {};
    GetGamingDeviceModelInformation(&info);
    switch (info.deviceId)
    {
        #ifndef NTDDI_WIN10_NI
        #pragma warning(disable : 4063)
        #define GAMING_DEVICE_DEVICE_ID_XBOX_SERIES_S static_cast<GAMING_DEVICE_DEVICE_ID>(0x1D27FABB)
        #define GAMING_DEVICE_DEVICE_ID_XBOX_SERIES_X static_cast<GAMING_DEVICE_DEVICE_ID>(0x2F7A3DFF)
        #define GAMING_DEVICE_DEVICE_ID_XBOX_SERIES_X_DEVKIT static_cast<GAMING_DEVICE_DEVICE_ID>(0xDE8A5661)
        #endif
        case GAMING_DEVICE_DEVICE_ID_XBOX_SERIES_S: return winrt::to_string(L"unavailable on xbox");
        case GAMING_DEVICE_DEVICE_ID_XBOX_SERIES_X: return winrt::to_string(L"unavailable on xbox");
    }

    /*winrt::Windows::Foundation::Collections::IVectorView<winrt::Windows::System::User> users 
        = winrt::Windows::System::User::FindAllAsync(winrt::Windows::System::UserType::LocalUser, winrt::Windows::System::UserAuthenticationStatus::LocallyAuthenticated).get();
    winrt::Windows::System::User currentUser = users.GetAt(0);
    winrt::Windows::Foundation::IInspectable nameObj = currentUser.GetPropertyAsync(winrt::Windows::System::KnownUserProperties::DisplayName()).get();
    winrt::hstring myname = winrt::unbox_value<winrt::hstring>(nameObj);*/
    return winrt::to_string(L"displayName");
#else
    std::vector<char> buffer;
    buffer.resize(UNLEN + 1);
    auto oldBufSize = static_cast<DWORD>(buffer.size());
    auto bufSize = oldBufSize;

    while (!::GetUserNameA(buffer.data(), &bufSize)) {
        DWORD le = ::GetLastError();
        if ((le == ERROR_INSUFFICIENT_BUFFER) && (oldBufSize != bufSize)) {
            oldBufSize = bufSize;
            buffer.resize(bufSize);

        } else {
            std::error_code ec(::GetLastError(), std::system_category());
            throw std::system_error(ec, "Failed to get user name.");
        }
    }

    return std::string(buffer.data());
#endif
#else /* _WIN32 */
    uid_t uid = ::getuid();
    auto passwd = ::getpwuid(uid);

    if (passwd == nullptr) {
        std::error_code ec(errno, std::system_category());
        throw std::system_error(ec, "Failed to get user name.");
    }

    return std::string(passwd->pw_name);

#endif /* _WIN32 */
}


#if 0
void trrojan::system_factors::crowbar(void) {
    auto smbios = smbios_information::read();

    {
        typedef smbios_information::bios_information_type entry_type;
        std::vector<const smbios_information::header_type *> entries;
        smbios.entries_by_type<entry_type>(std::back_inserter(entries));
        if (!entries.empty()) {
            auto e = reinterpret_cast<const entry_type *>(entries.front());
            std::cout << e->get_vendor() << std::endl;
            std::cout << e->get_version() << std::endl;
            std::cout << e->get_release_date() << std::endl;
        }
    }

    {
        typedef smbios_information::processor_information_type entry_type;
        std::vector<const smbios_information::header_type *> entries;
        smbios.entries_by_type<entry_type>(std::back_inserter(entries));
        for (auto h : entries) {
            auto e = reinterpret_cast<const entry_type *>(h);
            std::cout << e->get_manufacturer() << std::endl;
            std::cout << e->get_version() << std::endl;
            std::cout << e->get_socket_designation() << std::endl;
            std::cout << ((e->core_count == 0xFF)
                ? e->core_count2 : e->core_count) << std::endl;
            std::cout << ((e->thread_count == 0xFF)
                ? e->thread_count2 : e->thread_count) << std::endl;
            std::cout << static_cast<double>(e->current_speed) << std::endl;
        }
    }

    // TODO: mainboard
    {
        typedef smbios_information::baseboard_information_type entry_type;
        std::vector<const smbios_information::header_type *> entries;
        smbios.entries_by_type<entry_type>(std::back_inserter(entries));
        if (!entries.empty()) {
            auto e = reinterpret_cast<const entry_type *>(entries.front());
            std::cout << e->get_manufacturer() << std::endl;
            std::cout << e->get_product_name() << std::endl;
            std::cout << e->get_version() << std::endl;
            //retval.mainboard.type = e->get_version();
        }
    }

    {
        typedef smbios_information::memory_device_type entry_type;
        std::vector<const smbios_information::header_type *> entries;
        smbios.entries_by_type<entry_type>(std::back_inserter(entries));
        for (auto h : entries) {
            auto e = reinterpret_cast<const entry_type *>(h);
            if (e->size != 0) {
                std::cout << e->get_manufacturer() << std::endl;
                std::cout << e->get_part_number() << std::endl;
                std::cout << smbios_information::decode_memory_device_type(e->type) << std::endl;
                std::cout << smbios_information
                    ::decode_memory_device_type_detail(e->type_detail) << std::endl;
                std::cout << smbios_information
                    ::decode_memory_device_form_factor(e->form_factor) << std::endl;
                std::cout << e->get_device_locator() << std::endl;
                std::cout << e->get_bank_locator() << std::endl;
                std::cout << ((e->size == 0x7FFF)
                    ? e->extended_size : e->size) << std::endl;
                std::cout << static_cast<double>(e->speed) << std::endl;
            }
        }
    }

    {
        typedef smbios_information::system_information_type entry_type;
        std::vector<const smbios_information::header_type *> entries;
        smbios.entries_by_type<entry_type>(std::back_inserter(entries));
        if (!entries.empty()) {
            auto e = reinterpret_cast<const entry_type *>(entries.front());
            std::cout << e->get_manufacturer() << std::endl;
            std::cout << e->get_product_name() << std::endl;
            std::cout << e->get_version() << std::endl;
        }
    }
}
#endif


/*
 * trrojan::system_factors::get_retrievers
 */
const std::unordered_map<std::string, trrojan::system_factors::retriever_type>&
trrojan::system_factors::get_retrievers(void) {
    return ::retrievers;
}


/*
 * trrojan::system_factors::system_factors
 */
trrojan::system_factors::system_factors(void) {
#ifndef _UWP
    try {
        this->hwinfo = sysinfo::hardware_info::collect();
    } catch (std::exception ex) {
        log::instance().write(log_level::warning, ex);
    }
    try {
        this->osinfo = sysinfo::os_info::collect();
    } catch (std::exception ex) {
        log::instance().write(log_level::warning, ex);
    }
    try {
        this->smbios = sysinfo::smbios_information::read();
    } catch (std::exception ex) {
        log::instance().write(log_level::warning, ex);
    }
#endif
}
