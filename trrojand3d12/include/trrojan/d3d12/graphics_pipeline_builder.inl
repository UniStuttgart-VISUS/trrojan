// <copyright file="graphics_pipeline_builder.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


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


/*
 * trrojan::d3d12::graphics_pipeline_builder::foreach_subobject
 */
template<class TCallback>
std::size_t trrojan::d3d12::graphics_pipeline_builder::foreach_subobject(
        const TCallback& callback) {
    std::size_t retval = 0;

    for (std::size_t o = 0, s = 0; o < this->_stream.size(); o += s) {
        auto d = this->_stream.data() + o;
        auto t = *reinterpret_cast<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE *>(d);

        s = get_subobject_size(subobj_disp_list { }, t);
        ++retval;

        if (!callback(t, o, s)) {
            break;
        }
    }

    return retval;
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::get_subobject
 */
template<class TSubobj>
TSubobj& trrojan::d3d12::graphics_pipeline_builder::get_subobject(void) {
    BYTE *retval = nullptr;

    // Check whether the subobject is already in the stream.
    this->foreach_subobject([&retval, this](
            const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE t,
            const std::size_t o,
            const std::size_t s) {
        if (t == typename subobject_type<TSubobj>::value) {
            retval = this->_stream.data() + o;
            return false;
        } else {
            return true;
        }
    });

    // Not not found, add subobject at the end.
    if (retval == nullptr) {
        auto o = this->_stream.size();
        auto s = get_subobject_size(subobj_disp_list{},
            typename subobject_type<TSubobj>::value);
        this->_stream.resize(o + s);
        retval = this->_stream.data() + o;
        new (retval) TSubobj();
    }

    return *reinterpret_cast<TSubobj *>(retval);
}
