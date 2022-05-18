// <copyright file="graphics_pipeline_builder.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/d3d12/plugin.h"
#include "trrojan/d3d12/utilities.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// A builder object for <see cref="D3D12_GRAPHICS_PIPELINE_STATE_DESC" />.
    /// </summary>
    class graphics_pipeline_builder final {

    public:

        /// <summary>
        /// Build a <see cref="ID3D12PipelineState" /> from the current state of
        /// the descriptor.
        /// </summary>
        /// <param name="device"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12PipelineState> build(ID3D12Device *device);

        inline graphics_pipeline_builder& set_domain_shader(const BYTE *byte_code,
                const std::size_t cnt_byte_code) {
            set_shader(this->_desc.DS, byte_code, cnt_byte_code);
            return *this;
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_domain_shader(
                const BYTE(&byte_code)[N]) {
            set_shader(this->_desc.DS, byte_code, N);
            return *this;
        }

        inline graphics_pipeline_builder& set_domain_shader(
                const std::vector<std::uint8_t>& byte_code) {
            set_shader(this->_desc.DS, byte_code.data(), byte_code.size());
            return *this;
        }

#if !defined(TRROJAN_FOR_UWP)
        inline graphics_pipeline_builder& set_domain_shader(LPCTSTR name,
                LPCTSTR type = _T("SHADER")) {
            return this->set_domain_shader(d3d12::plugin::load_resource(name,
                type));
        }
#endif /* !defined(TRROJAN_FOR_UWP) */

        inline graphics_pipeline_builder& set_geometry_shader(
                const BYTE *byte_code,
                const std::size_t cnt_byte_code) {
            set_shader(this->_desc.GS, byte_code, cnt_byte_code);
            return *this;
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_geometry_shader(
                const BYTE(&byte_code)[N]) {
            set_shader(this->_desc.GS, byte_code, N);
            return *this;
        }

        inline graphics_pipeline_builder& set_geometry_shader(
                const std::vector<std::uint8_t>& byte_code) {
            set_shader(this->_desc.GS, byte_code.data(), byte_code.size());
            return *this;
        }

#if !defined(TRROJAN_FOR_UWP)
        inline graphics_pipeline_builder& set_geometry_shader(LPCTSTR name,
                LPCTSTR type = _T("SHADER")) {
            return this->set_geometry_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(TRROJAN_FOR_UWP) */

        inline graphics_pipeline_builder& set_hull_shader(const BYTE *byte_code,
                const std::size_t cnt_byte_code) {
            set_shader(this->_desc.HS, byte_code, cnt_byte_code);
            return *this;
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_hull_shader(
                const BYTE(&byte_code)[N]) {
            set_shader(this->_desc.HS, byte_code, N);
            return *this;
        }

        inline graphics_pipeline_builder& set_hull_shader(
                const std::vector<std::uint8_t> &byte_code) {
            set_shader(this->_desc.HS, byte_code.data(), byte_code.size());
            return *this;
        }

#if !defined(TRROJAN_FOR_UWP)
        inline graphics_pipeline_builder& set_hull_shader(LPCTSTR name,
                LPCTSTR type = _T("SHADER")) {
            return this->set_hull_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(TRROJAN_FOR_UWP) */

        inline graphics_pipeline_builder& set_input_layout(
                const D3D12_INPUT_ELEMENT_DESC *elements,
                const std::size_t cnt) {
            this->_desc.InputLayout.pInputElementDescs = elements;
            this->_desc.InputLayout.NumElements = static_cast<UINT>(cnt);
            return *this;
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_input_layout(
                const D3D12_INPUT_ELEMENT_DESC(&elements)[N]) {
            return this->set_input_layout(elements, N);
        }

        inline graphics_pipeline_builder& set_input_layout(
                const std::vector<D3D12_INPUT_ELEMENT_DESC>& elements) {
            return this->set_input_layout(elements.data(), elements.size());
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_input_layout(
                const std::array<D3D12_INPUT_ELEMENT_DESC, N> &elements) {
            return this->set_input_layout(elements.data(), elements.size());
        }

        template<class... T>
        inline graphics_pipeline_builder& set_input_layout(T... elements) {
            std::array<D3D12_INPUT_ELEMENT_DESC, sizeof...(T)> elms
                = { elements... };
            return this->set_input_layout(elms);
        }

        inline graphics_pipeline_builder& set_pixel_shader(
                const BYTE *byte_code,
                const std::size_t cnt_byte_code) {
            set_shader(this->_desc.PS, byte_code, cnt_byte_code);
            return *this;
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_pixel_shader(
                const BYTE(&byte_code)[N]) {
            set_shader(this->_desc.PS, byte_code, N);
            return *this;
        }

        inline graphics_pipeline_builder& set_pixel_shader(
                const std::vector<std::uint8_t>& byte_code) {
            set_shader(this->_desc.PS, byte_code.data(), byte_code.size());
            return *this;
        }

#if !defined(TRROJAN_FOR_UWP)
        inline graphics_pipeline_builder& set_pixel_shader(LPCTSTR name,
                LPCTSTR type = _T("SHADER")) {
            return this->set_pixel_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(TRROJAN_FOR_UWP) */

        inline graphics_pipeline_builder& set_primitive_topology(
                const D3D12_PRIMITIVE_TOPOLOGY_TYPE topology) {
            this->_desc.PrimitiveTopologyType = topology;
            return *this;
        }

        inline graphics_pipeline_builder& set_vertex_shader(
                const BYTE *byte_code,
                const std::size_t cnt_byte_code) {
            set_shader(this->_desc.VS, byte_code, cnt_byte_code);
            return *this;
        }

        template<size_t N>
        inline graphics_pipeline_builder& set_vertex_shader(
                const BYTE(&byte_code)[N]) {
            set_shader(this->_desc.VS, byte_code, N);
            return *this;
        }

        inline graphics_pipeline_builder& set_vertex_shader(
                const std::vector<std::uint8_t> &byte_code) {
            set_shader(this->_desc.VS, byte_code.data(), byte_code.size());
            return *this;
        }

#if !defined(TRROJAN_FOR_UWP)
        inline graphics_pipeline_builder& set_vertex_shader(LPCTSTR name,
                LPCTSTR type = _T("SHADER")) {
            return this->set_vertex_shader(plugin::load_resource(name, type));
        }
#endif /* !defined(TRROJAN_FOR_UWP) */

        /// <summary>
        /// Converts the builder into the underlying descriptor object.
        /// </summary>
        /// <returns>The pipeline state descriptor being modified by the
        /// builder.</returns>
        inline operator const D3D12_GRAPHICS_PIPELINE_STATE_DESC&(void) const {
            return this->_desc;
        }

    private:

        D3D12_GRAPHICS_PIPELINE_STATE_DESC _desc;

    };

} /* end namespace d3d12 */
} /* end namespace trrojan */
