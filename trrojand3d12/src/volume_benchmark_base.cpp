// <copyright file="volume_benchmark_base.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/volume_benchmark_base.h"

#include <cassert>
#include <stdexcept>

#include "trrojan/brudervn_xfer_func.h"
#include "trrojan/clipping.h"
#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/text.h"

#include "trrojan/d3d12/utilities.h"


#define _VOLUME_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d12::volume_benchmark_base::factor_##f = #f

_VOLUME_BENCH_DEFINE_FACTOR(data_set);
_VOLUME_BENCH_DEFINE_FACTOR(ert_threshold);
_VOLUME_BENCH_DEFINE_FACTOR(frame);
_VOLUME_BENCH_DEFINE_FACTOR(fovy_deg);
_VOLUME_BENCH_DEFINE_FACTOR(gpu_counter_iterations);
_VOLUME_BENCH_DEFINE_FACTOR(max_steps);
_VOLUME_BENCH_DEFINE_FACTOR(min_prewarms);
_VOLUME_BENCH_DEFINE_FACTOR(min_wall_time);
_VOLUME_BENCH_DEFINE_FACTOR(step_size);
_VOLUME_BENCH_DEFINE_FACTOR(xfer_func);

#undef _VOLUME_BENCH_DEFINE_FACTOR


/*
 * trrojan::d3d12::volume_benchmark_base::get_format
 */
