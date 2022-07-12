// <copyright file="graphics_pipeline_build.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


namespace trrojan {
namespace d3d12 {
namespace detail {

    /// <summary>
    /// Base template for retrieving a specific element from a tuple.
    /// </summary>
    template<class TNeedle, class TTuple, std::size_t Idx, class TSubobjs>
    struct subobj_finder { };

    /// <summary>
    /// Unsuccessful recursion stop.
    /// </summary>
    template<class TNeedle, class TTuple, std::size_t Idx>
    struct subobj_finder<TNeedle, TTuple, Idx, std::tuple<>> {
        typedef typename tuple_cat_type<TTuple, std::tuple<TNeedle>>::type type;
        static constexpr inline TNeedle *get(TTuple&) {
            return nullptr;
        }
    };

    /// <summary>
    /// Successful recursion stop.
    /// </summary>
    template<class TNeedle, class TTuple, std::size_t Idx, class... TSubobjs>
    struct subobj_finder<TNeedle, TTuple, Idx, std::tuple<TNeedle, TSubobjs...>> {
        typedef TTuple type;
        static constexpr inline TNeedle *get(TTuple& tuple) {
            return std::addressof(std::get<Idx>(tuple));
        }
    };

    /// <summary>
    /// Recursion step.
    /// </summary>
    template<class TNeedle, class TTuple, std::size_t Idx, class TSubobj,
        class... TSubobjs>
    struct subobj_finder<TNeedle, TTuple, Idx, std::tuple<TSubobj, TSubobjs...>>
        : subobj_finder<TNeedle, TTuple, Idx + 1, std::tuple<TSubobjs...>> { };

    /// <summary>
    /// Determine the type of the graphics pipline build from a tuple.
    /// </summary>
    template<class TTuple> struct build_from_tuple { };

    template<class... TTypes> struct build_from_tuple<std::tuple<TTypes...>> {
        typedef tgraphics_pipeline_build<TTypes...> type;
    };

    /// <summary>
    /// Retrieve the prefix of a tuple.
    /// </summary>
    template<class... TTypes, std::size_t... Idxs>
    auto sub_tie(std::tuple<TTypes...>& tuple, std::index_sequence<Idxs...>)
            -> decltype(std::tie(std::get<Idxs>(tuple)...)) {
        return std::tie(std::get<Idxs>(tuple)...);
    }

} /*namespace detail */
} /* namespace d3d12 */
} /* namespace trrojan */


/*
 * trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::build
 */
template<class... TSubobjects>
ATL::CComPtr<ID3D12PipelineState>
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::build(
        ID3D12Device2 *device) {
    if (device == nullptr) {
        throw ATL::CAtlException(E_POINTER);
    }

    D3D12_PIPELINE_STATE_STREAM_DESC desc;
    desc.SizeInBytes = sizeof(this->_subobjects);
    desc.pPipelineStateSubobjectStream = std::addressof(this->_subobjects);

    ATL::CComPtr<ID3D12PipelineState> retval;
    auto hr = device->CreatePipelineState(&desc, IID_PPV_ARGS(&retval));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * ...::detail::tgraphics_pipeline_build<TSubobjects...>::depth_stencil_desc
 */
template<class... TSubobjects>
D3D12_DEPTH_STENCIL_DESC&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::depth_stencil_desc(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL>(replacement);
}


/*
 * ...::detail::tgraphics_pipeline_build<TSubobjects...>::depth_stencil_format
 */
template<class... TSubobjects>
DXGI_FORMAT&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::depth_stencil_format(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT>(
        replacement);
}


/*
 * ...::detail::tgraphics_pipeline_build<TSubobjects...>::domain_shader
 */
template<class... TSubobjects>
D3D12_SHADER_BYTECODE&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::domain_shader(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_DS>(replacement);
}


/*
 * ...::detail::tgraphics_pipeline_build<TSubobjects...>::geometry_shader
 */
template<class... TSubobjects>
D3D12_SHADER_BYTECODE&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::geometry_shader(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_GS>(replacement);
}


/*
 * trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::get
 */
template<class... TSubobjects>
template<class T>
T& trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::get(
        pointer_type& replacement) {
    typedef typename std::decay<decltype(this->_subobjects)>::type tuple_type;
    typedef subobj_finder<T, tuple_type, 0, tuple_type> finder_type;

    if (finder_type::get(this->_subobjects) == nullptr) {
        auto r = new build_from_tuple<finder_type::type>::type();
        sub_tie(r->_subobjects, std::make_index_sequence<
            sizeof...(TSubobjects)> { }) = this->_subobjects;
        replacement.reset(r);
        return r->get<T>(replacement);

    } else {
        return *finder_type::get(this->_subobjects);
    }
}


/*
 * ...::detail::tgraphics_pipeline_build<TSubobjects...>::hull_shader
 */
template<class... TSubobjects>
D3D12_SHADER_BYTECODE&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::hull_shader(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_HS>(replacement);
}


/*
 * ...detail::tgraphics_pipeline_build<TSubobjects...>::input_layout_desc
 */
template<class... TSubobjects>
D3D12_INPUT_LAYOUT_DESC&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::input_layout_desc(
        pointer_type &replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT>(replacement);
}


/*
 * ...detail::tgraphics_pipeline_build<TSubobjects...>::pixel_shader
 */
template<class... TSubobjects>
D3D12_SHADER_BYTECODE&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::pixel_shader(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_PS>(replacement);
}


/*
 * ...::detail::tgraphics_pipeline_build<TSubobjects...>::primitive_topology
 */
template<class... TSubobjects>
D3D12_PRIMITIVE_TOPOLOGY_TYPE&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::primitive_topology(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY>(
        replacement);
}


/*
 * ...detail::tgraphics_pipeline_build<TSubobjects...>::render_target_formats
 */
template<class... TSubobjects>
D3D12_RT_FORMAT_ARRAY&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::render_target_formats(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS>(
        replacement);
}


/*
 * ...::detail::tgraphics_pipeline_build<TSubobjects...>::root_signature
 */
template<class... TSubobjects>
ID3D12RootSignature *&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::root_signature(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE>(
        replacement);
}


/*
 * ...::detail::tgraphics_pipeline_build<TSubobjects...>::sample_desc
 */
template<class... TSubobjects>
DXGI_SAMPLE_DESC&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::sample_desc(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC>(replacement);
}


/*
 * ...::detail::tgraphics_pipeline_build<TSubobjects...>::vertex_shader
 */
template<class... TSubobjects>
D3D12_SHADER_BYTECODE&
trrojan::d3d12::detail::tgraphics_pipeline_build<TSubobjects...>::vertex_shader(
        pointer_type& replacement) {
    return this->get<CD3DX12_PIPELINE_STATE_STREAM_VS>(replacement);
}
