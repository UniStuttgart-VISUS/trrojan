// <copyright file="cs_volume_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

//#include "trrojan/d3d12/rendering_technique.h"
#include "trrojan/d3d12/volume_benchmark_base.h"

#include "trrojan/d3d12/gpu_timer.h"

#include "SinglePassVolumePipeline.hlsli"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// A D3D12 volume renderer using a compute shader for traversing the rays.
    /// </summary>
    class TRROJAND3D12_API cs_volume_benchmark final
        : public volume_benchmark_base {

    public:

        typedef volume_benchmark_base::frame_type frame_type;
        typedef trrojan::d3d12::gpu_timer gpu_timer_type;
        typedef volume_benchmark_base::info_type info_type;
        typedef volume_benchmark_base::manoeuvre_step_type manoeuvre_step_type;
        typedef volume_benchmark_base::manoeuvre_type manoeuvre_type;
        typedef volume_benchmark_base::point_type point_type;
        typedef volume_benchmark_base::reader_type reader_type;
        typedef volume_benchmark_base::step_size_type step_size_type;
        typedef volume_benchmark_base::viewport_type viewport_type;

        cs_volume_benchmark(void);

        virtual ~cs_volume_benchmark(void) = default;

    protected:

        /// <inheritdoc />
        void on_device_switch(device &device) override;

        /// <inheritdoc />
        trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            power_collector::pointer& power_collector,
            const std::vector<std::string>& changed) override;

    private:

        void set_constants(const D3D12_CPU_DESCRIPTOR_HANDLE handle_view,
            const D3D12_CPU_DESCRIPTOR_HANDLE handle_ray) const;

        void set_descriptors(ID3D12GraphicsCommandList *cmd_list,
            const UINT frame) const;

        ATL::CComPtr<ID3D12Resource> _cb_ray;
        ATL::CComPtr<ID3D12PipelineState> _compute_pipeline;
        ATL::CComPtr<ID3D12RootSignature> _compute_signature;
        std::vector<std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> _descriptors;
        RaycastingConstants *_ray_constants;
        ATL::CComPtr<ID3D12Resource> _cb_view;
        std::vector<ATL::CComPtr<ID3D12Resource>> _uavs;
        ViewConstants *_view_constants;

        //rendering_technique::uav_type compute_target;
        //ATL::CComPtr<ID3D11Query> done_query;
        //rendering_technique::buffer_type raycasting_constants;
        //rendering_technique technique;
        //rendering_technique::buffer_type view_constants;

    };

} /* end namespace d3d12 */
} /* end namespace trrojan */
