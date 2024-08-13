// <copyright file="rendering_technique.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::d3d11::rendering_technique::combine_shader_stages
 */
template<class ...T>
trrojan::d3d11::rendering_technique::shader_stages
trrojan::d3d11::rendering_technique::combine_shader_stages(T... args) {
    shader_stages retval = 0;
    shader_stages ss[] = { static_cast<shader_stages>(args)... };

    for (auto& s : ss) {
        retval |= s;
    }

    return retval;
}


/*
 * trrojan::d3d11::rendering_technique::assert_range
 */
template<class T>
void trrojan::d3d11::rendering_technique::assert_range(std::vector<T>& dst,
        const std::vector<T>& src, const size_t offset) {
    auto req = offset + src.size();
    if (dst.size() < req) {
        dst.resize(req);
    }
}


/*
 * trrojan::d3d11::rendering_technique::assert_range
 */
template<class T>
void trrojan::d3d11::rendering_technique::assert_range(std::vector<T>& dst,
        const size_t offset) {
    auto req = offset + 1;
    if (dst.size() < req) {
        dst.resize(req);
    }
}



/*
 * trrojan::d3d11::rendering_technique::unsmart
 */
template<class T>
std::vector<T *> trrojan::d3d11::rendering_technique::unsmart(
        std::vector<winrt::com_ptr<T>>& input) {
    std::vector<T *> retval(input.size());
    std::transform(input.begin(), input.end(), retval.begin(),
        [](winrt::com_ptr<T>& i) { return i.get(); });
    return std::move(retval);
}
