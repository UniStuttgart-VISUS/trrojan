// <copyright file="rendering_technique.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <functional>
#include <map>
#include <stdexcept>
#include <vector>

#include <Windows.h>
#include <d3d11.h>

#include <winrt/base.h>

#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Encapsulates the graphics resources required for a specific rendering
    /// technique.
    /// </summary>
    class TRROJAND3D11_API rendering_technique {

    public:

        typedef winrt::com_ptr<ID3D11BlendState> blend_state_type;
        typedef winrt::com_ptr<ID3D11Buffer> buffer_type;
        typedef winrt::com_ptr<ID3D11ComputeShader> compute_shader_type;
        typedef winrt::com_ptr<ID3D11DepthStencilState> depth_state_type;
        typedef winrt::com_ptr<ID3D11DomainShader> domain_shader_type;
        typedef winrt::com_ptr<ID3D11GeometryShader> geometry_shader_type;
        typedef winrt::com_ptr<ID3D11HullShader> hull_shader_type;
        typedef winrt::com_ptr<ID3D11InputLayout> input_layout_type;
        typedef winrt::com_ptr<ID3D11PixelShader> pixel_shader_type;
        typedef winrt::com_ptr<ID3D11RasterizerState> rasteriser_state_type;
        typedef winrt::com_ptr<ID3D11SamplerState> sampler_state_type;
        typedef winrt::com_ptr<ID3D11ShaderResourceView> srv_type;
        typedef winrt::com_ptr<ID3D11UnorderedAccessView> uav_type;
        typedef winrt::com_ptr<ID3D11VertexShader> vertex_shader_type;

        /// <summary>
        /// Groups all information to bind and index buffer.
        /// </summary>
        struct index_buffer {
            buffer_type buffer;
            DXGI_FORMAT format;
            UINT offset;

            inline index_buffer(buffer_type buffer = nullptr,
                const DXGI_FORMAT format = DXGI_FORMAT_R16_UINT,
                const UINT offset = 0)
                : buffer(buffer), format(format), offset(offset) { }
        };

        /// <summary>
        /// Groups resources which should be bound to a shader stage.
        /// </summary>
        struct shader_resources {
            std::vector<buffer_type> constant_buffers;
            std::vector<srv_type> resource_views;
            std::vector<sampler_state_type> sampler_states;
            std::vector<uav_type> uavs;
        };

        /// <summary>
        /// Identifies a shader stage (mostly for resource binding).
        /// </summary>
        /// <remarks>
        /// <para>The individual constants are bitmasks which can be combined if
        /// necessary.</para>
        /// <para>Implementation note: please check correctness of
        /// <see cref="rendering_technique::foreach_stage" /> when changing the
        /// enumeration!</para>
        /// </remarks>
        enum class shader_stage {
            vertex = 0x0001,
            hull = 0x0002,
            domain = 0x0004,
            geometry = 0x0008,
            pixel = 0x0010,
            compute = 0x0020
        };

        /// <summary>
        /// A bitmask that identifies different versions of the same technique,
        /// e.g. using different input layouts.
        /// </summary>
        typedef std::uint32_t version_type;

        /// <summary>
        /// A list of different versions of the same technique, which can be
        /// identified by a <see cref="version_type" /> bitmask.
        /// </summary>
        typedef std::map<version_type, rendering_technique> version_list;

        /// <summary>
        /// Groups all information to emit a vertex buffer.
        /// </summary>
        struct vertex_buffer {
            buffer_type buffer;
            UINT offset;
            UINT size;
            UINT stride;

            inline vertex_buffer(buffer_type buffer = nullptr,
                const UINT size = 0,
                const UINT stride = 0,
                const UINT offset = 0)
                : buffer(buffer), offset(offset), size(size), stride(stride) { }
        };

        /// <summary>
        /// A combination of multiple <see cref="shader_stage" />s.
        /// </summary>
        typedef std::underlying_type<shader_stage>::type shader_stages;

        /// <summary>
        /// Combine several shader stages into a bitmask.
        /// </summary>
        template<class... T>
        static shader_stages combine_shader_stages(T... args);

        /// <summary>
        /// Set the shader resource view at the given index.
        /// </summary>
        static void set_shader_resource_view(shader_resources& res,
            srv_type srv, const UINT idx);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(void);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(const std::string& name);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(const std::string& name,
            std::vector<vertex_buffer>&& vb,
            winrt::com_ptr<ID3D11InputLayout> il,
            const D3D11_PRIMITIVE_TOPOLOGY pt,
            winrt::com_ptr<ID3D11VertexShader> vs, shader_resources&& vsRes,
            winrt::com_ptr<ID3D11HullShader> hs, shader_resources&& hsRes,
            winrt::com_ptr<ID3D11DomainShader> ds, shader_resources&& dsRes,
            winrt::com_ptr<ID3D11GeometryShader> gs, shader_resources&& gsRes,
            winrt::com_ptr<ID3D11PixelShader> ps, shader_resources&& psRes);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(const std::string& name, const vertex_buffer& vb,
            winrt::com_ptr<ID3D11InputLayout> il,
            const D3D11_PRIMITIVE_TOPOLOGY pt,
            winrt::com_ptr<ID3D11VertexShader> vs, shader_resources&& vsRes,
            winrt::com_ptr<ID3D11HullShader> hs, shader_resources&& hsRes,
            winrt::com_ptr<ID3D11DomainShader> ds, shader_resources&& dsRes,
            winrt::com_ptr<ID3D11GeometryShader> gs, shader_resources&& gsRes,
            winrt::com_ptr<ID3D11PixelShader> ps, shader_resources&& psRes);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(const std::string& name,
            winrt::com_ptr<ID3D11InputLayout> il,
            const D3D11_PRIMITIVE_TOPOLOGY pt,
            winrt::com_ptr<ID3D11VertexShader> vs, shader_resources&& vsRes,
            winrt::com_ptr<ID3D11HullShader> hs, shader_resources&& hsRes,
            winrt::com_ptr<ID3D11DomainShader> ds, shader_resources&& dsRes,
            winrt::com_ptr<ID3D11GeometryShader> gs, shader_resources&& gsRes,
            winrt::com_ptr<ID3D11PixelShader> ps, shader_resources&& psRes);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(const std::string& name,
            winrt::com_ptr<ID3D11InputLayout> il,
            const D3D11_PRIMITIVE_TOPOLOGY pt,
            winrt::com_ptr<ID3D11VertexShader> vs, shader_resources&& vsRes,
            winrt::com_ptr<ID3D11PixelShader> ps, shader_resources&& psRes);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(const std::string& name,
            std::vector<vertex_buffer>&& vb,
            winrt::com_ptr<ID3D11InputLayout> il,
            const D3D11_PRIMITIVE_TOPOLOGY pt,
            winrt::com_ptr<ID3D11VertexShader> vs, shader_resources&& vsRes,
            winrt::com_ptr<ID3D11PixelShader> ps, shader_resources&& psRes);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(const std::string& name,
            std::vector<vertex_buffer>&& vb, index_buffer&& ib,
            winrt::com_ptr<ID3D11InputLayout> il,
            const D3D11_PRIMITIVE_TOPOLOGY pt,
            winrt::com_ptr<ID3D11VertexShader> vs, shader_resources&& vsRes,
            winrt::com_ptr<ID3D11PixelShader> ps, shader_resources&& psRes);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(const std::string& name,
            winrt::com_ptr<ID3D11InputLayout> il,
            const D3D11_PRIMITIVE_TOPOLOGY pt,
            winrt::com_ptr<ID3D11VertexShader> vs, shader_resources&& vsRes,
            winrt::com_ptr<ID3D11GeometryShader> gs, shader_resources&& gsRes,
            winrt::com_ptr<ID3D11PixelShader> ps, shader_resources&& psRes);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(const std::string& name,
            winrt::com_ptr<ID3D11InputLayout> il,
            const D3D11_PRIMITIVE_TOPOLOGY pt,
            winrt::com_ptr<ID3D11VertexShader> vs, shader_resources&& vsRes,
            winrt::com_ptr<ID3D11HullShader> hs, shader_resources&& hsRes,
            winrt::com_ptr<ID3D11DomainShader> ds, shader_resources&& dsRes,
            winrt::com_ptr<ID3D11PixelShader> ps, shader_resources&& psRes);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        rendering_technique(const std::string& name,
            winrt::com_ptr<ID3D11ComputeShader> cs, shader_resources&& csRes);

        /// <summary>
        /// Finalise the instance.
        /// </summary>
        ~rendering_technique(void) = default;

        /// <summary>
        /// Activates the technique by setting the shaders and resources
        /// required for rendering.
        /// </summary>
        /// <param name="ctx"></param>
        void apply(winrt::com_ptr<ID3D11DeviceContext> ctx);

        /// <summary>
        /// Get, if used, the compute shader for the rendering technique.
        /// </summary>
        /// <returns>A pointer to the shader or <c>nullptr</c> if no shader is
        /// required.</returns>
        inline compute_shader_type compute_shader(void) {
            return this->computeShader;
        }

        /// <summary>
        /// Get, if used, the domain shader for the rendering technique.
        /// </summary>
        /// <returns>A pointer to the shader or <c>nullptr</c> if no shader is
        /// required.</returns>
        inline domain_shader_type domain_shader(void) {
            return this->domainShader;
        }

        /// <summary>
        /// Get, if used, the geometry shader for the rendering technique.
        /// </summary>
        /// <returns>A pointer to the shader or <c>nullptr</c> if no shader is
        /// required.</returns>
        inline geometry_shader_type geometry_shader(void) {
            return this->geometryShader;
        }

        /// <summary>
        /// Get, if used, the hull shader for the rendering technique.
        /// </summary>
        /// <returns>A pointer to the shader or <c>nullptr</c> if no shader is
        /// required.</returns>
        inline hull_shader_type hull_shader(void) {
            return this->hullShader;
        }

        /// <summary>
        /// Gets the vertex input layout required for the technique.
        /// </summary>
        /// <returns></returns>
        inline input_layout_type input_layout(void) {
            return this->inputLayout;
        }

        /// <summary>
        /// Gets a human-redable name of the rendering technique.
        /// </summary>
        /// <returns></returns>
        inline const std::string& name(void) const {
            return this->_name;
        }

        /// <summary>
        /// Get, if used, the pixel shader for the rendering technique.
        /// </summary>
        /// <returns>A pointer to the shader or <c>nullptr</c> if no shader is
        /// required.</returns>
        inline pixel_shader_type pixel_shader(void) {
            return this->pixelShader;
        }

        /// <summary>
        /// Gets the primitive topology used for the technique,
        /// </summary>
        /// <returns></returns>
        inline D3D11_PRIMITIVE_TOPOLOGY primitve_topology(void) {
            return this->primitiveTopology;
        }

        /// <summary>
        /// Gets a map of shader resources required to be bound to the different
        /// shader stages.
        /// </summary>
        /// <returns></returns>
        inline const std::map<shader_stage, shader_resources>& resources(
                void) const {
            return this->_resources;
        }

        /// <summary>
        /// Sets a blend state that is activated with the technique.
        /// </summary>
        inline void set_blend_state(const blend_state_type& blendState) {
            this->blendState = blendState;
        }

        /// <summary>
        /// Add or replace the constant buffers starting at index
        /// <paramref name="start" /> in the given stages.
        /// </summary>
        void set_constant_buffers(const std::vector<buffer_type>& buffers,
            const shader_stages stages, const UINT start = 0);

        /// <summary>
        /// Add or replace the constant buffers starting at index
        /// <paramref name="start" /> in the given stages.
        /// </summary>
        void set_constant_buffers(const buffer_type buffer,
            const shader_stages stages, const UINT start = 0);

        //// <summary>
        /// Set a specific depth/stencil state to apply.
        //// </summary>
        void set_depth_stencil_state(depth_state_type& depthStencilState) {
            this->depthStencilState = depthStencilState;
        }

        //// <summary>
        /// Set a specific rasteriser state to apply.
        //// </summary>
        void set_rasteriser_state(rasteriser_state_type& rasteriserState) {
            this->rasteriserState = rasteriserState;
        }

        /// <summary>
        /// Add or replace the shader resource views starting at index
        /// <paramref name="start" /> in the given stages.
        /// </summary>
        void set_shader_resource_views(const std::vector<srv_type> &srvs,
            const shader_stages stages, const UINT start = 0);

        /// <summary>
        /// Add or replace the shader resource views starting at index
        /// <paramref name="start" /> in the given stages.
        /// </summary>
        void set_shader_resource_views(const srv_type& srv,
            const shader_stages stages, const UINT start = 0);

        /// <summary>
        /// Add or replace the vertex buffers starting at index
        /// <paramref name="start" />.
        /// <summary>
        void set_vertex_buffers(const std::vector<vertex_buffer>& vbs,
            const UINT start = 0);

        /// <summary>
        /// Add or replace the vertex buffers starting at index
        /// <paramref name="start" />.
        /// <summary>
        void set_vertex_buffers(const vertex_buffer& vb,
            const UINT start = 0);

        /// <summary>
        /// Add or replace the UAVs starting at index
        /// <paramref name="start" /> in the given stages.
        /// </summary>
        void set_uavs(const std::vector<uav_type> &uavs,
            const shader_stages stages, const UINT start = 0);

        /// <summary>
        /// Add or replace the UAVs starting at index
        /// <paramref name="start" /> in the given stages.
        /// </summary>
        void set_uavs(const uav_type& uav,
            const shader_stages stages, const UINT start = 0);

        /// <summary>
        /// Gets the vertex buffers to be bound.
        /// </summary>
        /// <returns></returns>
        inline const std::vector<vertex_buffer>& vertex_buffers(void) const {
            return this->vertexBuffers;
        }

        /// <summary>
        /// Get, if used, the vertex shader for the rendering technique.
        /// </summary>
        /// <returns>A pointer to the shader or <c>nullptr</c> if no shader is
        /// required.</returns>
        inline vertex_shader_type vertex_shader(void) {
            return this->vertexShader;
        }

    private:

        /// <summary>
        /// Ensure that <paramref name="dst" /> has at least so many elements
        /// that <paramref name="src" /> can be copied starting at index
        /// <paramre fname="offset" />.
        /// </summary>
        template<class T>
        static void assert_range(std::vector<T>& dst, const std::vector<T>& src,
            const size_t offset);

        /// <summary>
        /// Ensure that <paramref name="dst" /> has at least so many elements
        /// that one element can be copied starting at index
        /// <paramre fname="offset" />.
        /// </summary>
        template<class T>
        static void assert_range(std::vector<T>& dst, const size_t offset);

        /// <summary>
        /// Converts a vector of smart pointers into dumb pointers without
        /// increasing the reference count.
        /// </summary>
        template<class T>
        static std::vector<T *> unsmart(std::vector<winrt::com_ptr<T>>& input);

        /// <summary>
        /// Invokes <paramref name="action" /> for all stages marked in
        /// <paramref name="stages" />.
        /// </summary>
        void foreach_stage(const shader_stages stages,
            const std::function<void(shader_resources&)>& action);

        blend_state_type blendState;
        compute_shader_type computeShader;
        depth_state_type depthStencilState;
        domain_shader_type domainShader;
        geometry_shader_type geometryShader;
        hull_shader_type hullShader;
        index_buffer indexBuffer;
        input_layout_type inputLayout;
        std::string _name;
        pixel_shader_type pixelShader;
        D3D11_PRIMITIVE_TOPOLOGY primitiveTopology;
        rasteriser_state_type rasteriserState;
        std::map<shader_stage, shader_resources> _resources;
        std::vector<vertex_buffer> vertexBuffers;
        vertex_shader_type vertexShader;

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */

#include "trrojan/d3d11/rendering_technique.inl"
