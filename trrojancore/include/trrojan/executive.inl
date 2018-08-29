/// <copyright file="executive.inl" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::executive::get_benchmarks
 */
template<class I> void trrojan::executive::get_benchmarks(I oit) const {
    for (auto& p : this->plugins) {
        plugin_base::benchmark_list benchmarks;
        p->create_benchmarks(benchmarks);
        for (auto& b : benchmarks) {
            *oit++ = b;
        }
    }
}


/*
 * trrojan::executive::get_environments
 */
template<class I> void trrojan::executive::get_environments(I oit) const {
    for (auto& e : this->environments) {
        *oit++ = e.second;
    }
}
