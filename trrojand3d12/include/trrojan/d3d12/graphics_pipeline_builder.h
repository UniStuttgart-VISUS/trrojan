// <copyright file="graphics_pipeline_builder.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/io.h"

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
        /// Initialises a new instance.
        /// </summary>
        graphics_pipeline_builder(void);

        /// <summary>
        /// Build a <see cref="ID3D12PipelineState" /> from the current state of
        /// the descriptor.
        /// </summary>
        /// <param name="device"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12PipelineState> build(ID3D12Device *device);

        /// <summary>
        /// Applies the default depth/stencil state as described on
        /// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_depth_stencil_desc.
        /// </summary>
        /// <param name=""></param>
        /// <returns></returns>
        graphics_pipeline_builder& reset_depth_stencil_state(void);

        /// <summary>
        /// Applies the default rasteriser state as described on
        /// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_rasterizer_desc.
        /// </summary>
        /// <param name=""></param>
        graphics_pipeline_builder& reset_rasteriser_state(void);

        /// <summary>
        /// Erases all shaders.
        /// </summary>
        graphics_pipeline_builder& reset_shaders(void);

        /// <summary>
        /// Apply the specified depth stencil state.
        /// </summary>
        /// <param name="desc"></param>
        /// <returns></returns>
        inline graphics_pipeline_builder& set_depth_stencil_state(
                const D3D12_DEPTH_STENCIL_DESC& desc) {
            this->_desc.DepthStencilState = desc;
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
            this->_ds = std::move(byte_code);
            set_shader(this->_desc.DS, this->_ds.data(), this->_ds.size());
            return *this;
        }

#if !defined(TRROJAN_FOR_UWP)
        inline graphics_pipeline_builder& set_domain_shader_from_resource(
                LPCTSTR name, LPCTSTR type = _T("SHADER")) {
            return this->set_domain_shader(d3d12::plugin::load_resource(name,
                type));
        }
#endif /* !defined(TRROJAN_FOR_UWP) */

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
            this->_gs = std::move(byte_code);
            set_shader(this->_desc.GS, this->_gs.data(), this->_gs.size());
            return *this;
        }

#if !defined(TRROJAN_FOR_UWP)
        inline graphics_pipeline_builder& set_geometry_shader_from_resource(
                LPCTSTR name, LPCTSTR type = _T("SHADER")) {
            return this->set_geometry_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(TRROJAN_FOR_UWP) */

        inline graphics_pipeline_builder& set_geometry_shader_from_file(
                LPCSTR path) {
            return this->set_geometry_shader(read_binary_file(path));
        }

        inline graphics_pipeline_builder& set_geometry_shader_from_file(
                const std::string& path) {
            return this->set_geometry_shader_from_file(path.data());
        }

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
            this->_hs = std::move(byte_code);
            set_shader(this->_desc.HS, this->_hs.data(), this->_hs.size());
            return *this;
        }

#if !defined(TRROJAN_FOR_UWP)
        inline graphics_pipeline_builder& set_hull_shader_from_resource(
                LPCTSTR name, LPCTSTR type = _T("SHADER")) {
            return this->set_hull_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(TRROJAN_FOR_UWP) */

        inline graphics_pipeline_builder& set_hull_shader_from_file(
                LPCSTR path) {
            return this->set_hull_shader(read_binary_file(path));
        }

        inline graphics_pipeline_builder& set_hull_shader_from_file(
                const std::string& path) {
            return this->set_hull_shader_from_file(path.data());
        }

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
            this->_il = std::move(elements);
            this->_desc.InputLayout.pInputElementDescs = this->_il.data();
            this->_desc.InputLayout.NumElements = static_cast<UINT>(
                this->_il.size());
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
            this->_ps = std::move(byte_code);
            set_shader(this->_desc.PS, this->_ps.data(), this->_ps.size());
            return *this;
        }

        //inline graphics_pipeline_builder& set_pixel_shader(
        //        const std::vector<std::uint8_t>& byte_code) {
        //    this->_ps = byte_code;
        //    set_shader(this->_desc.PS, this->_ps.data(), this->_ps.size());
        //    return *this;
        //}

#if !defined(TRROJAN_FOR_UWP)
        inline graphics_pipeline_builder& set_pixel_shader_from_resource(
                LPCTSTR name, LPCTSTR type = _T("SHADER")) {
            return this->set_pixel_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(TRROJAN_FOR_UWP) */

        inline graphics_pipeline_builder& set_pixel_shader_from_file(
                LPCSTR path) {
            return this->set_pixel_shader(read_binary_file(path));
        }

        inline graphics_pipeline_builder& set_pixel_shader_from_file(
                const std::string& path) {
            return this->set_pixel_shader_from_file(path.data());
        }

        inline graphics_pipeline_builder& set_primitive_topology(
                const D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) {
            this->_desc.PrimitiveTopologyType = topology;
            return *this;
        }

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
            this->_vs = std::move(byte_code);
            set_shader(this->_desc.VS, this->_vs.data(), this->_vs.size());
            return *this;
        }

#if !defined(TRROJAN_FOR_UWP)
        inline graphics_pipeline_builder& set_vertex_shader_from_resource(
                LPCTSTR name, LPCTSTR type = _T("SHADER")) {
            return this->set_vertex_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(TRROJAN_FOR_UWP) */

        inline graphics_pipeline_builder& set_vertex_shader_from_file(
                LPCSTR path) {
            return this->set_vertex_shader(read_binary_file(path));
        }

        inline graphics_pipeline_builder& set_vertex_shader_from_file(
                const std::string& path) {
            return this->set_vertex_shader_from_file(path.data());
        }

        /// <summary>
        /// Converts the builder into the underlying descriptor object.
        /// </summary>
        /// <returns>The pipeline state descriptor being modified by the
        /// builder.</returns>
        inline operator const D3D12_GRAPHICS_PIPELINE_STATE_DESC&(void) const {
            return this->_desc;
        }

        /// <summary>
        /// Provides access to the underlying descriptor object.
        /// </summary>
        inline operator D3D12_GRAPHICS_PIPELINE_STATE_DESC& (void) {
            return this->_desc;
        }

    private:

        std::vector<BYTE> _ds;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC _desc;
        std::vector<BYTE> _gs;
        std::vector<BYTE> _hs;
        std::vector<D3D12_INPUT_ELEMENT_DESC> _il;
        std::vector<BYTE> _ps;
        ATL::CComPtr<ID3D12RootSignature> _root_sig;
        std::vector<BYTE> _vs;
    };

} /* end namespace d3d12 */
} /* end namespace trrojan */
