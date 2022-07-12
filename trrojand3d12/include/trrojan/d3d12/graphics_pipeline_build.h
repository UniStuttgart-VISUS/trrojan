// <copyright file="graphics_pipeline_build.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <tuple>
#include <type_traits>

#include "trrojan/tuple_cat_type.h"

#include "trrojan/d3d12/d3dx12.h"
#include "trrojan/d3d12/plugin.h"
#include "trrojan/d3d12/utilities.h"


namespace trrojan {
namespace d3d12 {
namespace detail {

    /// <summary>
    /// Defines the interface of a graphics pipeline description managed by the
    /// <see cref="graphics_pipeline_builder" />.
    /// </summary>
    class graphics_pipeline_build {

    public:

        /// <summary>
        /// The type of a pointer to this build.
        /// </summary>
        typedef std::unique_ptr<graphics_pipeline_build> pointer_type;

        /// <summary>
        /// Build a <see cref="ID3D12PipelineState" /> from the current state of
        /// the build.
        /// </summary>
        /// <param name="device">The device for which the pipeline state is
        /// intended for.</param>
        /// <returns>The pipeline state described by this build.</returns>
        /// <exception cref="ATL::CAtlException">If <paramref name="device" />
        /// is <c>nullptr</c> or if the pipline state could not be created.
        /// </exception>
        virtual ATL::CComPtr<ID3D12PipelineState> build(
            ID3D12Device2 *device) = 0;

        /// <summary>
        /// Get the <see cref="D3D12_DEPTH_STENCIL_DESC" /> from the object or
        /// create a new one having this object.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual D3D12_DEPTH_STENCIL_DESC& depth_stencil_desc(
            pointer_type& replacement) = 0;

        /// <summary>
        /// Gets the depth/stencil format from the object or create a new one
        /// that has this subobject.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual DXGI_FORMAT& depth_stencil_format(
            pointer_type& replacement) = 0;

        /// <summary>
        /// Gets or adds the domain shader.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual D3D12_SHADER_BYTECODE& domain_shader(
            pointer_type& replacement) = 0;

        /// <summary>
        /// Gets or adds the geometry shader.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual D3D12_SHADER_BYTECODE& geometry_shader(
            pointer_type& replacement) = 0;

        /// <summary>
        /// Gets or adds the hull shader.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual D3D12_SHADER_BYTECODE& hull_shader(
            pointer_type& replacement) = 0;

        /// <summary>
        /// Gets or add the input layout desciptor.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual D3D12_INPUT_LAYOUT_DESC& input_layout_desc(
            pointer_type& replacement) = 0;

        /// <summary>
        /// Gets or adds the pixel shader.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual D3D12_SHADER_BYTECODE& pixel_shader(
            pointer_type& replacement) = 0;

        /// <summary>
        /// Gets or adds the primitive topology.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE& primitive_topology(
            pointer_type &replacement) = 0;

        /// <summary>
        /// Gets or adds the render target formats.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual D3D12_RT_FORMAT_ARRAY& render_target_formats(
            pointer_type& replacement) = 0;

        /// <summary>
        /// Gets or adds the root signature.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual ID3D12RootSignature *& root_signature(
            pointer_type &replacement) = 0;

        /// <summary>
        /// Gets or adds the super-sampling descriptor.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual DXGI_SAMPLE_DESC& sample_desc(pointer_type& replacement) = 0;

        /// <summary>
        /// Gets or adds the vertex shader.
        /// </summary>
        /// <param name="replacement"></param>
        /// <returns></returns>
        virtual D3D12_SHADER_BYTECODE& vertex_shader(
            pointer_type& replacement) = 0;
    };


    /// <summary>
    /// Implementation of <see cref="graphics_pipeline_build" />.
    /// </summary>
    /// <typeparam name="TSubobjects"></typeparam>
    template<class... TSubobjects>
    class tgraphics_pipeline_build final : public graphics_pipeline_build {

    public:

        /// <inheritdoc />
        ATL::CComPtr<ID3D12PipelineState> build(ID3D12Device2 *device) override;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline tgraphics_pipeline_build(void) { }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline explicit tgraphics_pipeline_build(
                std::tuple<TSubobjects...>&& subobjects)
            : _subobjects(std::move(subobjects)) { }

        /// <inheritdoc />
        D3D12_DEPTH_STENCIL_DESC& depth_stencil_desc(
            pointer_type& replacement) override;

        /// <inheritdoc />
        DXGI_FORMAT& depth_stencil_format(
            pointer_type& replacement) override;

        /// <inheritdoc />
        D3D12_SHADER_BYTECODE& domain_shader(
            pointer_type& replacement) override;

        /// <inheritdoc />
        D3D12_SHADER_BYTECODE& geometry_shader(
            pointer_type& replacement) override;

        /// <summary>
        /// Get (and optionally add it before) the subobject of the specified type.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="replacement"></param>
        template<class T> T& get(pointer_type& replacement);

        /// <inheritdoc />
        D3D12_SHADER_BYTECODE& hull_shader(
            pointer_type& replacement) override;

        /// <inheritdoc />
        D3D12_INPUT_LAYOUT_DESC& input_layout_desc(
            pointer_type &replacement) override;

        /// <inheritdoc />
        D3D12_SHADER_BYTECODE& pixel_shader(
            pointer_type& replacement) override;

        /// <inheritdoc />
        D3D12_PRIMITIVE_TOPOLOGY_TYPE& primitive_topology(
            pointer_type& replacement) override;

        /// <inheritdoc />
        D3D12_RT_FORMAT_ARRAY& render_target_formats(
            pointer_type& replacement) override;

        /// <inheritdoc />
        ID3D12RootSignature *& root_signature(
            pointer_type& replacement) override;

        /// <inheritdoc />
        DXGI_SAMPLE_DESC& sample_desc(pointer_type& replacement) override;

        /// <inheritdoc />
        D3D12_SHADER_BYTECODE& vertex_shader(
            pointer_type& replacement) override;

        /// <summary>
        /// The subobject list passed to <see cref="build" />.
        /// </summary>
        std::tuple<TSubobjects...> _subobjects;
    };

} /* end namespace detail */
} /* end namespace d3d12 */
} /* end namespace trrojan */

#include "trrojan/d3d12/graphics_pipeline_build.inl"
