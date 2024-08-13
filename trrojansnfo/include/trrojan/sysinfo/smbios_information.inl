/// <copyright file="smbios_information.inl" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

/// <copyright file="smbios_information.inl" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2015 Visualisierungsinstitut der Universität Stuttgart.
/// Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::sysinfo::smbios_information::get_string
 */
template<class T>
const char *trrojan::sysinfo::smbios_information::get_string(const T *structure,
        const string_type id) {
    if (structure == nullptr) {
        throw std::invalid_argument("'structure' must not be nullptr.");
    }

    if (id > 0) {
        auto header = &(structure->header);
        auto retval = reinterpret_cast<const char *>(header) + header->length;
        for (string_type s = 1; s < id; ++s) {
            while (*retval != 0) {
                ++retval;
            }
            ++retval;
        }
        // TODO: Handle illegal indices
        return retval;

    } else {
        return "";
    }
}


/*
 * trrojan::sysinfo::smbios_information::entries
 */
template<class I, class P, class T>
void trrojan::sysinfo::smbios_information::entries(I oit, P predicate) const {
    const auto BEGIN = this->rawData + this->tableBegin;
    const auto END = this->rawData + this->tableEnd;

    for (auto ptr = BEGIN; ptr < END;) {
        auto header = reinterpret_cast<const header_type *>(ptr);
        if (header->length < 4) {
            throw std::runtime_error("SMBIOS structure is broken.");
        }

        /* Return the pointer to the header. */
        if (predicate(header)) {
            *oit++ = reinterpret_cast<const T *>(ptr);
        }

        /* Search next handle. */
        // From http://git.savannah.gnu.org/cgit/dmidecode.git/tree/dmidecode.c
        ptr += header->length;
        while ((ptr < END) && ((ptr[0] != 0) || (ptr[1] != 0))) {
            ++ptr;
        }
        ptr += 2;

        // From http://git.savannah.gnu.org/cgit/dmidecode.git/tree/dmidecode.c:
        // SMBIOS v3 requires stopping at this marker.
        if ((header->type == 127) && (this->enumFlags & FLAG_STOP_AT_EOT)) {
            break; 
        }
    }
}


/*
 * trrojan::sysinfo::smbios_information::entries_by_type
 */
template<class T, class I>
void trrojan::sysinfo::smbios_information::entries_by_type(I oit) const {
    static auto pred = [](const header_type *h) -> bool {
        return (h->type == detail::structure_desc<T>::id);
    };
    this->entries<I, decltype(pred), T>(oit, pred);
}

///*
// * smbios_information::entry_point
// */
//template<class T>
//const T *smbios_information::entry_point(void) const {
//    if (this->has_entry_point<T>()) {
//        return this->raw_data<T>();
//    } else {
//        throw std::exception("Incompatible entry point.");
//    }
//}
