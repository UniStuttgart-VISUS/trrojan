/// <copyright file="volume_benchmark_base.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <limits>

#include "trrojan/camera.h"
#include "trrojan/datraw_base.h"

#include "trrojan/d3d11/benchmark_base.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Base class for D3D11 volume rendering benchmarks.
    /// </summary>
    class TRROJAND3D11_API volume_benchmark_base : public benchmark_base,
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

        static void load_volume(const char *path, const frame_type frame,
            ID3D11Device *device, info_type& outInfo,
            ID3D11Texture3D **outTexture,
            ID3D11ShaderResourceView **outSrv = nullptr);

        static inline void load_volume(const configuration& config,
                d3d11::device& device, info_type& outInfo,
                ID3D11Texture3D **outTexture,
                ID3D11ShaderResourceView **outSrv = nullptr) {
            auto path = config.get<std::string>(factor_data_set);
            auto frame = config.get<frame_type>(factor_frame);
            return volume_benchmark_base::load_volume(path.c_str(), frame,
                device.d3d_device(), outInfo, outTexture, outSrv);
        }

        static void load_xfer_func(const char *path, ID3D11Device *device,
            ID3D11Texture1D **outTexture,
            ID3D11ShaderResourceView **outSrv = nullptr);

        static void load_xfer_func(const configuration& config,
            d3d11::device& device, ID3D11Texture1D **outTexture,
            ID3D11ShaderResourceView **outSrv = nullptr);

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
        virtual trrojan::result on_run(d3d11::device& device,
            const configuration& config,
            const std::vector<std::string>& changed);

        trrojan::perspective_camera camera;
        ATL::CComPtr<ID3D11ShaderResourceView> data_view;
        ATL::CComPtr<ID3D11SamplerState> linear_sampler;
        ATL::CComPtr<ID3D11ShaderResourceView> xfer_func_view;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
