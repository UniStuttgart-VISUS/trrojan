/// <copyright file="system_factors.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <string>
#include <unordered_map>

#include "trrojan/export.h"
#include "trrojan/named_variant.h"
#include "trrojan/variant.h"
#include "trrojan/smbios_information.h"


namespace trrojan {

    /// <summary>
    /// This class provides factors defined by the system the programme is
    /// currently running on.
    /// </summary>
    class TRROJANCORE_API system_factors {

    public:

        /// <summary>
        /// Type of a pointer-to-member retrieving a system factor from an 
        /// instance of <see cref="trrojan::system_factors" />.
        /// </summary>
        typedef variant(trrojan::system_factors::*retriever_type)(void) const;

        /// <summary>
        /// Answer the only instance of this class.
        /// </summary>
        static const system_factors& instance(void) {
            static system_factors instance;
            return instance;
        }

        /// <summary>
        /// Name of the built-in factor describing the BIOS.
        /// </summary>
        static const std::string factor_bios;

        /// <summary>
        /// Name of the built-in factor describing the host name.
        /// </summary>
        static const std::string factor_computer_name;

        /// <summary>
        /// Name of the built-in factor describing the CPU(s).
        /// </summary>
        static const std::string factor_cpu;

        /// <summary>
        /// Name of the built-in factor descrbing the amount of RAM installed.
        /// </summary>
        static const std::string factor_installed_memory;

        /// <summary>
        /// Name of the built-in factor describing the logical CPU cores
        /// available on the system.
        /// </summary>
        static const std::string factor_logical_cores;

        /// <summary>
        /// Name of the built-in factor describing the mainboard.
        /// </summary>
        static const std::string factor_mainboard;

        /// <summary>
        /// Name of the built-in factor describing the operating system.
        /// </summary>
        static const std::string factor_os;

        /// <summary>
        /// Name of the built-in factor describing the operating system version.
        /// </summary>
        static const std::string factor_os_version;

        /// <summary>
        /// Name of the built-in factor describing whether the current process
        /// is elevated.
        /// </summary>
        static const std::string factor_process_elevated;

        /// <summary>
        /// Name of the built-in factor describing the memory hardware.
        /// </summary>
        static const std::string factor_ram;

        /// <summary>
        /// Name of the built-in factor describing the system (if the OEM
        /// set the information in SMBIOS).
        /// </summary>
        static const std::string factor_system_desc;

        /// <summary>
        /// Name of the built-in factor describing the user name.
        /// </summary>
        static const std::string factor_user_name;

        variant bios(void) const;

        variant computer_name(void) const;

        variant cpu(void) const;

        variant get(const std::string& factor) const;

        /// <summary>
        /// Answer all system factors.
        /// </summary>
        template<class I> inline void get(I oit) const {
            for (auto r : system_factors::get_retrievers()) {
                *oit++ = named_variant(r.first, (this->*(r.second))());
            }
        }

        variant installed_memory(void) const;

        variant mainboard(void) const;

        variant os(void) const;

        variant os_version(void) const;

        variant process_elevated(void) const;

        variant logical_cores(void) const;

        variant ram(void) const;

        variant system_desc(void) const;

        variant user_name(void) const;

    private:

        static const std::unordered_map<std::string, retriever_type>&
        get_retrievers(void);

        system_factors(void);

        system_factors(const system_factors&) = delete;

        system_factors& operator =(const system_factors&) = delete;

        smbios_information smbios;

    };
}
