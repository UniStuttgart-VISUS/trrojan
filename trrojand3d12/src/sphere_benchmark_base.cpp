// <copyright file="sphere_benchmark_base.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_benchmark_base.h"

#include <algorithm>
#include <cinttypes>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "trrojan/clipping.h"
#include "trrojan/constants.h"
#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/log.h"
#include "trrojan/mmpld_reader.h"
#include "trrojan/result.h"
#include "trrojan/system_factors.h"
#include "trrojan/timer.h"

#include "trrojan/d3d12/plugin.h"
#include "trrojan/d3d12/sphere_benchmark.h"
#include "trrojan/d3d12/utilities.h"

#include "sphere_techniques.h"
#include "SpherePipeline.hlsli"



/*
 * trrojan::d3d12::sphere_benchmark_base::optimise_order
 */
void trrojan::d3d12::sphere_benchmark_base::optimise_order(
        configuration_set& inOutConfs) {
    inOutConfs.optimise_order({ 
        sphere_rendering_configuration::factor_data_set,
        sphere_rendering_configuration::factor_frame,
        benchmark_base::factor_device });
}


/*
 * trrojan::d3d12::sphere_benchmark_base::required_factors
 */
std::vector<std::string> trrojan::d3d12::sphere_benchmark_base::required_factors(
        void) const {
    static const std::vector<std::string> retval = {
        sphere_rendering_configuration::factor_data_set,
        benchmark_base::factor_device
    };
    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_primitive_topology
 */
D3D12_PRIMITIVE_TOPOLOGY
trrojan::d3d12::sphere_benchmark_base::get_primitive_topology(
        const shader_id_type shader_code) {
    const auto is_geo = ((shader_code & SPHERE_TECHNIQUE_USE_GEO) != 0);
    const auto is_tess = ((shader_code & SPHERE_TECHNIQUE_USE_TESS) != 0);

    if (is_geo) {
        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    }

    if (is_tess) {
        return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
    }

    if (is_technique(shader_code, SPHERE_TECHNIQUE_QUAD_INST)) {
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    }

    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_primitive_topology_type
 */
D3D12_PRIMITIVE_TOPOLOGY_TYPE
trrojan::d3d12::sphere_benchmark_base::get_primitive_topology_type(
        const shader_id_type shader_code) {
    const auto is_geo = ((shader_code & SPHERE_TECHNIQUE_USE_GEO) != 0);
    const auto is_tess = ((shader_code & SPHERE_TECHNIQUE_USE_TESS) != 0);

    if (is_geo) {
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    }

    if (is_tess) {
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    }

    if (is_technique(shader_code, SPHERE_TECHNIQUE_QUAD_INST)) {
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    }

    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_descriptors
 */
void trrojan::d3d12::sphere_benchmark_base::set_descriptors(
        ID3D12GraphicsCommandList *cmd_list,
        const descriptor_table_type& descriptors) {
    assert(cmd_list != nullptr);
    cmd_list->SetDescriptorHeaps(1, &descriptors.first.p);

    for (UINT i = 0; i < descriptors.second.size(); ++i) {
        cmd_list->SetGraphicsRootDescriptorTable(i,
            descriptors.second[i]);
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_shaders
 */
void trrojan::d3d12::sphere_benchmark_base::set_shaders(
        graphics_pipeline_builder& builder, const shader_id_type shader_id) {
    //const auto is_col = ((shader_id & SPHERE_INPUT_PV_COLOUR) != 0);
    builder.reset_shaders();
#if defined(TRROJAN_FOR_UWP)
    _LOOKUP_SPHERE_SHADER_FILES(builder, shader_id,
        sphere_benchmark_base::resolve_shader_path);
#else /* defined(TRROJAN_FOR_UWP) */
    _LOOKUP_SPHERE_SHADER_RESOURCES(builder, shader_id);
#endif /* defined(TRROJAN_FOR_UWP) */
}


/*
 * trrojan::d3d12::sphere_benchmark_base::sphere_benchmark_base
 */
trrojan::d3d12::sphere_benchmark_base::sphere_benchmark_base(
        const std::string& name)
    : benchmark_base(name), _cnt_descriptor_tables(0),
        _sphere_constants(nullptr), _tessellation_constants(nullptr),
        _view_constants(nullptr) {
    // Declare the configuration data we need to have.
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_adapt_tess_maximum,
        static_cast<unsigned int>(8)));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_adapt_tess_minimum,
        static_cast<unsigned int>(4)));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_adapt_tess_scale,
        2.0f));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_conservative_depth,
        false));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_edge_tess_factor, {
            sphere_rendering_configuration::edge_tess_factor_type { 4, 4, 4, 4}
        }));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_fit_bounding_box,
        false));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_force_float_colour,
        false));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_frame,
        static_cast<sphere_rendering_configuration::frame_type>(0)));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_gpu_counter_iterations,
        static_cast<unsigned int>(7)));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_hemi_tess_scale,
        0.5f));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_inside_tess_factor, { 
            sphere_rendering_configuration::inside_tess_factor_type { 4, 4 }
        }));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_method,
        sphere_rendering_configuration::get_methods()));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_min_prewarms,
        static_cast<unsigned int>(4)));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_min_wall_time,
        static_cast<unsigned int>(1000)));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_poly_corners,
        4u));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_vs_raygen,
        false));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_rendering_configuration::factor_vs_xfer_function,
        false));

    this->add_default_manoeuvre();
}


