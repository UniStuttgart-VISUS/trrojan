/// <copyright file="system_factors.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/system_factors.h"

#include <cinttypes>
#include <fstream>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <sstream>
#include <string>
#include <system_error>
#include <thread>

#ifdef _WIN32
#include <Windows.h>
#else /* _WIN32 */
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#endif /* _WIN32 */

#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/text.h"


/*
 * trrojan::system_factors::factor_bios
 */
const std::string trrojan::system_factors::factor_bios("bios");


/*
 * trrojan::system_factors::factor_cpu
 */
const std::string trrojan::system_factors::factor_cpu("cpu");


/*
 * trrojan::system_factors::factor_installed_memory
 */
const std::string trrojan::system_factors::factor_installed_memory(
    "installed_memory");


/*
 * trrojan::system_factors::factor_logical_cores
 */
const std::string trrojan::system_factors::factor_logical_cores(
    "logical_cores");


/*
 * trrojan::system_factors::factor_mainboard
 */
const std::string trrojan::system_factors::factor_mainboard("mainboard");


/*
 * trrojan::system_factors::factor_os
 */
const std::string trrojan::system_factors::factor_os("os");


/*
 * trrojan::system_factors::factor_os_version
 */
const std::string trrojan::system_factors::factor_os_version("os_version");


/*
 * trrojan::system_factors::factor_ram
 */
const std::string trrojan::system_factors::factor_ram("ram");


/*
 * trrojan::system_factors::factor_system_desc
 */
const std::string trrojan::system_factors::factor_system_desc("system_desc");


/*
 * trrojan::system_factors::bios
 */
trrojan::variant trrojan::system_factors::bios(void) const {
    typedef smbios_information::bios_information_type entry_type;
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
            << e->get_release_date() << ")" << std::ends;
        return variant(value.str());
    }
}


/*
 * trrojan::system_factors::cpu
 */
trrojan::variant trrojan::system_factors::cpu(void) const {
    typedef smbios_information::processor_information_type entry_type;
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
            value << std::ends;
            
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
        value << std::ends;

        return variant(value.str());
    }

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
    if (factor == system_factors::factor_bios) {
        return system_factors::bios();

    } else if (factor == system_factors::factor_cpu) {
        return system_factors::cpu();

    } else if (factor == system_factors::factor_installed_memory) {
        return system_factors::installed_memory();

    } else if (factor == system_factors::factor_mainboard) {
        return system_factors::mainboard();

    } else if (factor == system_factors::factor_os) {
        return system_factors::os();

    } else if (factor == system_factors::factor_os_version) {
        return system_factors::os_version();

    } else if (factor == system_factors::factor_logical_cores) {
        return system_factors::logical_cores();

    } else if (factor == system_factors::factor_ram) {
        return system_factors::ram();

    } else if (factor == system_factors::factor_system_desc) {
        return system_factors::system_desc();

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
            "of installed physical memory with error code 0x%x.", err);
    }

#else /* _WIN32 */
    struct sysinfo info;

    if (sysinfo(&info) == 0) {
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
            "of installed physical memory with error code 0x%x.", err);
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
    typedef smbios_information::baseboard_information_type entry_type;
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
            << e->get_version() << ")" << std::ends;
        return variant(value.str());
    }
}


/*
 * trrojan::system_factors::os
 */
