// <copyright file="benchmark.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Valentin Bruder</author>
// <author>Christoph Müller</author>


/*
 * trrojan::benchmark_base::check_changed_factors
 */
template<class I> bool trrojan::benchmark_base::check_changed_factors(
        const trrojan::configuration& new_config, I oit,
        const bool update_last) {
    auto retval = false;

    // Note: new_config must be iterated in the outer loop, because
    // it allows us to detect whether there is a new factor (this is
    // relevant for the first call).
    for (auto& n : new_config) {
        bool found = false;

        for (auto& l : this->_last_config) {
            if (n.name() == l.name()) {
                found = true;
                if (n.value() != l.value()) {
                    *oit++ = n.name();
                    retval = true;
                }
            }
        } /* end for (auto& l : this->_last_config) */

        if (!found) {
            // If the factor was not in the old configuration, we need to
            // report it as changed, too.
            *oit++ = n.name();
            retval = true;
        }
    }

    if (retval && update_last) {
        // Update the reference configuration as needed and requested.
        this->_last_config = new_config;
    }

    return retval;
}
