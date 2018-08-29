/// <copyright file="trroll_parser.inl" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::trroll_parser::parse_value
 */
template<trrojan::variant_type T, trrojan::variant_type... Ts>
trrojan::variant trrojan::trroll_parser::parse_value(
        detail::variant_type_list_t<T, Ts...>, const std::string& str,
        const variant_type type) {
    if (type == variant_type::boolean) {
        // Booleans require special handling because of multiple possible
        // representations.
        return trrojan::parse_bool(str);

    } else if ((type == variant_type::string)
            || (type == variant_type::wstring)) {
        // Strings can be returned directly.
        return str;

    } else if ((type == variant_type::device)
        || (type == variant_type::environment)) {
        // Assume devices and environments are names.
        // TODO: unsure whether this is clever ...
        return str;

    } else if (type == T) {
        // Use generic parse method based on std::stringstream.
        //return trrojan::parse<typename variant_type_traits<T>::type>(str);
        return trroll_parser::parse_value<T>(str);

    }  else {
        // Try next T.
        return trroll_parser::parse_value(detail::variant_type_list_t<Ts...>(),
            str, type);
    }
}


/*
 * trrojan::trroll_parser::skip_nonspaces
 */
template<class I> I trrojan::trroll_parser::skip_nonspaces(I it, I end) {
    while ((it != end) && !std::isspace(*it)) {
        ++it;
    }
    return it;
}

/*
 * trrojan::trroll_parser::skip_spaces
 */
template<class I> I trrojan::trroll_parser::skip_spaces(I it, I end) {
    while ((it != end) && std::isspace(*it)) {
        ++it;
    }
    return it;
}


/*
 * trrojan::trroll_parser::to_string
 */
template<trrojan::variant_type T, trrojan::variant_type ...Ts>
std::string trrojan::trroll_parser::to_string(
        detail::variant_type_list_t<T, Ts...>, const variant_type type) {
    if (type == T) {
        return variant_type_traits<T>::name();
    } else {
        return trroll_parser::to_string(detail::variant_type_list_t<Ts...>(),
            type);
    }
}


/*
 * trroll_parser::tokenise
 */
template<class Is, class P>
std::vector<Is> trrojan::trroll_parser::tokenise(Is begin, Is end, P sepPred) {
    auto cur = begin;
    std::vector<Is> retval;

    while (cur != end) {
        retval.push_back(cur);
        cur = std::find_if(++cur, end, sepPred);
    }

    retval.push_back(end);

    return retval;
}



/*
 * trroll_parser::tokenise
 */
template<class Is, class It>
std::vector<Is> trrojan::trroll_parser::tokenise(Is begin, Is end, It sepBegin,
        It sepEnd) {
    typedef typename Is::value_type char_type;
    return trroll_parser::tokenise(begin, end, [&sepBegin, &sepEnd](char_type c) {
        return (std::find(sepBegin, sepEnd, c) != sepEnd);
    });
}