trrojan::variant trrojan::system_factors::os(void) const {
#ifdef _WIN32
    OSVERSIONINFOEXA vi;
    ::ZeroMemory(&vi, sizeof(vi));
    vi.dwOSVersionInfoSize = sizeof(vi);

#pragma warning(push)
#pragma warning(disable: 4996)
    if (!::GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&vi))) {
        std::error_code ec(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to get operating system version.");
    }
#pragma warning(pop)

    auto isWorkstation = (vi.wProductType == VER_NT_WORKSTATION);

    // https://msdn.microsoft.com/de-de/library/windows/desktop/ms724833(v=vs.85).aspx
    if ((vi.dwMajorVersion == 5) && (vi.dwMinorVersion == 0)) {
        return std::string("Windows 2000");

    } else if ((vi.dwMajorVersion == 5) && (vi.dwMinorVersion == 1)) {
        return std::string("Windows XP");

    } else if ((vi.dwMajorVersion == 5) && (vi.dwMinorVersion == 2)) {
        auto server2 = ::GetSystemMetrics(SM_SERVERR2);

        if (isWorkstation) {
            return std::string("Windows XP");

        } else if (server2 == 0) {
            return std::string("Windows Server 2003");

        } else if (vi.wSuiteMask & VER_SUITE_WH_SERVER) {
            return std::string("Windows Home Server");

        } else if (server2 != 0) {
            return std::string("Windows Server 2003 R2");

        } else {
            // Should be unreachable
            return std::string("Windows 5.2");
        }

    } else if ((vi.dwMajorVersion == 6) && (vi.dwMinorVersion == 0)) {
        return isWorkstation
                ? std::string("Windows Vista")
                : std::string("Windows Server 2008");

    } else if ((vi.dwMajorVersion == 6) && (vi.dwMinorVersion == 1)) {
        return isWorkstation
            ? std::string("Windows 7")
            : std::string("Windows Server 2008 R2");

    } else if ((vi.dwMajorVersion == 6) && (vi.dwMinorVersion == 2)) {
        return isWorkstation
            ? std::string("Windows 8")
            : std::string("Windows Server 2012");

    } else if ((vi.dwMajorVersion == 6) && (vi.dwMinorVersion == 3)) {
        return isWorkstation
            ? std::string("Windows 8.1")
            : std::string("Windows Server 2012 R2");

    } else if ((vi.dwMajorVersion == 10) && (vi.dwMinorVersion == 0)) {
        return isWorkstation
            ? std::string("Windows 10")
            : std::string("Windows Server 2016");

    } else {
        return isWorkstation
            ? std::string("Windows")
            : std::string("Windows Server");
    }

#else /* _WIN32 */
    utsname vi;

    // http://stackoverflow.com/questions/6315666/c-get-linux-distribution-name-version
    if (::uname(&vi) < 0) {
        std::error_code ec(errno, std::system_category());
        throw std::system_error(ec, "Failed to get operating system version.");
    }

    return std::string(vi.sysname);
#endif /* _WIN32 */
}


/*
 * trrojan::system_factors::os_version
 */
trrojan::variant trrojan::system_factors::os_version(void) const {
#ifdef _WIN32
    OSVERSIONINFOEXA vi;
    ::ZeroMemory(&vi, sizeof(vi));
    vi.dwOSVersionInfoSize = sizeof(vi);

#pragma warning(push)
#pragma warning(disable: 4996)
    if (!::GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&vi))) {
        std::error_code ec(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to get operating system version.");
    }
#pragma warning(pop)

    std::stringstream str;
    str << vi.dwMajorVersion << "."
        << vi.dwMinorVersion << "."
        << vi.dwBuildNumber;

    return str.str();

#else /* _WIN32 */
    utsname vi;

    // http://stackoverflow.com/questions/6315666/c-get-linux-distribution-name-version
    if (::uname(&vi) < 0) {
        std::error_code ec(errno, std::system_category());
        throw std::system_error(ec, "Failed to get operating system version.");
    }

    return std::string(vi.release);
#endif /* _WIN32 */
}


/*
 * trrojan::system_factors::ram
 */
trrojan::variant trrojan::system_factors::ram(void) const {
    typedef smbios_information::memory_device_type entry_type;
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
        value << std::ends;

        return variant(value.str());
    }
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
    typedef smbios_information::system_information_type entry_type;
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
            << e->get_version() << ")" << std::ends;
        return variant(value.str());
    }
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
 * trrojan::system_factors::system_factors
 */
trrojan::system_factors::system_factors(void) {
    try {
        this->smbios = smbios_information::read();
    } catch (std::exception ex) {
        log::instance().write(log_level::warning, ex);
    }
}
