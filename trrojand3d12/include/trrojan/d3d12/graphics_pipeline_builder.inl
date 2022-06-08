// <copyright file="graphics_pipeline_builder.inl" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2022 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph M�ller</author>


/*
 * trrojan::d3d12::graphics_pipeline_builder::set_render_targets
 */
template<class... T>
trrojan::d3d12::graphics_pipeline_builder&
trrojan::d3d12::graphics_pipeline_builder::set_render_targets(
        const T... formats) {
    std::vector<DXGI_FORMAT> fmts { formats... };
    return this->set_render_targets(fmts);
}
