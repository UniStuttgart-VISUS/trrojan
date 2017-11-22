/// <copyright file="benchmark_base.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <Windows.h>
#include <d3d11.h>

#include "trrojan/benchmark.h"

#include "trrojan/d3d11/debug_render_target.h"
#include "trrojan/d3d11/debugable.h"
#include "trrojan/d3d11/device.h"
#include "trrojan/d3d11/hash.h"
#include "trrojan/d3d11/render_target.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Base class for D3D11 benchmarks.
    /// </summary>
    class TRROJAND3D11_API benchmark_base : public trrojan::benchmark_base,
            public debugable {

    public:

        /// <summary>
        /// The type to specify the <see cref="factor_viewport" /> factor.
        /// </summary>
        typedef std::array<std::uint32_t, 2> viewport_type;

        /// <summary>
        /// Boolean factor enabling the debug view.
        /// </summary>
        static const std::string factor_debug_view;

        /// <summary>
        /// The name of the viewport factor.
        /// </summary>
        static const std::string factor_viewport;

        virtual ~benchmark_base(void);

        virtual bool can_run(trrojan::environment env,
            trrojan::device device) const noexcept;

        virtual void draw_debug_view(ATL::CComPtr<ID3D11Device> device,
            ATL::CComPtr<ID3D11DeviceContext> deviceContext);

        virtual void on_debug_view_resized(ATL::CComPtr<ID3D11Device> device,
            const unsigned int width, const unsigned int height);

        virtual void on_debug_view_resizing(void);

        virtual trrojan::result run(const configuration& c);

    protected:

        static bool contains(const std::string& needle,
            const std::vector<std::string>& haystack);

        static ATL::CComPtr<ID3D11Buffer> create_buffer(d3d11::device& device,
            const D3D11_USAGE usage, const D3D11_BIND_FLAG binding,
            const void *data, const UINT cntData, const UINT cpuAccess = 0);

        template<size_t N>
        static ATL::CComPtr<ID3D11DomainShader> create_domain_shader(
            d3d11::device& device, const BYTE(&byteCode)[N]);

        template<size_t N>
        static ATL::CComPtr<ID3D11GeometryShader> create_geometry_shader(
            d3d11::device& device, const BYTE(&byteCode)[N]);

        template<size_t N>
        static ATL::CComPtr<ID3D11HullShader> create_hull_shader(
            d3d11::device& device, const BYTE(&byteCode)[N]);

        template<size_t N>
        static ATL::CComPtr<ID3D11InputLayout> create_input_layout(
            d3d11::device& device,
            const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements,
            const BYTE(&byteCode)[N]);

        template<size_t N>
        static ATL::CComPtr<ID3D11PixelShader> create_pixel_shader(
            d3d11::device& device, const BYTE(&byteCode)[N]);

        template<size_t N>
        static ATL::CComPtr<ID3D11VertexShader> create_vertex_shader(
            d3d11::device& device, const BYTE(& byteCode)[N]);

        benchmark_base(const std::string& name);

        /// <summary>
        /// Performs the actual test on behalf of the <see cref="run" /> method.
        /// </summary>
        /// <param name="device">The device to use. It is guaranteed that the
        /// device is obtained from <paramref name="config" />.</param>
        /// <param name="config">The configuration to run.</param>
        /// <param name="changed">The names of the factors that have been
        /// changed since the last test run.</param>
        /// <returns>The test results.</returns>
        virtual trrojan::result on_run(d3d11::device& device,
            const configuration& config,
            const std::vector<std::string>& changed) = 0;

    private:

        typedef trrojan::benchmark_base base;

        render_target benchTarget;

        std::shared_ptr<debug_render_target> debugTarget;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */

#include "trrojan/d3d11/benchmark_base.inl"
