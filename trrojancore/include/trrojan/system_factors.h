/// <copyright file="system_factors.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph M�ller</author>

#pragma once

#include <string>

#include "trrojan/export.h"
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
        /// Name of the built-in factor describing the memory hardware.
        /// </summary>
        static const std::string factor_ram;

        /// <summary>
        /// Name of the built-in factor describing the system (if the OEM
        /// set the information in SMBIOS).
        /// </summary>
        static const std::string factor_system_desc;

        variant bios(void) const;

        variant cpu(void) const;

        variant get(const std::string& factor) const;

        variant installed_memory(void) const;

        variant mainboard(void) const;

        variant logical_cores(void) const;

        variant ram(void) const;

        variant system_desc(void) const;

    private:

        system_factors(void);

        system_factors(const system_factors&) = delete;

        system_factors& operator =(const system_factors&) = delete;

        smbios_information smbios;

    };
}