// <copyright file="volume_benchmark_base.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d11/volume_benchmark_base.h"

#include <stdexcept>

#include "trrojan/brudervn_xfer_func.h"
#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/text.h"

#include "trrojan/d3d11/utilities.h"


/*
 * trrojan::d3d11::volume_benchmark_base::get_format
 */
DXGI_FORMAT trrojan::d3d11::volume_benchmark_base::get_format(
        const info_type& info) {
    auto resolution = info.resolution();
    if (resolution.size() != 3) {
        throw std::invalid_argument("The given data set is not a 3D volume.");
    }

    auto components = info.components();
    if ((components < 1) || (components > 4)) {
        throw std::invalid_argument("The number of per-voxel components of the "
            "given data set is not within [1, 4]");
    }

    switch (info.format()) {
        case datraw::scalar_type::int8:
            switch (components) {
                case 1: return DXGI_FORMAT_R8_SNORM;
                case 2: return DXGI_FORMAT_R8G8_SNORM;
                case 4: return DXGI_FORMAT_R8G8B8A8_SNORM;
            }
            break;

        case datraw::scalar_type::int16:
            switch (components) {
                case 1: return DXGI_FORMAT_R16_SNORM;
                case 2: return DXGI_FORMAT_R16G16_SNORM;
                case 4: return DXGI_FORMAT_R16G16B16A16_SNORM;
            }
            break;

        case datraw::scalar_type::int32:
            switch (components) {
                case 1: return DXGI_FORMAT_R32_SINT;
                case 2: return DXGI_FORMAT_R32G32_SINT;
                case 3: return DXGI_FORMAT_R32G32B32_SINT;
                case 4: return DXGI_FORMAT_R32G32B32A32_SINT;
            }
            break;

        case datraw::scalar_type::uint8:
            switch (components) {
                case 1: return DXGI_FORMAT_R8_UNORM;
                case 2: return DXGI_FORMAT_R8G8_UNORM;
                case 4: return DXGI_FORMAT_R8G8B8A8_UNORM;
            }
            break;

        case datraw::scalar_type::uint16:
            switch (components) {
                case 1: return DXGI_FORMAT_R16_UNORM;
                case 2: return DXGI_FORMAT_R16G16_UNORM;
                case 4: return DXGI_FORMAT_R16G16B16A16_UNORM;
            }
            break;

        case datraw::scalar_type::uint32:
            switch (components) {
                case 1: return DXGI_FORMAT_R32_UINT;
                case 2: return DXGI_FORMAT_R32G32_UINT;
                case 3: return DXGI_FORMAT_R32G32B32_UINT;
                case 4: return DXGI_FORMAT_R32G32B32A32_UINT;
            }
            break;

        case datraw::scalar_type::float16:
            switch (components) {
                case 1: return DXGI_FORMAT_R16_FLOAT;
                case 2: return DXGI_FORMAT_R16G16_FLOAT;
                case 4: return DXGI_FORMAT_R16G16B16A16_FLOAT;
            }
            break;

        case datraw::scalar_type::float32:
            switch (components) {
                case 1: return DXGI_FORMAT_R32_FLOAT;
                case 2: return DXGI_FORMAT_R32G32_FLOAT;
                case 3: return DXGI_FORMAT_R32G32B32_FLOAT;
                case 4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
            break;
    }

    throw std::invalid_argument("The given scalar data type is unknown or "
        "unsupported.");
}


#define _VOL_BENCH_DEFINE_FACTOR(f)                                            \
const char *trrojan::d3d11::volume_benchmark_base::factor_##f = #f

_VOL_BENCH_DEFINE_FACTOR(data_set);
_VOL_BENCH_DEFINE_FACTOR(ert_threshold);
_VOL_BENCH_DEFINE_FACTOR(frame);
_VOL_BENCH_DEFINE_FACTOR(fovy_deg);
_VOL_BENCH_DEFINE_FACTOR(gpu_counter_iterations);
_VOL_BENCH_DEFINE_FACTOR(max_steps);
_VOL_BENCH_DEFINE_FACTOR(min_prewarms);
_VOL_BENCH_DEFINE_FACTOR(min_wall_time);
_VOL_BENCH_DEFINE_FACTOR(prewarm_precision);
_VOL_BENCH_DEFINE_FACTOR(step_size);
_VOL_BENCH_DEFINE_FACTOR(xfer_func);


/*
 * trrojan::d3d11::volume_benchmark_base::load_brudervn_xfer_func
 */
void trrojan::d3d11::volume_benchmark_base::load_brudervn_xfer_func(
        const char *path, ID3D11Device *device, ID3D11Texture1D **outTexture,
        ID3D11ShaderResourceView **outSrv) {
    auto data = trrojan::load_brudervn_xfer_func(path);
    load_xfer_func(data, device, outTexture, outSrv);
}


/*
 * trrojan::d3d11::volume_benchmark_base::load_volume
 */
void trrojan::d3d11::volume_benchmark_base::load_volume(const char *path,
        const frame_type frame, ID3D11Device *device, info_type& outInfo,
        ID3D11Texture3D **outTexture, ID3D11ShaderResourceView **outSrv) {
    auto reader = reader_type::open(path);

    if (outTexture == nullptr) {
        throw std::invalid_argument("'outTexture' must not be nullptr.");
    }

    if (!reader.move_to(frame)) {
        throw std::invalid_argument("The given frame number does not exist.");
    }

    auto resolution = reader.info().resolution();
    if (resolution.size() != 3) {
        throw std::invalid_argument("The given data set is not a 3D volume.");
    }

    {
        // Load the volume and create an immutable texture.
        auto data = reader.read_current();

        D3D11_TEXTURE3D_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.Format = volume_benchmark_base::get_format(reader.info());
        desc.MipLevels = 1;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.Width = resolution[0];
        desc.Height = resolution[1];
        desc.Depth = resolution[2];

        D3D11_SUBRESOURCE_DATA id;
        ::ZeroMemory(&id, sizeof(id));
        id.pSysMem = data.data();
        id.SysMemPitch = resolution[0];
        id.SysMemSlicePitch = id.SysMemPitch * resolution[1];

        auto hr = device->CreateTexture3D(&desc, &id, outTexture);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    if (outSrv != nullptr) {
        // If requested by the user, directly create a shader resource view for
        // the volume.
        auto hr = device->CreateShaderResourceView(*outTexture, nullptr,
            outSrv);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    // Return the info block in case of success.
    outInfo = reader.info();
}


/*
 * trrojan::d3d11::volume_benchmark_base::load_xfer_func
 */
void trrojan::d3d11::volume_benchmark_base::load_xfer_func(
        const std::vector<std::uint8_t>& data, ID3D11Device *device,
        ID3D11Texture1D **outTexture, ID3D11ShaderResourceView **outSrv) {
    auto cnt = std::div(static_cast<long>(data.size()), 4l);

    if (cnt.rem != 0) {
        throw std::invalid_argument("The transfer function texture does not "
            "hold valid data in DXGI_FORMAT_R8G8B8A8_UNORM.");
    }

    {
        D3D11_TEXTURE1D_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.ArraySize = 1;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.MipLevels = 1;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.Width = static_cast<UINT>(cnt.quot);

        D3D11_SUBRESOURCE_DATA id;
        ::ZeroMemory(&id, sizeof(id));
        id.pSysMem = data.data();

        auto hr = device->CreateTexture1D(&desc, &id, outTexture);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
        set_debug_object_name(*outTexture, "volume_data_set");
    }

    if (outSrv != nullptr) {
        // If requested by the user, directly create a shader resource view for
        // the transfer function.
        auto hr = device->CreateShaderResourceView(*outTexture, nullptr,
            outSrv);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }
}


/*
 * trrojan::d3d11::volume_benchmark_base::load_xfer_func
 */
void trrojan::d3d11::volume_benchmark_base::load_xfer_func(const char *path,
        ID3D11Device *device, ID3D11Texture1D **outTexture,
        ID3D11ShaderResourceView **outSrv) {
    auto data = read_binary_file(path);
    load_xfer_func(data, device, outTexture, outSrv);
}


/*
 * trrojan::d3d11::volume_benchmark_base::load_xfer_func
 */
void trrojan::d3d11::volume_benchmark_base::load_xfer_func(
        const configuration& config, d3d11::device& device,
        ID3D11Texture1D **outTexture, ID3D11ShaderResourceView **outSrv) {
    try {
        auto path = config.get<std::string>(factor_xfer_func);

        if (ends_with(path, std::string(".brudervn"))) {
            return volume_benchmark_base::load_brudervn_xfer_func(path.c_str(),
                device.d3d_device(), outTexture, outSrv);
        } else {
            return volume_benchmark_base::load_xfer_func(path.c_str(),
                device.d3d_device(), outTexture, outSrv);
        }

    } catch (...) {
        // Fall back to a fully linear transfer function.
        auto dev = device.d3d_device();
        std::array<std::uint32_t, 256> linearXfer;

        {
            std::uint32_t i = 0;
            std::generate(linearXfer.begin(), linearXfer.end(), [&i]() {
                auto retval = (i << 24) | (i << 16) | (i << 8) | i;
                ++i;
                return retval;
            });
        }

        {
            D3D11_TEXTURE1D_DESC desc;
            ::ZeroMemory(&desc, sizeof(desc));
            desc.ArraySize = 1;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.MipLevels = 1;
            desc.Usage = D3D11_USAGE_IMMUTABLE;
            desc.Width = static_cast<UINT>(linearXfer.size());

            D3D11_SUBRESOURCE_DATA id;
            ::ZeroMemory(&id, sizeof(id));
            id.pSysMem = linearXfer.data();

            auto hr = dev->CreateTexture1D(&desc, &id, outTexture);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
            set_debug_object_name(*outTexture, "transfer_function");
        }

        if (outSrv != nullptr) {
            auto hr = dev->CreateShaderResourceView(*outTexture, nullptr,
                outSrv);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }
    }
}


/*
 * trrojan::d3d11::volume_benchmark_base::volume_benchmark_base
 */
trrojan::d3d11::volume_benchmark_base::volume_benchmark_base(
        const std::string& name) : benchmark_base(name) {
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_ert_threshold, 0.0f));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_frame, static_cast<frame_type>(0)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_fovy_deg, 60.0f));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_gpu_counter_iterations, static_cast<unsigned int>(7)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_step_size, static_cast<step_size_type>(1)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_max_steps, static_cast<unsigned int>(0)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_min_prewarms, static_cast<unsigned int>(4)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_min_wall_time, static_cast<unsigned int>(1000)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_prewarm_precision, 1.0f));

    this->add_default_manoeuvre();
}