/*
 * trrojan::d3d12::sphere_benchmark_base::clear_stale_data
 */
void trrojan::d3d12::sphere_benchmark_base::clear_stale_data(
        const std::vector<std::string>& changed) {
    if (contains_any(changed,
            sphere_rendering_configuration::factor_data_set,
            sphere_rendering_configuration::factor_frame,
            sphere_rendering_configuration::factor_force_float_colour,
            sphere_rendering_configuration::factor_fit_bounding_box)) {
        this->_data.clear();
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_clipping_planes
 */
void trrojan::d3d12::sphere_benchmark_base::configure_camera(
        const configuration& config, const float fovy) {
    this->set_aspect_from_viewport(this->_camera);
    this->_camera.set_fovy(fovy);
    graphics_benchmark_base::apply_manoeuvre(this->_camera, config,
        this->_data.bbox_start(), this->_data.bbox_end());
    trrojan::set_clipping_planes(this->_camera, this->_data.bbox(),
        this->_data.max_radius());
}


/*
 * trrojan::d3d12::sphere_benchmark_base::count_descriptor_tables
 */
UINT trrojan::d3d12::sphere_benchmark_base::count_descriptor_tables(
        const shader_id_type shader_code, const bool include_root) const {
    // Three constant buffers are always required.
    UINT retval = 3;

    if (is_any_technique(shader_code, SPHERE_INPUT_PV_INTENSITY
            | SPHERE_TECHNIQUE_USE_INSTANCING)) {
        // Note: we use a little hack that the instancing data are always in t1,
        // even if there is no colour map bound to t0. This makes the allocation
        // here a bit easier as it is always the same size ...
        retval += 2;
    }

    if (is_any_technique(shader_code, SPHERE_INPUT_PP_INTENSITY)) {
        ++retval;
    }

    if (include_root) {
        // Add one slot for the root descriptor table.
        ++retval;
    }

    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::create_colour_map_view
 */
void trrojan::d3d12::sphere_benchmark_base::create_colour_map_view(
        ID3D12Device *device, const D3D12_CPU_DESCRIPTOR_HANDLE handle) {
    assert(device != nullptr);
    assert(this->_colour_map);
    device->CreateShaderResourceView(this->_colour_map, nullptr, handle);
}


/*
 * trrojan::d3d12::sphere_benchmark_base::create_constant_buffer_view
 */
void trrojan::d3d12::sphere_benchmark_base::create_constant_buffer_view(
        ID3D12Device *device, const UINT buffer,
        const D3D12_CPU_DESCRIPTOR_HANDLE sphere_constants,
        const D3D12_CPU_DESCRIPTOR_HANDLE view_constants,
        const D3D12_CPU_DESCRIPTOR_HANDLE tessellation_constants) {
    assert(device != nullptr);

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferLocation = this->get_sphere_constants(buffer);
        desc.SizeInBytes = align_constant_buffer_size(sizeof(SphereConstants));
        device->CreateConstantBufferView(&desc, sphere_constants);
    }

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferLocation = this->get_tessellation_constants(buffer);
        desc.SizeInBytes = align_constant_buffer_size(
            sizeof(TessellationConstants));
        device->CreateConstantBufferView(&desc, tessellation_constants);
    }

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferLocation = this->get_view_constants(buffer);
        desc.SizeInBytes = align_constant_buffer_size(sizeof(ViewConstants));
        device->CreateConstantBufferView(&desc, view_constants);
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::create_descriptor_heaps
 */
void trrojan::d3d12::sphere_benchmark_base::create_descriptor_heaps(
        ID3D12Device *device, const shader_id_type shader_code) {
    this->create_descriptor_heaps(device, this->_cnt_descriptor_tables
        = this->count_descriptor_tables(shader_code, true)); // SFX ;-)
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_pipeline_builder
 */
trrojan::d3d12::graphics_pipeline_builder
trrojan::d3d12::sphere_benchmark_base::get_pipeline_builder(
        const shader_id_type shader_code) {
    const auto is_flt = ((shader_code & SPHERE_INPUT_FLT_COLOUR) != 0);
    const auto is_geo = ((shader_code & SPHERE_TECHNIQUE_USE_GEO) != 0);
    const auto is_inst = ((shader_code & SPHERE_TECHNIQUE_USE_INSTANCING) != 0);
    const auto is_ps_tex = ((shader_code & SPHERE_INPUT_PP_INTENSITY) != 0);
    const auto is_ray = ((shader_code & SPHERE_TECHNIQUE_USE_RAYCASTING) != 0);
    const auto is_srv = ((shader_code & SPHERE_TECHNIQUE_USE_SRV) != 0);
    const auto is_tess = ((shader_code & SPHERE_TECHNIQUE_USE_TESS) != 0);
    const auto is_vs_tex = ((shader_code & SPHERE_INPUT_PV_INTENSITY) != 0);

    graphics_pipeline_builder retval;

    // Set the shaders from the big generated lookup table.
    set_shaders(retval, shader_code);

    // Set the input layout for techniques using VBs.
    if (!is_srv) {
        retval.set_input_layout(this->_data.input_layout());
    }

    retval.set_depth_stencil_format(DXGI_FORMAT_D32_FLOAT)
        .set_depth_state(true)
//.set_two_sided(true)// TODO
        .set_primitive_topology(get_primitive_topology_type(shader_code))
        .set_render_targets(DXGI_FORMAT_R8G8B8A8_UNORM)
        .set_sample_desc();

    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_pipeline_state
 */
ATL::CComPtr<ID3D12PipelineState>
trrojan::d3d12::sphere_benchmark_base::get_pipeline_state(ID3D12Device *device,
        const shader_id_type shader_code) {
    assert(device != nullptr);
    auto it = this->_pipeline_cache.find(shader_code);
    auto is_create = (it == this->_pipeline_cache.end());

    if (!is_create) {
        // If the device was switched, we need to recreate the pipeline state
        // on the new device.
        auto existing_device = get_device(it->second);
        is_create = (existing_device != device);
    }

    if (is_create) {
        log::instance().write_line(log_level::debug, "Building pipeline state "
            "for shader code 0x{:x} ...", shader_code);
        auto builder = this->get_pipeline_builder(shader_code);
        auto retval = this->_pipeline_cache[shader_code] = builder.build(device);
        set_debug_object_name(retval, "Pipeline state \"0x{:x}\"", shader_code);
        return retval;

    } else {
        return it->second;
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_root_signature
 */
ATL::CComPtr<ID3D12RootSignature>
trrojan::d3d12::sphere_benchmark_base::get_root_signature(ID3D12Device *device,
        const shader_id_type shader_code) {
    assert(device != nullptr);
    auto it = this->_root_sig_cache.find(shader_code);
    auto is_create = (it == this->_root_sig_cache.end());

    if (!is_create) {
        // If the device was switched, we need to recreate the root signature
        // on the new device.
        auto existing_device = get_device(it->second);
        is_create = (existing_device != device);
    }

    if (is_create) {
        auto builder = this->get_pipeline_builder(shader_code);
        auto retval = this->_root_sig_cache[shader_code]
            = graphics_pipeline_builder::root_signature_from_shader(device,
                builder);
        set_debug_object_name(retval, "Root signature \"0x{:x}\"", shader_code);
        return retval;

    } else {
        return it->second;
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_sphere_constants
 */
D3D12_GPU_VIRTUAL_ADDRESS
trrojan::d3d12::sphere_benchmark_base::get_sphere_constants(
        const UINT buffer) const {
    assert(this->_constant_buffer != nullptr);
    auto retval = this->_constant_buffer->GetGPUVirtualAddress();
    retval = offset_by_n<SphereConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffer);
    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_tessellation_constants
 */
D3D12_GPU_VIRTUAL_ADDRESS
trrojan::d3d12::sphere_benchmark_base::get_tessellation_constants(
        const UINT buffer) const {
    assert(this->_constant_buffer != nullptr);
    const auto buffers = this->pipeline_depth();
    auto retval = this->_constant_buffer->GetGPUVirtualAddress();

    // Skip all sphere and view constants.
    retval = offset_by_n<SphereConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffers);
    retval = offset_by_n<ViewConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffers);

    retval = offset_by_n<TessellationConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffer);

    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_view_constants
 */
void trrojan::d3d12::sphere_benchmark_base::get_view_constants(
        ViewConstants& out_constants) const {
    auto projection = this->_camera.calc_projection_mxz0();
    out_constants.ProjMatrix[0] = DirectX::XMFLOAT4X4(
        glm::value_ptr(projection));

    auto& view = this->_camera.get_view_mx();
    out_constants.ViewMatrix[0] = DirectX::XMFLOAT4X4(
        glm::value_ptr(view));

    auto viewInv = glm::inverse(view);
    out_constants.ViewInvMatrix[0] = DirectX::XMFLOAT4X4(
        glm::value_ptr(viewInv));

    auto viewProj = projection * view;
    out_constants.ViewProjMatrix[0] = DirectX::XMFLOAT4X4(
        glm::value_ptr(viewProj));

    auto viewProjInv = glm::inverse(viewProj);
    out_constants.ViewProjInvMatrix[0] = DirectX::XMFLOAT4X4(
        glm::value_ptr(viewProjInv));

    const auto& viewport = this->viewport();
    out_constants.Viewport.x = viewport.TopLeftX;
    out_constants.Viewport.y = viewport.TopLeftY;
    out_constants.Viewport.z = viewport.Width;
    out_constants.Viewport.w = viewport.Height;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_view_constants
 */
D3D12_GPU_VIRTUAL_ADDRESS
trrojan::d3d12::sphere_benchmark_base::get_view_constants(
        const UINT buffer) const {
    assert(this->_constant_buffer != nullptr);
    const auto buffers = this->pipeline_depth();
    auto retval = this->_constant_buffer->GetGPUVirtualAddress();

    // Skip all SphereConstants.
    retval = offset_by_n<SphereConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffers);

    retval = offset_by_n<ViewConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffer);

    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::on_device_switch
 */
void trrojan::d3d12::sphere_benchmark_base::on_device_switch(device& device) {
    assert(device.d3d_device() != nullptr);
    static constexpr auto CONSTANT_BUFFER_SIZE
        = align_constant_buffer_size(sizeof(SphereConstants))
        + align_constant_buffer_size(sizeof(ViewConstants))
        + align_constant_buffer_size(sizeof(TessellationConstants));
    benchmark_base::on_device_switch(device);

    // PSOs and root sigs are device-specific, so clear all cached ones.
    this->_pipeline_cache.clear();
    this->_root_sig_cache.clear();

    // Resources are device-specific, so delete and recreate them.
    {
        auto cmd_list = this->create_graphics_command_list();
        set_debug_object_name(cmd_list, "Viridis initialisation command list");
        this->_colour_map = create_viridis_colour_map(device, cmd_list);
    }

    this->_constant_buffer = create_constant_buffer(device.d3d_device(),
        this->pipeline_depth() * CONSTANT_BUFFER_SIZE);

    // Persistently map the upload buffer for constants.
    {
        void *p;
        auto hr = this->_constant_buffer->Map(0, nullptr, &p);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        set_debug_object_name(this->_constant_buffer.p, "constant_buffer");

        this->_sphere_constants = static_cast<SphereConstants *>(p);
        p = offset_by_n<SphereConstants,
            D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(
            p, this->pipeline_depth());

        this->_view_constants = static_cast<ViewConstants *>(p);
        p = offset_by_n<ViewConstants,
            D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(
                p, this->pipeline_depth());

        this->_tessellation_constants= static_cast<TessellationConstants *>(p);
        assert(static_cast<void *>(this->_sphere_constants)
            < static_cast<void *>(this->_view_constants));
        assert(static_cast<void *>(this->_view_constants)
            < static_cast<void *>(this->_tessellation_constants));
    }

    // Data are also device-specific.
    this->_data.clear();
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_descriptors
 */
trrojan::d3d12::sphere_benchmark_base::descriptor_table_type
trrojan::d3d12::sphere_benchmark_base::set_descriptors(
        ID3D12Device *device, const shader_id_type shader_code,
        const UINT frame) {
    assert(device != nullptr);
    auto heap = this->_descriptor_heaps[frame];
    assert(heap->GetDesc().Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    auto cpu_handle = heap->GetCPUDescriptorHandleForHeapStart();
    auto gpu_handle = heap->GetGPUDescriptorHandleForHeapStart();
    const auto increment = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> tables;

    // Create VS resource descriptors.
    const auto pv_intensity = is_technique(shader_code,
        SPHERE_INPUT_PV_INTENSITY);
    const auto use_instancing = is_technique(shader_code,
        SPHERE_TECHNIQUE_USE_INSTANCING);
    if (pv_intensity || use_instancing) {
        tables.push_back(gpu_handle);

        if (pv_intensity) {
            log::instance().write_line(log_level::debug, "Rendering technique "
                "uses per-vertex-colouring. Setting transfer function ...");
            device->CreateShaderResourceView(this->_colour_map, nullptr,
                cpu_handle);
            cpu_handle.ptr += increment;
            gpu_handle.ptr += increment;
        }

        if (use_instancing) {
            log::instance().write_line(log_level::debug, "Rendering technique "
                "uses instancing. Setting structured buffer view ...");
            this->create_buffer_resource_view(this->_data.data(), 0,
                this->_data.spheres(), this->_data.stride(), cpu_handle);
            cpu_handle.ptr += increment;
            gpu_handle.ptr += increment;
        }
    }

    // Create PS resource descriptors.
    if (is_any_technique(shader_code, SPHERE_INPUT_PP_INTENSITY)) {
        log::instance().write_line(log_level::debug, "Rendering technique uses "
            "per-pixel colouring. Setting transfer function as t0.");
        tables.push_back(gpu_handle);

        device->CreateShaderResourceView(this->_colour_map, nullptr,
            cpu_handle);

        cpu_handle.ptr += increment;
        gpu_handle.ptr += increment;
    }

    // Bind the constant buffers. There are always the same independent from the
    // rendering technique used.
    {
        log::instance().write_line(log_level::debug, "Setting constant "
            "buffers.");
        tables.push_back(gpu_handle);

        auto sphere_constants = cpu_handle;
        cpu_handle.ptr += increment;
        gpu_handle.ptr += increment;

        auto view_constants = cpu_handle;
        cpu_handle.ptr += increment;
        gpu_handle.ptr += increment;

        auto tess_constants = cpu_handle;
        cpu_handle.ptr += increment;
        gpu_handle.ptr += increment;
        assert(sphere_constants.ptr < view_constants.ptr);
        assert(view_constants.ptr < tess_constants.ptr);

        this->create_constant_buffer_view(device, frame, sphere_constants,
            view_constants, tess_constants);

        //cmd_list->SetGraphicsRootConstantBufferView(0,
        //    this->get_sphere_constants(frame));
        //cmd_list->SetGraphicsRootConstantBufferView(1,
        //    this->get_tessellation_constants(frame));
        //cmd_list->SetGraphicsRootConstantBufferView(2,
        //    this->get_view_constants(frame));
    }

    return std::make_pair(heap, tables);
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_vertex_buffer
 */
void trrojan::d3d12::sphere_benchmark_base::set_vertex_buffer(
        ID3D12GraphicsCommandList *cmd_list, const shader_id_type shader_code) {
    assert(cmd_list != nullptr);
    if (!is_technique(shader_code, SPHERE_TECHNIQUE_USE_SRV)) {
        D3D12_VERTEX_BUFFER_VIEW desc;
        desc.BufferLocation = this->_data.data()->GetGPUVirtualAddress();
        desc.SizeInBytes = this->_data.data()->GetDesc().Width;
        desc.StrideInBytes = this->_data.stride();

        log::instance().write_line(log_level::debug, "Rendering technique uses "
            "vertex buffer. Setting 0x{0:x} ...", desc.BufferLocation);
        cmd_list->IASetVertexBuffers(0, 1, &desc);
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::update_constants
 */
void trrojan::d3d12::sphere_benchmark_base::update_constants(
        const sphere_rendering_configuration& config, const UINT buffer) {
    assert(buffer < this->pipeline_depth());
    this->_data.get_sphere_constants(*index_constant_buffer(
        this->_sphere_constants, buffer));
    config.get_tessellation_constants(*index_constant_buffer(
        this->_tessellation_constants, buffer));
    this->get_view_constants(*index_constant_buffer(
        this->_view_constants, buffer));
}
