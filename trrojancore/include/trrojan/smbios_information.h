/// <copyright file="smbios_information.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

/// <copyright file="smbios_information.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2015 Visualisierungsinstitut der Universität Stuttgart.
/// Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Provides means to read and process the system management BIOS (SMBIOS).
    /// </summary>
    class TRROJANCORE_API smbios_information {

    public:

        typedef uint8_t byte_type;
        typedef uint32_t dword_type;
        typedef uint16_t handle_type;
        typedef uint64_t qword_type;
        typedef uint8_t string_type;    // Represents the index of the string.
        typedef uint16_t word_type;

        // See http://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.0.0.pdf
#pragma pack(push)
#pragma pack(1)
        typedef struct {
            byte_type anchor[5];
            byte_type checksum;
            word_type table_length;
            dword_type table_address;
            word_type table_num_structs;
            byte_type bcd_revision;
        } dmi_entry_point_type;

        typedef struct {
            byte_type anchor[4];
            byte_type checksum;
            byte_type struct_size;
            byte_type major_version;
            byte_type minor_version;
            word_type max_struct_size;
            byte_type revision;
            byte_type formatted_area[5];
            byte_type intermediate_anchor[5];
            byte_type intermediate_checksum;
            word_type table_length;
            dword_type table_address;
            word_type table_num_structs;
            byte_type bcd_revision;
            byte_type _padding;
        } smbios_2_1_entry_point_type;

        typedef struct {
            byte_type anchor[5];
            byte_type checksum;
            byte_type struct_size;
            byte_type major_version;
            byte_type minor_version;
            byte_type docrev;
            byte_type revision;
            byte_type _reserved;
            dword_type max_struct_size;
            qword_type table_address;
        } smbios_3_0_entry_point_type;

        typedef struct {
            byte_type type;
            byte_type length;
            handle_type handle;
        } header_type;

        typedef struct {
            header_type header;
            string_type vendor;
            string_type version;
            word_type starting_address_segment;
            string_type release_date;
            byte_type rom_size;       // 64KB * (rom_size + 1) is actual size.
            /* SMBIOS 2.1+ */
            qword_type characteristics;
            /* SMBIOS 2.4+ */
            byte_type extension_bytes1;
            byte_type extension_bytes2;
            byte_type major_release;
            byte_type minor_release;
            byte_type firmware_major_release;
            byte_type firmware_minor_release;

            inline const char *get_vendor(void) const {
                return smbios_information::get_string(this, this->vendor);
            }

            inline const char *get_version(void) const {
                return smbios_information::get_string(this, this->version);
            }

            inline const char *get_release_date(void) const {
                return smbios_information::get_string(this, this->release_date);
            }
        } bios_information_type;

        typedef struct {
            header_type header;
            string_type manufacturer;
            string_type product_name;
            string_type version;
            string_type serial_number;
            /* SMBIOS 2.1+ */
            byte_type uuid[16];
            byte_type wake_up_type;
            /* SMBIOS 2.4+ */
            byte_type sku_number;
            byte_type family;

            inline const char *get_manufacturer(void) const {
                return smbios_information::get_string(this, this->manufacturer);
            }

            inline const char *get_product_name(void) const {
                return smbios_information::get_string(this, this->product_name);
            }

            inline const char *get_version(void) const {
                return smbios_information::get_string(this, this->version);
            }

            inline const char *get_serial_number(void) const {
                return smbios_information::get_string(this, this->serial_number);
            }
        } system_information_type;

        typedef struct {
            header_type header;
            string_type manufacturer;
            string_type product_name;
            string_type version;
            string_type serial_number;
            string_type asset_tag;
            byte_type feature_flags;
            string_type location_in_chassis;
            handle_type chassis_handle;
            byte_type board_type;
            byte_type number_of_contained_object_handles;
            handle_type contained_object_handles[255];

            inline const char *get_manufacturer(void) const {
                return smbios_information::get_string(this, this->manufacturer);
            }

            inline const char *get_product_name(void) const {
                return smbios_information::get_string(this, this->product_name);
            }

            inline const char *get_version(void) const {
                return smbios_information::get_string(this, this->version);
            }

            inline const char *get_serial_number(void) const {
                return smbios_information::get_string(this, this->serial_number);
            }

            inline const char *get_asset_tag(void) const {
                return smbios_information::get_string(this, this->asset_tag);
            }
        } baseboard_information_type;

        typedef struct {
            header_type header;
            string_type manufacturer;
            string_type type;
            string_type version;
            string_type serial_number;
            string_type asset_tag;
            /* SMBIOS 2.1+ */
            byte_type bootup_state;
            byte_type power_supply_state;
            byte_type thermal_state;
            byte_type security_status;
            /* SMBIOS 2.3+ */
            dword_type oem_defined;
            byte_type height;
            byte_type number_of_power_cords;
            byte_type contained_element_count;
            byte_type contained_element_record_length;
            byte_type contained_elements[255 * 255];
        } chassis_information_type;

        typedef struct {
            header_type header;
            string_type socket_designation;
            byte_type type;
            byte_type family;
            string_type manufacturer;
            qword_type id;
            string_type version;
            byte_type voltage;
            word_type external_clock;
            word_type max_speed;
            word_type current_speed;
            byte_type status;
            byte_type upgrade;
            /* SMBIOS 2.1+ */
            handle_type l1_cache_handle;
            handle_type l2_cache_handle;
            handle_type l3_cache_handle;
            /* SMBIOS 2.3+ */
            string_type serial_number;
            string_type asset_tag;
            string_type part_number;
            /* SMBIOS 2.5+ */
            byte_type core_count;
            byte_type core_enabled;
            byte_type thread_count;
            word_type characteristics;
            /* SMBIOS 2.6+ */
            word_type family2;
            /* SMBIOS 3.0+ */
            word_type core_count2;
            word_type core_enabled2;
            word_type thread_count2;

            inline const char *get_socket_designation(void) const {
                return smbios_information::get_string(this, this->socket_designation);
            }

            inline const char *get_manufacturer(void) const {
                return smbios_information::get_string(this, this->manufacturer);
            }

            inline const char *get_version(void) const {
                return smbios_information::get_string(this, this->version);
            }

            inline const char *get_serial_number(void) const {
                // TODO: Range check!
                return smbios_information::get_string(this, this->serial_number);
            }

            inline const char *get_asset_tag(void) const {
                // TODO: Range check!
                return smbios_information::get_string(this, this->asset_tag);
            }

            inline const char *get_part_number(void) const {
                // TODO: Range check!
                return smbios_information::get_string(this, this->part_number);
            }
        } processor_information_type;

        typedef struct {
            header_type header;
            byte_type error_detecting_method;
            byte_type error_correction_capability;
            byte_type supported_interleave;
            byte_type current_interleave;
            byte_type maximum_memory_module_size; // 2^n is the maximum size in MB.
            word_type supported_speeds;
            word_type supported_memory_types;
            byte_type memory_module_voltage;
            byte_type number_of_associated_memory_slots;          // x
            handle_type memory_module_configuration_handles[1]; // (2x -1)
            /* SMBIOS 2.1+ */
            //byte_type enabled_error_correcting_capabilities;
        } memory_controller_information_type;

        typedef struct {
            header_type header;
            string_type socket_designation;
            byte_type bank_connections;
            byte_type current_speed;
            word_type type;
            byte_type installed_size;
            byte_type enabled_size;
            byte_type error_status;
        } memory_module_information_type;

        typedef struct {
            header_type header;
            string_type socket_designation;
            word_type configuration;
            word_type maximum_size;         // Bit 15 == 0: 1K; == 1: 64K
            word_type installed_size;       // Bit 15 == 0: 1K; == 1: 64K
            word_type supported_sram_type;
            word_type current_sram_type;
            byte_type speed;
            byte_type error_correction_type;
            byte_type system_cache_type;
            byte_type associativity;
        } cache_information_type;

        typedef struct {
            header_type header;
            /* SMBIOS 2.1+ */
            byte_type location;
            byte_type use;
            byte_type memory_error_correction;
            dword_type maximum_capacity;
            handle_type memory_error_information_handle;
            word_type number_of_memory_devices;
            /* SMBIOS 2.7+ */
            qword_type extended_maximum_capacity;
        } physical_memory_array_type;

        typedef struct {
            header_type header;
            /* SMBIOS 2.1+ */
            handle_type physical_memory_array_handle;
            handle_type memory_error_information_handle;
            word_type total_width;
            word_type data_width;
            word_type size;     // 0: not installed,  7FFFh: use extended_size
            byte_type form_factor;
            byte_type device_set;
            string_type device_locator;
            string_type bank_locator;
            byte_type type;
            word_type type_detail;
            /* SMBIOS 2.3+ */
            word_type speed;
            string_type manufacturer;
            string_type serial_number;
            string_type asset_tag;
            string_type part_number;
            /* SMBIOS 2.6+ */
            byte_type attributes;
            /* SMBIOS 2.7+ */
            dword_type extended_size;
            word_type configured_memory_clock_speed;
            /* SMBIOS 2.8+ */
            word_type minimum_voltage;
            word_type maximum_voltage;
            word_type configured_voltage;

            inline const char *get_device_locator(void) const {
                // TODO: Range check!
                return smbios_information::get_string(this, this->device_locator);
            }

            inline const char *get_bank_locator(void) const {
                // TODO: Range check!
                return smbios_information::get_string(this, this->bank_locator);
            }

            inline const char *get_manufacturer(void) const {
                // TODO: Range check!
                return smbios_information::get_string(this, this->manufacturer);
            }

            inline const char *get_serial_number(void) const {
                // TODO: Range check!
                return smbios_information::get_string(this, this->serial_number);
            }

            inline const char *get_part_number(void) const {
                // TODO: Range check!
                return smbios_information::get_string(this, this->part_number);
            }
        } memory_device_type;

        typedef struct {
            header_type header;
            /* SMBIOS 2.1+ */
            byte_type type;
            byte_type granularity;
            byte_type operation;
            dword_type vendor_syndrome;
            dword_type memory_array_error_address;
            dword_type device_error_address;
            dword_type resolution;
        } memory_error_information_type;

#pragma pack(pop)

        /// <summary>
        /// Answer the string representation of a memory device form factor
        /// <paramref name="value" /> from a
        /// <see cref="trrojan::smbios_information::memory_device_type" />
        /// structure.
        /// </summary>
        /// <param name="value">A value from a
        /// <see cref="trrojan::smbios_information::memory_device_type::form_factor" />
        /// member.</param>
        /// <returns>A human-readable interpretation of
        /// <paramref name="value" />.</returns>
        static std::string decode_memory_device_form_factor(
            const byte_type value);

        /// <summary>
        /// Answer the string representation of a memory device type
        /// <paramref name="value" /> from a
        /// <see cref="trrojan::smbios_information::memory_device_type" />
        /// structure.
        /// </summary>
        /// <param name="value">A value from a
        /// <see cref="trrojan::smbios_information::memory_device_type::type" />
        /// member.</param>
        /// <returns>A human-readable interpretation of
        /// <paramref name="value" />.</returns>
        static std::string decode_memory_device_type(const byte_type value);

        /// <summary>
        /// Answer the string representation of a memory device type details
        /// <paramref name="value" /> from a
        /// <see cref="trrojan::smbios_information::memory_device_type" />
        /// structure.
        /// </summary>
        /// <param name="value">A value from a
        /// <see cref="trrojan::smbios_information::memory_device_type::type_detail" />
        /// member.</param>
        /// <returns>A human-readable interpretation of
        /// <paramref name="value" />.</returns>
        static std::string decode_memory_device_type_detail(
            const word_type value);

        /// <summary>
        /// Decode the <paramref name="id" />th  string value in an SMBIOS
        /// structure of type <tparamref name="T" />.
        /// </summary>
        /// <param name="structure"></param>
        /// <param name="id"></param>
        /// <tparam name="T"></tparam>
        /// <returns></returns>
        template<class T>
        static const char *get_string(const T *structure, const string_type id);

        /// <summary>
        /// Reads the SMBIOS information of the local system and returns the
        /// whole block.
        /// </summary>
        /// <exception cref="std::runtime_error">In case the SMBIOS data could
        /// not be read.</exception>
        /// <exception cref="std::bad_alloc">In case memory for the SMBIOS table
        /// could not be allocated.</exception>
        static smbios_information read(void);

        //template<class T> static inline byte_type type_id(void) {
        //    throw std::exception("Unknown type.");
        //}

        //template<> static inline byte_type type_id<bios_information_type>(void) {
        //    return 0;
        //}

        /// <summary>
        /// Initialises an empty instance.
        /// </summary>
        inline smbios_information(void)
            : enumFlags(0), tableBegin(0), tableEnd(0) { }

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs"></param>
        smbios_information(const smbios_information& rhs);

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs"></param>
        smbios_information(smbios_information&& rhs);

        /// <summary>
        /// Finalise the instance.
        /// </summary>
        ~smbios_information(void);

        template<class I, class P> void entries(I oit, P predicate) const;

        template<class I> inline void entries(I oit) const {
            this->entries(oit, [](const header_type *) -> bool { return true; });
        }

        /// <summary>
        /// Retrieves all SMBIOS entries of the specified type
        /// <tparamref name="T" />.
        /// </summary>
        /// <param name="oit">An output iterator for <tparamref name="T" />.
        /// </param>
        /// <tparam name=T"></tparam>
        /// <tparam name=I"></tparam>
        template<class T, class I> void entries_by_type(I oit) const;

        template<class I>
        inline void entries_by_type_id(I oit, const byte_type type) const {
            this->entries(oit, [type](const header_type *h) -> bool {
                return (h->type == type);
            });
        }

        //template<class T> const T *entry_point(void) const;

        //template<class T> inline bool has_entry_point(void) const {
        //    return false;
        //}

        //template<>
        //bool inline has_entry_point<dmi_entry_point_type>(void) const {
        //    return (this->rawData.size() >= sizeof(dmi_entry_point_type));
        //}

        //template<>
        //bool inline has_entry_point<smbios_2_1_entry_point_type>(void) const {
        //    const char *ANCHOR = "_SM_";
        //    return ((this->rawData.size() > sizeof(ANCHOR))
        //        && (::memcmp(this->rawData.data(), ANCHOR, sizeof(ANCHOR)) == 0));
        //}

        //template<>
        //bool inline has_entry_point<smbios_3_0_entry_point_type>(void) const {
        //    const char *ANCHOR = "_SM3_";
        //    return ((this->rawData.size() > sizeof(ANCHOR))
        //        && (::memcmp(this->rawData.data(), ANCHOR, sizeof(ANCHOR)) == 0));
        //}

        smbios_information& operator =(const smbios_information& rhs);

        smbios_information& operator =(smbios_information&& rhs);

    private:

        static bool validate_checksum(const uint8_t *buf, const size_t len);

        uint32_t enumFlags;
        std::vector<uint8_t> rawData;
        size_t tableBegin;
        size_t tableEnd;
    };


