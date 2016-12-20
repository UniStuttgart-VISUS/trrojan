/// <copyright file="system_factors.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/system_factors.h"

#include "trrojan/smbios_information.h"


#include <iostream>
#include <iterator>


void trrojan::system_factors::crowbar(void) {
#ifdef _WIN32
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
#endif
}
