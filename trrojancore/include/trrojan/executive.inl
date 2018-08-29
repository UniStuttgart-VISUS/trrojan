/// <copyright file="executive.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
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
