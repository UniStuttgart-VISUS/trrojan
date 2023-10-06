// <copyright file="volume_benchmark_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <limits>

#include "trrojan/camera.h"

#include "trrojan/d3d12/benchmark_base.h"
#include "trrojan/d3d12/graphics_pipeline_builder.h"
#include "trrojan/d3d12/volume_rendering_configuration.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Base class for D3D12 volume rendering benchmarks.
    /// </summary>
    class TRROJAND3D12_API volume_benchmark_base : public benchmark_base,
            public trrojan::datraw_base {

    public:

        typedef volume_rendering_configuration::frame_type frame_type;
        typedef datraw_base::info_type info_type;
        typedef benchmark_base::manoeuvre_step_type manoeuvre_step_type;
        typedef benchmark_base::manoeuvre_type manoeuvre_type;
        typedef benchmark_base::point_type point_type;
        typedef datraw_base::reader_type reader_type;
        typedef volume_rendering_configuration::step_size_type step_size_type;
        typedef benchmark_base::viewport_type viewport_type;

        static DXGI_FORMAT get_format(const info_type& info);

        virtual ~volume_benchmark_base(void) = default;

    protected:

        static ATL::CComPtr<ID3D12Resource> load_brudervn_xfer_func(
            const std::string& path,
            d3d12::device& device,
            ID3D12GraphicsCommandList *cmd_list,
            const D3D12_RESOURCE_STATES state);

        static ATL::CComPtr<ID3D12Resource> load_volume(
            const std::string& path,
            const frame_type frame,
            d3d12::device& device,
            ID3D12GraphicsCommandList *cmd_list,
            const D3D12_RESOURCE_STATES state,
            info_type& outInfo);

        static inline ATL::CComPtr<ID3D12Resource> load_volume(
                const volume_rendering_configuration& config,
                d3d12::device& device,
                ID3D12GraphicsCommandList *cmd_list,
                const D3D12_RESOURCE_STATES state,
                info_type& outInfo) {
            return load_volume(config.data_set(), config.frame(),
                device, cmd_list, state, outInfo);
        }

        static ATL::CComPtr<ID3D12Resource> load_xfer_func(
            const std::vector<std::uint8_t>& data,
            d3d12::device& device,
            ID3D12GraphicsCommandList *cmd_list,
            const D3D12_RESOURCE_STATES state);

        static ATL::CComPtr<ID3D12Resource> load_xfer_func(
            const std::string& path,
            d3d12::device& device,
            ID3D12GraphicsCommandList *cmd_list,
            const D3D12_RESOURCE_STATES state);

        static ATL::CComPtr<ID3D12Resource> load_xfer_func(
            const volume_rendering_configuration& config,
            d3d12::device& device,
            ID3D12GraphicsCommandList *cmd_list,
            const D3D12_RESOURCE_STATES state);

        template<class T> static inline T zero_is_max(T& value) {
            if (value == static_cast<T>(0)) {
                value = (std::numeric_limits<T>::max)();
            }
            return value;
        }

        volume_benchmark_base(const std::string& name);

        /// <summary>
        /// Prepares the data set and the transfer function stored in
        /// <see cref="data_view" /> and <see cref="xfer_func_view" />.
        /// </summary>
        /// <remarks>
        /// Subclasses should call this implementation before they perform their
        /// actual work and try to access the views provided by this base class.
        /// </remarks>
        /// <param name="device"></param>
        /// <param name="config"></param>
        /// <param name="changed"></param>
        /// <returns><c>nullptr</c>.</returns>
        virtual trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            const std::vector<std::string>& changed);

        trrojan::perspective_camera camera;
        //ATL::CComPtr<ID3D11ShaderResourceView> data_view;
        //ATL::CComPtr<ID3D11SamplerState> linear_sampler;
        //ATL::CComPtr<ID3D11ShaderResourceView> xfer_func_view;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
