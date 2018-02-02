/// <copyright file="rendering_technique.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::d3d11::rendering_technique::set_constant_buffers
 */
template<class... T>
void trrojan::d3d11::rendering_technique::set_constant_buffers(
        const shader_stages stages, const UINT start, T... buffers) {
    ATL::CComPtr<ID3D11Buffer> src[] = { buffers... };
    const auto size = start + sizeof...(buffers);

    for (auto s = static_cast<shader_stages>(shader_stage::vertex);
            s <= static_cast<shader_stages>(shader_stage::pixel);
            s <<= 1) {
        auto t = static_cast<shader_stage>(s);
        auto& dst = this->_resources[t].constant_buffers;

        if (dst.size() < size) {
            dst.resize(size);
        }

        std::copy(src, src + size, dst.begin() + start);
    }
}


/*
 * trrojan::d3d11::rendering_technique::set_shader_resource_views
 */
template<class... T>
void trrojan::d3d11::rendering_technique::set_shader_resource_views(
        const shader_stages stages, const UINT start, T... views) {
    ATL::CComPtr<ID3D11ShaderResourceView> src[] = { views... };
    const auto size = start + sizeof...(views);

    for (auto s = static_cast<shader_stages>(shader_stage::vertex);
            s <= static_cast<shader_stages>(shader_stage::pixel);
            s <<= 1) {
        auto t = static_cast<shader_stage>(s);
        auto& dst = this->_resources[t].resource_views;

        if (dst.size() < size) {
            dst.resize(size);
        }

        std::copy(src, src + size, dst.begin() + start);
    }
}


/*
 * trrojan::d3d11::rendering_technique::unsmart
 */
template<class T>
std::vector<T *> trrojan::d3d11::rendering_technique::unsmart(
        std::vector<ATL::CComPtr<T>>& input) {
    std::vector<T *> retval(input.size());
    std::transform(input.begin(), input.end(), retval.begin(),
        [](ATL::CComPtr<T>& i) { return i.p; });
    return std::move(retval);
}
