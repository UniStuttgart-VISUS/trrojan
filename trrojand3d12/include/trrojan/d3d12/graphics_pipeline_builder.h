// <copyright file="graphics_pipeline_builder.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <functional>
#include <type_traits>
#include <utility>

#include "trrojan/aligned_allocator.h"
#include "trrojan/enum_dispatch_list.h"
#include "trrojan/io.h"

#include "trrojan/d3d12/pipeline_state_subobject_type_traits.h"
#include "trrojan/d3d12/plugin.h"
#include "trrojan/d3d12/utilities.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// A builder object for <see cref="D3D12_GRAPHICS_PIPELINE_STATE_DESC" />.
    /// </summary>
    /// <remarks>
    /// The builder will not only hold the descriptor, but also create a copy
    /// of all dynamic elements like shader code to make sure that there are
    /// no dangling pointers to these data.
    /// </remarks>
    class graphics_pipeline_builder final {

    public:

        /// <summary>
        /// Extracts, if any embedded, the root signature from shader byte code.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="byte_code"></param>
        /// <returns></returns>
        static ATL::CComPtr<ID3D12RootSignature> root_signature_from_shader(
            ID3D12Device *device, const std::vector<BYTE>& byte_code);

        /// <summary>
        /// Extracts the root signature from the first shader in the pipeline
        /// that has one.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="builder"></param>
        /// <returns></returns>
        static ATL::CComPtr<ID3D12RootSignature> root_signature_from_shader(
            ID3D12Device *device, const graphics_pipeline_builder& builder);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        graphics_pipeline_builder(void);

        /// <summary>
        /// Build a <see cref="ID3D12PipelineState" /> from the current state of
        /// the descriptor.
        /// </summary>
        /// <param name="device"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12PipelineState> build(ID3D12Device2 *device);

        /// <summary>
        /// Build a <see cref="ID3D12PipelineState" /> from the current state of
        /// the descriptor.
        /// </summary>
        /// <param name="device"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12PipelineState> build(ID3D12Device *device);

        /// <summary>
        /// Removes all shader subobjects from the builder.
        /// </summary>
        void reset_shaders(void);

        /// <summary>
        /// Convenience method for enabling depth buffer.
        /// </summary>
        /// <param name="enabled"></param>
        /// <param name="func"></param>
        /// <param name="mask"></param>
        /// <returns></returns>
        inline graphics_pipeline_builder& set_depth_state(const bool enabled,
                const D3D12_COMPARISON_FUNC func = D3D12_COMPARISON_FUNC_LESS,
                const D3D12_DEPTH_WRITE_MASK mask = D3D12_DEPTH_WRITE_MASK_ALL) {
            auto& so = this->get_value<
                CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL>();
            so.DepthEnable = enabled ? TRUE : FALSE;
            so.DepthFunc = func;
            so.DepthWriteMask = mask;
            return *this;
        }

        /// <summary>
        /// Apply the specified depth stencil state.
        /// </summary>
        /// <param name="desc"></param>
        /// <returns></returns>
        inline graphics_pipeline_builder& set_depth_stencil_state(
                const D3D12_DEPTH_STENCIL_DESC& desc) {
            auto& so = this->get_value<
                CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL>();
            // For some weird reason, this must be explicit ...
            so = CD3DX12_DEPTH_STENCIL_DESC(desc);
            return *this;
        }

        inline graphics_pipeline_builder& set_depth_stencil_format(
                const DXGI_FORMAT format) {
            auto& so = this->get_value<
                CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT>();
            so = format;
            return *this;
        }

        inline graphics_pipeline_builder& set_domain_shader(const BYTE *byte_code,
                const std::size_t cnt_byte_code) {
            return this->set_domain_shader(std::vector<BYTE>(byte_code,
                byte_code + cnt_byte_code));
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_domain_shader(
                const BYTE(&byte_code)[N]) {
            return this->set_domain_shader(std::vector<BYTE>(byte_code,
                byte_code + N));
        }

        inline graphics_pipeline_builder& set_domain_shader(
                std::vector<std::uint8_t>&& byte_code) {
            auto& so = this->get_value<CD3DX12_PIPELINE_STATE_STREAM_DS>();
            this->_ds = std::move(byte_code);
            set_shader(so, this->_ds.data(), this->_ds.size());
            return *this;
        }

        inline graphics_pipeline_builder& set_domain_shader_from_file(
                LPCSTR path) {
            return this->set_domain_shader(read_binary_file(path));
        }

        inline graphics_pipeline_builder& set_domain_shader_from_file(
                const std::string& path) {
            return this->set_domain_shader_from_file(path.data());
        }

#if !defined(_UWP)
        inline graphics_pipeline_builder& set_domain_shader_from_resource(
                LPCTSTR name, LPCTSTR type = _T("SHADER")) {
            return this->set_domain_shader(d3d12::plugin::load_resource(name,
                type));
        }
#endif /* !defined(_UWP) */

        inline graphics_pipeline_builder& set_domain_shader(LPCSTR path) {
            return this->set_domain_shader(read_binary_file(path));
        }

        inline graphics_pipeline_builder& set_geometry_shader(
                const BYTE *byte_code,
                const std::size_t cnt_byte_code) {
            return this->set_geometry_shader(std::vector<BYTE>(byte_code,
                byte_code + cnt_byte_code));
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_geometry_shader(
                const BYTE(&byte_code)[N]) {
            return this->set_geometry_shader(std::vector<BYTE>(byte_code,
                byte_code + N));
        }

        inline graphics_pipeline_builder& set_geometry_shader(
                std::vector<std::uint8_t>&& byte_code) {
            auto& so = this->get_value<CD3DX12_PIPELINE_STATE_STREAM_GS>();
            this->_gs = std::move(byte_code);
            set_shader(so, this->_gs.data(), this->_gs.size());
            return *this;
        }

        inline graphics_pipeline_builder& set_geometry_shader_from_file(
                LPCSTR path) {
            return this->set_geometry_shader(read_binary_file(path));
        }

        inline graphics_pipeline_builder& set_geometry_shader_from_file(
                const std::string& path) {
            return this->set_geometry_shader_from_file(path.data());
        }

#if !defined(_UWP)
        inline graphics_pipeline_builder& set_geometry_shader_from_resource(
                LPCTSTR name, LPCTSTR type = _T("SHADER")) {
            return this->set_geometry_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(_UWP) */

        inline graphics_pipeline_builder& set_hull_shader(const BYTE *byte_code,
                const std::size_t cnt_byte_code) {
            return this->set_hull_shader(std::vector<BYTE>(byte_code,
                byte_code + cnt_byte_code));
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_hull_shader(
                const BYTE(&byte_code)[N]) {
            return this->set_hull_shader(std::vector<BYTE>(byte_code,
                byte_code + N));
        }

        inline graphics_pipeline_builder& set_hull_shader(
                std::vector<std::uint8_t>&& byte_code) {
            auto& so = this->get_value<CD3DX12_PIPELINE_STATE_STREAM_HS>();
            this->_hs = std::move(byte_code);
            set_shader(so, this->_hs.data(), this->_hs.size());
            return *this;
        }

        inline graphics_pipeline_builder& set_hull_shader_from_file(
                LPCSTR path) {
            return this->set_hull_shader(read_binary_file(path));
        }

        inline graphics_pipeline_builder& set_hull_shader_from_file(
                const std::string& path) {
            return this->set_hull_shader_from_file(path.data());
        }

#if !defined(_UWP)
        inline graphics_pipeline_builder& set_hull_shader_from_resource(
                LPCTSTR name, LPCTSTR type = _T("SHADER")) {
            return this->set_hull_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(_UWP) */

        inline graphics_pipeline_builder& set_input_layout(
                const D3D12_INPUT_ELEMENT_DESC *elements,
                const std::size_t cnt) {
            return this->set_input_layout(std::vector<D3D12_INPUT_ELEMENT_DESC>(
                elements, elements + cnt));
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_input_layout(
                const D3D12_INPUT_ELEMENT_DESC(&elements)[N]) {
            return this->set_input_layout(elements, N);
        }

        inline graphics_pipeline_builder& set_input_layout(
                std::vector<D3D12_INPUT_ELEMENT_DESC>&& elements) {
            auto& so = this->get_value<
                CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT>();
            this->_il = std::move(elements);
            so.pInputElementDescs = this->_il.data();
            so.NumElements = static_cast<UINT>(this->_il.size());
            return *this;
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_input_layout(
                const std::array<D3D12_INPUT_ELEMENT_DESC, N>& elements) {
            return this->set_input_layout(std::vector< D3D12_INPUT_ELEMENT_DESC>(
                elements.begin(), elements.end()));
        }

        template<class... T>
        inline graphics_pipeline_builder& set_input_layout(T... elements) {
            std::vector<D3D12_INPUT_ELEMENT_DESC> il = { elements... };
            return this->set_input_layout(std::move(il));
        }

        inline graphics_pipeline_builder& set_pixel_shader(
                const BYTE *byte_code,
                const std::size_t cnt_byte_code) {
            return this->set_pixel_shader(std::vector<BYTE>(byte_code,
                byte_code + cnt_byte_code));
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_pixel_shader(
                const BYTE(&byte_code)[N]) {
            return this->set_pixel_shader(std::vector<BYTE>(byte_code,
                byte_code + N));
        }

        inline graphics_pipeline_builder& set_pixel_shader(
                std::vector<std::uint8_t>&& byte_code) {
            auto& so = this->get_value<CD3DX12_PIPELINE_STATE_STREAM_PS>();
            this->_ps = std::move(byte_code);
            set_shader(so, this->_ps.data(), this->_ps.size());
            return *this;
        }

        //inline graphics_pipeline_builder& set_pixel_shader(
        //        const std::vector<std::uint8_t>& byte_code) {
        //    this->_ps = byte_code;
        //    set_shader(this->_desc.PS, this->_ps.data(), this->_ps.size());
        //    return *this;
        //}

        inline graphics_pipeline_builder& set_pixel_shader_from_file(
                LPCSTR path) {
            return this->set_pixel_shader(read_binary_file(path));
        }

        inline graphics_pipeline_builder& set_pixel_shader_from_file(
                const std::string& path) {
            return this->set_pixel_shader_from_file(path.data());
        }

#if !defined(_UWP)
        inline graphics_pipeline_builder &set_pixel_shader_from_resource(
            LPCTSTR name, LPCTSTR type = _T("SHADER")) {
            return this->set_pixel_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(_UWP) */

        inline graphics_pipeline_builder& set_primitive_topology(
                const D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) {
            auto& so = this->get_value<
                CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY>();
            so = topology;
            return *this;
        }

        template<class... T>
        graphics_pipeline_builder& set_render_targets(const T... formats);

        graphics_pipeline_builder& set_render_targets(
            const std::vector<DXGI_FORMAT>& formats);

        /// <summary>
        /// Sets the specified root signature.
        /// </summary>
        /// <remarks>
        /// The builder will hold a reference of the root signature while it is
        /// alive.
        /// </remarks>
        /// <param name="root_signature"></param>
        /// <returns></returns>
        graphics_pipeline_builder& set_root_signature(
            ID3D12RootSignature *root_signature);

        inline graphics_pipeline_builder& set_sample_desc(
                const DXGI_SAMPLE_DESC& desc) {
            auto& so = this->get_value<
                CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC>();
            so = desc;
            return *this;
        }

        inline graphics_pipeline_builder& set_sample_desc(
                const UINT count = 1, const UINT quality = 0) {
            auto& so = this->get_value<
                CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC>();
            so.Count = count;
            so.Quality = quality;
            return *this;
        }

        inline graphics_pipeline_builder& set_two_sided(
                const bool two_sided = true) {
            auto& so = this->get_value<
                CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER>();
            so.CullMode = two_sided
                ? D3D12_CULL_MODE_NONE
                : D3D12_CULL_MODE_BACK;
            return *this;
        }

        inline graphics_pipeline_builder& set_vertex_shader(
                const BYTE *byte_code,
                const std::size_t cnt_byte_code) {
            return this->set_vertex_shader(std::vector<BYTE>(byte_code,
                byte_code + cnt_byte_code));
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_vertex_shader(
                const BYTE(&byte_code)[N]) {
            return this->set_vertex_shader(std::vector<BYTE>(byte_code,
                byte_code + N));
        }

        inline graphics_pipeline_builder& set_vertex_shader(
                std::vector<std::uint8_t>&& byte_code) {
            auto& so = this->get_value<CD3DX12_PIPELINE_STATE_STREAM_VS>();
            this->_vs = std::move(byte_code);
            set_shader(so, this->_vs.data(), this->_vs.size());
            return *this;
        }

        inline graphics_pipeline_builder& set_vertex_shader_from_file(
                LPCSTR path) {
            return this->set_vertex_shader(read_binary_file(path));
        }

        inline graphics_pipeline_builder& set_vertex_shader_from_file(
                const std::string& path) {
            return this->set_vertex_shader_from_file(path.data());
        }

#if !defined(_UWP)
        inline graphics_pipeline_builder& set_vertex_shader_from_resource(
                LPCTSTR name, LPCTSTR type = _T("SHADER")) {
            return this->set_vertex_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(_UWP) */

    private:

        typedef aligned_allocator<BYTE> alloc_type;

        typedef contiguous_enum_dispatch_list<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE,
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING> subobj_disp_list;

        template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE... Types>
        using subobj_type_list = enum_dispatch_list<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE, Types...>;

        template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type,
                D3D12_PIPELINE_STATE_SUBOBJECT_TYPE... Types>
        inline static constexpr std::size_t get_subobject_size(
                subobj_type_list<Type, Types...>,
                const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value) {
            if (value == Type) {
                return sizeof(pipeline_state_subobject_type_traits<Type>::type);
            } else {
                return get_subobject_size(subobj_type_list<Types...> { }, value);
            }
        }

        inline static std::size_t get_subobject_size(subobj_type_list<>,
                const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value) {
            throw std::invalid_argument("An unexpected pipeline state "
                "subobject was encountered.");
        }

        template<class TCallback>
        std::size_t foreach_subobject(const TCallback& callback);

        template<class TSubobj> TSubobj& get_subobject(void);

        template<class TSubobj>
        inline typename subobject_inner_type<TSubobj>::type& get_value(void) {
            typedef typename subobject_inner_type<TSubobj>::type type;
            return static_cast<type&>(this->get_subobject<TSubobj>());
        }

        std::vector<BYTE> _ds;
        std::vector<BYTE> _gs;
        std::vector<BYTE> _hs;
        std::vector<D3D12_INPUT_ELEMENT_DESC> _il;
        std::vector<BYTE> _ps;
        ATL::CComPtr<ID3D12RootSignature> _root_sig;
        std::vector<BYTE, alloc_type> _stream;
        std::vector<BYTE> _vs;

    };

} /* end namespace d3d12 */
} /* end namespace trrojan */

#include "trrojan/d3d12/graphics_pipeline_builder.inl"
