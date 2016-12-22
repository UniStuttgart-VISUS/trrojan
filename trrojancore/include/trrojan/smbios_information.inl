/// <copyright file="smbios_information.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph M�ller</author>

/// <copyright file="smbios_information.inl" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2015 Visualisierungsinstitut der Universit�t Stuttgart.
/// Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph M�ller</author>


/*
 * trrojan::smbios_information::get_string
 */
template<class T>
const char *trrojan::smbios_information::get_string(const T *structure,
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
 * trrojan::smbios_information::entries
 */
template<class I, class P, class T>
void trrojan::smbios_information::entries(I oit, P predicate) const {
    const auto BEGIN = this->rawData.data() + this->tableBegin;
    const auto END = this->rawData.data() + this->tableEnd;

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

        // TODO: http://git.savannah.gnu.org/cgit/dmidecode.git/tree/dmidecode.c
        // if (h.type == 127 && (flags & FLAG_STOP_AT_EOT)) break;
    }
}


/*
 * trrojan::smbios_information::entries_by_type
 */
template<class T, class I>
void trrojan::smbios_information::entries_by_type(I oit) const {
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