/// <copyright file="rendering_technique.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <functional>
#include <map>
#include <stdexcept>
#include <vector>

#include <atlbase.h>
#include <Windows.h>
#include <d3d11.h>

#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Encapsulates the graphics resources required for a specific rendering
    /// technique.
    /// </summary>
    class TRROJAND3D11_API rendering_technique {

    public:

        /// <summary>
        /// Groups resources which should be bound to a shader stage.
        /// </summary>
        struct shader_resources {
            std::vector<ATL::CComPtr<ID3D11Buffer>> constant_buffers;
            std::vector<ATL::CComPtr<ID3D11ShaderResourceView>> resource_views;
        };

        /// <summary>
        /// Identifies a shader stage (mostly for resource binding).
        /// </summary>
        /// <remarks>
        /// The individual constants are bitmasks which can be combined if
        /// necessary.
        /// </remarks>
        enum class shader_stage {
            vertex = 0x0001,
            hull = 0x0002,
            domain = 0x0004,
            geometry = 0x0008,
            pixel = 0x0010
        };

        typedef std::underlying_type<shader_stage>::type shader_stages;

        /// <summary>
        /// Activates the technique by setting the shaders and resources
        /// required for rendering.
        /// </summary>
        /// <param name="ctx"></param>
        void apply(ID3D11DeviceContext *ctx);

        /// <summary>
        /// Get, if used, the domain shader for the rendering technique.
        /// </summary>
        /// <returns>A pointer to the shader or <c>nullptr</c> if no shader is
        /// required.</returns>
        inline ATL::CComPtr<ID3D11DomainShader> domain_shader(void) {
            return this->domainShader;
        }

        /// <summary>
        /// Get, if used, the geometry shader for the rendering technique.
        /// </summary>
        /// <returns>A pointer to the shader or <c>nullptr</c> if no shader is
        /// required.</returns>
        inline ATL::CComPtr<ID3D11GeometryShader> geometry_shader(void) {
            return this->geometryShader;
        }

        /// <summary>
        /// Get, if used, the hull shader for the rendering technique.
        /// </summary>
        /// <returns>A pointer to the shader or <c>nullptr</c> if no shader is
        /// required.</returns>
        inline ATL::CComPtr<ID3D11HullShader> hull_shader(void) {
            return this->hullShader;
        }

        /// <summary>
        /// Gets the vertex input layout required for the technique.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<ID3D11InputLayout> input_layout(void) {
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
        inline ATL::CComPtr<ID3D11PixelShader> pixel_shader(void) {
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
        /// Add or replace the constant buffers starting at index
        /// <paramref name="start" /> in the given stages.
        /// </summary>
        template<class... T>
        void set_constant_buffers(const shader_stages stages, const UINT start,
            T... buffers);

        /// <summary>
        /// Add or replace the shader resource views starting at index
        /// <paramref name="start" /> in the given stages.
        /// </summary>
        template<class... T>
        void set_shader_resource_views(const shader_stages stages,
            const UINT start, T... views);

        //void set_vertex_buffers();

        /// <summary>
        /// Gets the vertex buffers to be bound.
        /// </summary>
        /// <returns></returns>
        inline const std::vector<ATL::CComPtr<ID3D11Buffer>>& vertex_buffers(
                void) const {
            return this->vertexBuffers;
        }

        /// <summary>
        /// Get, if used, the vertex shader for the rendering technique.
        /// </summary>
        /// <returns>A pointer to the shader or <c>nullptr</c> if no shader is
        /// required.</returns>
        inline ATL::CComPtr<ID3D11VertexShader> vertex_shader(void) {
            return this->vertexShader;
        }

    private:

        template<class T>
        static std::vector<T *> unsmart(std::vector<ATL::CComPtr<T>>& input);

        ATL::CComPtr<ID3D11DomainShader> domainShader;
        ATL::CComPtr<ID3D11GeometryShader> geometryShader;
        ATL::CComPtr<ID3D11HullShader> hullShader;
        ATL::CComPtr<ID3D11InputLayout> inputLayout;
        std::string _name;
        ATL::CComPtr<ID3D11PixelShader> pixelShader;
        D3D11_PRIMITIVE_TOPOLOGY primitiveTopology;
        std::map<shader_stage, shader_resources> _resources;
        std::vector<ATL::CComPtr<ID3D11Buffer>> vertexBuffers;
        ATL::CComPtr<ID3D11VertexShader> vertexShader;

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */

#include "trrojan/d3d11/rendering_technique.inl"