/*
 * trrojan::d3d11::volume_benchmark_base::on_run
 */
trrojan::result trrojan::d3d11::volume_benchmark_base::on_run(
        d3d11::device& device, const configuration& config,
        const std::vector<std::string>& changed) {
    // If the device has changed, invalidate the data as well. Data-independent
    // resources can be re-created immediately.
    if (contains(changed, factor_device)) {
        // Constant buffers.
        //this->sphere_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
        //    D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(SphereConstants));
        //set_debug_object_name(this->sphere_constants.p, "sphere_constants");

        // Textures and SRVs.
        this->data_view = nullptr;
        this->xfer_func_view = nullptr;

        // Samplers.
        this->linear_sampler = create_linear_sampler(device.d3d_device());

        // Queries.
        //this->done_query = create_event_query(dev);
        //this->stats_query = create_pipline_stats_query(dev);
    }

    // Invalidate data set if it has changed.
    if (contains_any(changed, factor_data_set, factor_frame)) {
        this->data_view = nullptr;
    }

    // Invalidate transfer function if it has changed.
    if (contains(changed, factor_xfer_func)) {
        this->xfer_func_view = nullptr;
    }

    // Recreate resources.
    if (this->data_view == nullptr) {
        ATL::CComPtr<ID3D11Texture3D> tex;
        volume_benchmark_base::load_volume(config, device, this->_volume_info,
            &tex, &this->data_view);
    }

    if (this->xfer_func_view == nullptr) {
        ATL::CComPtr<ID3D11Texture1D> tex;
        volume_benchmark_base::load_xfer_func(config, device, &tex,
            &this->xfer_func_view);
    }

    return trrojan::result();
}


/*
 * trrojan::d3d11::volume_benchmark_base::optimise_order
 */
void trrojan::d3d11::volume_benchmark_base::optimise_order(
        configuration_set& inOutConfs) {
    inOutConfs.optimise_order({
        factor_data_set,
        factor_xfer_func,
        factor_device
    });
}
