﻿// <copyright file="volume_benchmark_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <array>
#include <limits>

#include "trrojan/camera.h"
#include "trrojan/datraw_base.h"

#include "trrojan/d3d12/benchmark_base.h"
#include "trrojan/d3d12/graphics_pipeline_builder.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Base class for D3D12 volume rendering benchmarks.
    /// </summary>
    class TRROJAND3D12_API volume_benchmark_base : public benchmark_base,
            public trrojan::datraw_base {

    public:

        typedef datraw_base::frame_type frame_type;
        typedef datraw_base::info_type info_type;
        typedef benchmark_base::manoeuvre_step_type manoeuvre_step_type;
        typedef benchmark_base::manoeuvre_type manoeuvre_type;
        typedef benchmark_base::point_type point_type;
        typedef datraw_base::reader_type reader_type;
        typedef float step_size_type;
        typedef benchmark_base::viewport_type viewport_type;

        static DXGI_FORMAT get_format(const info_type& info);

        static const char *factor_data_set;
        static const char *factor_ert_threshold;
        static const char *factor_frame;
        static const char *factor_fovy_deg;
        static const char *factor_gpu_counter_iterations;
        static const char *factor_max_steps;
        static const char *factor_min_prewarms;
        static const char *factor_min_wall_time;
        static const char *factor_step_size;
        static const char *factor_xfer_func;

        virtual ~volume_benchmark_base(void) = default;

    protected:

        static winrt::com_ptr<ID3D12Resource> load_brudervn_xfer_func(
            const std::string& path,
            d3d12::device& device,
            ID3D12GraphicsCommandList *cmd_list,
            const D3D12_RESOURCE_STATES state,
            winrt::com_ptr<ID3D12Resource>& out_staging);

        static winrt::com_ptr<ID3D12Resource> load_volume(
            const std::string& path,
            const frame_type frame,
            d3d12::device& device,
            ID3D12GraphicsCommandList *cmd_list,
            const D3D12_RESOURCE_STATES state,
            info_type& out_info,
            winrt::com_ptr<ID3D12Resource>& out_staging);

        static inline winrt::com_ptr<ID3D12Resource> load_volume(
                const configuration& config,
                d3d12::device& device,
                ID3D12GraphicsCommandList *cmd_list,
                const D3D12_RESOURCE_STATES state,
                info_type&out_info,
                winrt::com_ptr<ID3D12Resource>& out_staging) {
            return load_volume(config.get<std::string>(factor_data_set),
                config.get<unsigned int>(factor_frame),
                device, cmd_list, state, out_info, out_staging);
        }

        static winrt::com_ptr<ID3D12Resource> load_xfer_func(
            const std::vector<std::uint8_t>& data,
            d3d12::device& device,
            ID3D12GraphicsCommandList *cmd_list,
            const D3D12_RESOURCE_STATES state,
            winrt::com_ptr<ID3D12Resource>& out_staging);

        static winrt::com_ptr<ID3D12Resource> load_xfer_func(
            const std::string& path,
            d3d12::device& device,
            ID3D12GraphicsCommandList *cmd_list,
            const D3D12_RESOURCE_STATES state,
            winrt::com_ptr<ID3D12Resource>& out_staging);

        static winrt::com_ptr<ID3D12Resource> load_xfer_func(
            const configuration& config,
            d3d12::device& device,
            ID3D12GraphicsCommandList *cmd_list,
            const D3D12_RESOURCE_STATES state,
            winrt::com_ptr<ID3D12Resource>& out_staging);

        template<class TValue>
        static inline TValue zero_is_max(TValue& value) noexcept {
            if (value == static_cast<TValue>(0)) {
                value = (std::numeric_limits<TValue>::max)();
            }
            return value;
        }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="name"></param>
        volume_benchmark_base(const std::string& name);

        /// <summary>
        /// Recreates device-dependent resources.
        /// </summary>
        /// <param name="device"></param>
        virtual void on_device_switch(device& device) override;

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
        /// <param name="power_collector"></param>
        /// <param name="changed"></param>
        /// <returns><c>nullptr</c>.</returns>
        virtual trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            power_collector::pointer& power_collector,
            const std::vector<std::string>& changed);

        /// <summary>
        /// Creates shader resource views for the volume and the transfer
        /// function textures at the specified descriptor locations.
        /// </summary>
        /// <param name="handle_volume"></param>
        /// <param name="handle_xfer_func"></param>
        void set_textures(const D3D12_CPU_DESCRIPTOR_HANDLE handle_volume,
            const D3D12_CPU_DESCRIPTOR_HANDLE handle_xfer_func) const;

        /// <summary>
        /// The camera determining the view constants.
        /// </summary>
        /// <remarks>
        /// <see cref="volume_benchmark_base::on_run" /> will update the camera
        /// from the <see cref="configuration" /> passed to the method.
        /// </remarks>
        trrojan::perspective_camera _camera;

        /// <summary>
        /// The bounding box (begin and end, in this order) of the volume data
        /// set currently loaded.
        /// </summary>
        /// <remarks>
        /// <see cref="volume_benchmark_base::on_run" /> will update this
        /// information when loading a new <see cref="_volume_info" />.
        /// </remarks>
        std::array<glm::vec3, 2> _volume_bbox;

    private:

        winrt::com_ptr<ID3D12Resource> _tex_volume;
        winrt::com_ptr<ID3D12Resource> _tex_xfer_func;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