DXGI_FORMAT trrojan::d3d12::volume_benchmark_base::get_format(
        const info_type& info) {
    // Note: this is identical with the D3D11 case.
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


/*
 * trrojan::d3d12::volume_benchmark_base::load_brudervn_xfer_func
 */
ATL::CComPtr<ID3D12Resource>
trrojan::d3d12::volume_benchmark_base::load_brudervn_xfer_func(
        const std::string& path,
        d3d12::device& device,
        ID3D12GraphicsCommandList *cmd_list,
        const D3D12_RESOURCE_STATES state) {
    log::instance().write_line(log_level::debug, "Loading transfer function "
        "from {} ...", path);
    const auto data = trrojan::load_brudervn_xfer_func(path);
    return load_xfer_func(data, device, cmd_list, state);
}


/*
 * trrojan::d3d12::volume_benchmark_base::load_volume
 */
ATL::CComPtr<ID3D12Resource>
trrojan::d3d12::volume_benchmark_base::load_volume(
        const std::string& path,
        const frame_type frame,
        d3d12::device& device,
        ID3D12GraphicsCommandList *cmd_list,
        const D3D12_RESOURCE_STATES state,
        info_type& outInfo) {
    log::instance().write_line(log_level::debug, "Loading volume data "
        "from {} ...", path);
    auto reader = reader_type::open(path);

    if (!reader.move_to(frame)) {
        throw std::invalid_argument("The given frame number does not exist.");
    }

    auto resolution = reader.info().resolution();
    if (resolution.size() != 3) {
        throw std::invalid_argument("The given data set is not a 3D volume.");
    }

    if (cmd_list == nullptr) {
        throw std::invalid_argument("A valid command list for uploading the "
            "volume data set must be provided.");
    }

    // Stage the volume.
    auto data = reader.read_current();
    auto buffer = create_upload_buffer(device.d3d_device(), data.size());
    set_debug_object_name(buffer.p, "volume_staging");
    stage_data(buffer, data.data(), data.size());

    // Copy the data from the staging buffer to the texture.
    const auto format = get_format(reader.info());
    auto retval = create_texture(device.d3d_device(), resolution[0],
        resolution[1], resolution[2], format);
    set_debug_object_name(retval.p, "xfer_func");

    auto src_loc = get_copy_location(buffer);
    assert(src_loc.PlacedFootprint.Footprint.Format == DXGI_FORMAT_UNKNOWN);
    src_loc.PlacedFootprint.Footprint.Format = format;
    src_loc.PlacedFootprint.Footprint.Width = reader.info().element_size();
    auto dst_loc = get_copy_location(retval);

    cmd_list->CopyTextureRegion(&dst_loc, 0, 0, 0, &src_loc, nullptr);
    transition_resource(cmd_list, retval, D3D12_RESOURCE_STATE_COPY_DEST,
        state);

    // Return the info block in case of success.
    outInfo = reader.info();

    return retval;
}


/*
 * trrojan::d3d12::volume_benchmark_base::load_xfer_func
 */
ATL::CComPtr<ID3D12Resource>
trrojan::d3d12::volume_benchmark_base::load_xfer_func(
        const std::vector<std::uint8_t>& data,
        d3d12::device& device,
        ID3D12GraphicsCommandList *cmd_list,
        const D3D12_RESOURCE_STATES state) {
    const auto cnt = std::div(static_cast<long>(data.size()), 4l);
    const auto format = DXGI_FORMAT_R8G8B8A8_UNORM;

    if (cnt.rem != 0) {
        throw std::invalid_argument("The transfer function texture does not "
            "hold valid data in DXGI_FORMAT_R8G8B8A8_UNORM.");
    }

    if (cmd_list == nullptr) {
        throw std::invalid_argument("A valid command list for uploading the "
            "transfer function must be provided.");
    }

    // Create a staging buffer for uploading the data.
    auto buffer = create_upload_buffer(device.d3d_device(), data.size());
    set_debug_object_name(buffer.p, "xfer_func_staging");
    stage_data(buffer, data.data(), data.size());

    // Copy the data from the staging buffer to the texture.
    auto retval = create_texture(device.d3d_device(), cnt.quot, format);
    set_debug_object_name(retval.p, "xfer_func");

    auto src_loc = get_copy_location(buffer);
    assert(src_loc.PlacedFootprint.Footprint.Format == DXGI_FORMAT_UNKNOWN);
    src_loc.PlacedFootprint.Footprint.Format = format;
    src_loc.PlacedFootprint.Footprint.Width /= 4;
    auto dst_loc = get_copy_location(retval);

    cmd_list->CopyTextureRegion(&dst_loc, 0, 0, 0, &src_loc, nullptr);
    transition_resource(cmd_list, retval, D3D12_RESOURCE_STATE_COPY_DEST,
        state);

    return retval;
}


/*
 * trrojan::d3d12::volume_benchmark_base::load_xfer_func
 */
ATL::CComPtr<ID3D12Resource>
trrojan::d3d12::volume_benchmark_base::load_xfer_func(
        const std::string &path,
        d3d12::device& device,
        ID3D12GraphicsCommandList *cmd_list,
        const D3D12_RESOURCE_STATES state) {
    log::instance().write_line(log_level::debug, "Loading transfer function "
        "from {} ...", path);
    const auto data = read_binary_file(path);
    return load_xfer_func(data, device, cmd_list, state);
}


/*
 * trrojan::d3d12::volume_benchmark_base::load_xfer_func
 */
ATL::CComPtr<ID3D12Resource>
trrojan::d3d12::volume_benchmark_base::load_xfer_func(
        const configuration& config,
        d3d12::device& device,
        ID3D12GraphicsCommandList *cmd_list,
        const D3D12_RESOURCE_STATES state) {
    try {
        auto path = config.get<std::string>(factor_xfer_func);

        if (ends_with(path, std::string(".brudervn"))) {
            return volume_benchmark_base::load_brudervn_xfer_func(
                path, device, cmd_list, state);
        } else {
            return volume_benchmark_base::load_xfer_func(
                path, device, cmd_list, state);
        }

    } catch (...) {
        log::instance().write_line(log_level::debug, "Creating linear transfer "
            "function as fallback solution.");
        std::vector<std::uint8_t> data(256 * 4);
        for (std::uint8_t i = 0; i < data.size(); i += 4) {
            data[static_cast<std::size_t>(i) * 4 + 0] = i;
            data[static_cast<std::size_t>(i) * 4 + 1] = i;
            data[static_cast<std::size_t>(i) * 4 + 2] = i;
            data[static_cast<std::size_t>(i) * 4 + 3] = i;
        }

        return volume_benchmark_base::load_xfer_func(
            data, device, cmd_list, state);
    }
}


/*
 * trrojan::d3d12::volume_benchmark_base::volume_benchmark_base
 */
trrojan::d3d12::volume_benchmark_base::volume_benchmark_base(
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

    this->add_default_manoeuvre();
}


/*
 * trrojan::d3d12::volume_benchmark_base::on_device_switch
 */
void trrojan::d3d12::volume_benchmark_base::on_device_switch(device& device) {
    benchmark_base::on_device_switch(device);
    this->_tex_volume = nullptr;
    this->_tex_xfer_func = nullptr;
}


/*
 * trrojan::d3d12::volume_benchmark_base::on_run
 */
trrojan::result trrojan::d3d12::volume_benchmark_base::on_run(
        d3d12::device& device,
        const configuration& config,
        const std::vector<std::string>& changed) {
    // Clear all resource that have been invalidated by the change of factors.
    if (contains_any(changed, factor_device, factor_data_set, factor_frame)) {
        this->_tex_volume = nullptr;
    }
    if (contains_any(changed, factor_device, factor_xfer_func)) {
        this->_tex_xfer_func = nullptr;
    }

    // Create all resources managed by the base class that are invalid.
    {
        auto cmd_list = this->create_graphics_command_list_for(
            this->_tex_volume,
            this->_tex_xfer_func);

        if (this->_tex_volume == nullptr) {
            this->_tex_volume = this->load_volume(config, device,
                cmd_list, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE,
                this->_volume_info);
            this->calc_bounding_box(this->_volume_bbox.front(),
                this->_volume_bbox.back());
        }

        if (this->_tex_xfer_func == nullptr) {
            this->_tex_xfer_func = this->load_xfer_func(config, device,
                cmd_list, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
        }

        if (cmd_list != nullptr) {
            device.close_and_execute_command_list(cmd_list);
            device.wait_for_gpu();
        }
    }

    // Update the camera from the configuration.
    this->set_aspect_from_viewport(this->_camera);
    this->_camera.set_fovy(config.get<float>(factor_fovy_deg));
    graphics_benchmark_base::apply_manoeuvre(this->_camera, config,
        this->_volume_bbox.front(), this->_volume_bbox.back());
    trrojan::set_clipping_planes(this->_camera, this->_volume_bbox);

    return trrojan::result();
}
