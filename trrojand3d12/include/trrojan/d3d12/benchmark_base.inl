// <copyright file="benchmark_base.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::d3d12::benchmark_base::create_graphics_command_list_for
 */
template<class... TResource>
trrojan::d3d12::benchmark_base::graphics_command_list
trrojan::d3d12::benchmark_base::create_graphics_command_list_for(
        TResource&&... resources) {
    std::array<IUnknown *, sizeof...(resources)> unknowns { resources.get()...};
    auto create = std::any_of(unknowns.begin(), unknowns.end(),
        [](IUnknown *u) { return (u == nullptr); });
    return create ? this->create_graphics_command_list() : nullptr;
}
