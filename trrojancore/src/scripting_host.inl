/// <copyright file="scripting_host.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


#if defined(WITH_CHAKRA)
/*
 * trrojan::scripting_host::fits_range
 */
template<class T>
bool trrojan::scripting_host::fits_range(const double value) {
    if (std::is_integral<T>::value) {
        auto i = 0.0;
        if (std::modf(value, &i) != 0.0) {
            // Target should be integral, but value is not.
            return false;
        }
    }

    return ((std::numeric_limits<T>::lowest() <= value)
        && ((std::numeric_limits<T>::max)() >= value));
}


/*
 * trrojan::variant trrojan::scripting_host::to_variant
 */
template<class T> trrojan::variant trrojan::scripting_host::to_variant(
        const std::vector<double>& value) {
    switch (value.size()) {
        case 1:
            return static_cast<T>(value[0]);

        case 2:
            return std::array<T, 2> {
                static_cast<T>(value[0]),
                static_cast<T>(value[1])
            };

        case 3:
            return std::array<T, 3> {
                static_cast<T>(value[0]),
                static_cast<T>(value[1]),
                static_cast<T>(value[2])
            };

        case 4:
            return std::array<T, 4> {
                static_cast<T>(value[0]),
                static_cast<T>(value[1]),
                static_cast<T>(value[2]),
                static_cast<T>(value[3])
            };

        default:
            throw std::runtime_error("Vectorial variants must have at most "
                "four components.");
    }
}
#endif /* defined(WITH_CHAKRA) */