namespace detail {

    /// <summary>
    /// Allows for inferring the ID of a SMBIOS structure from its C++ type.
    /// </summary>
    /// <tparam name="T">The type of the SMBIOS structure to derive the ID
    /// for.</tparam>
    template<class T> struct structure_desc { };

    template<>
    struct structure_desc<smbios_information::bios_information_type> {
        static const smbios_information::byte_type id = 0;
    };

    template<>
    struct structure_desc<smbios_information::system_information_type> {
        static const smbios_information::byte_type id = 1;
    };

    template<>
    struct structure_desc<smbios_information::baseboard_information_type> {
        static const smbios_information::byte_type id = 2;
    };

    template<>
    struct structure_desc<smbios_information::chassis_information_type> {
        static const smbios_information::byte_type id = 3;
    };

    template<>
    struct structure_desc<smbios_information::processor_information_type> {
        static const smbios_information::byte_type id = 4;
    };

    template<>
    struct structure_desc<smbios_information::memory_controller_information_type> {
        static const smbios_information::byte_type id = 5;
    };

    template<>
    struct structure_desc<smbios_information::memory_module_information_type> {
        static const smbios_information::byte_type id = 6;
    };

    template<>
    struct structure_desc<smbios_information::cache_information_type> {
        static const smbios_information::byte_type id = 7;
    };

    template<>
    struct structure_desc<smbios_information::physical_memory_array_type> {
        static const smbios_information::byte_type id = 16;
    };

    template<>
    struct structure_desc<smbios_information::memory_device_type> {
        static const smbios_information::byte_type id = 17;
    };

    template<>
    struct structure_desc<smbios_information::memory_error_information_type> {
        static const smbios_information::byte_type id = 18;
    };


    /// <summary>
    /// Answer the number of elements in a stack-allocated array.
    /// </summary>
    /// <remarks>
    /// This is a backport of <see cref="std::size" /> from the C++17 STL, which
    /// is already available in VS2015, but not in gcc.
    /// </remarks>
    template<class T, size_t S>
    inline constexpr size_t size(const T(&)[S]) noexcept {
        return S;
    }
}
}

#include "smbios_information.inl"
