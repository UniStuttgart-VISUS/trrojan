// <copyright file="ram_streaming_sphere_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/ram_streaming_sphere_benchmark.h"

#include "trrojan/d3d12/measurement_context.h"


/*
 * ...::ram_streaming_sphere_benchmark::ram_streaming_sphere_benchmark
 */
trrojan::d3d12::ram_streaming_sphere_benchmark::ram_streaming_sphere_benchmark(
        void) : sphere_benchmark_base("ram-stream-sphere-renderer") {
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_streaming_context::factor_batch_count, 8u));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_streaming_context::factor_batch_size, 1024u));
}


/*
 * trrojan::d3d12::ram_streaming_sphere_benchmark::count_descriptor_tables
 */
UINT trrojan::d3d12::ram_streaming_sphere_benchmark::count_descriptor_tables(
        const shader_id_type shader_code, const bool include_root) const {
    // Let the base class compute how many descriptors we need for emitting one
    // draw call.
    auto retval = sphere_benchmark_base::count_descriptor_tables(shader_code,
        false);

    // All of the above is required for each batch that can run in parallel.
    retval *= static_cast<UINT>(this->_stream.batch_count());

    if (include_root) {
        ++retval;
    }

    return retval;
}


/*
 * trrojan::d3d12::ram_streaming_sphere_benchmark::on_run
 */
trrojan::result trrojan::d3d12::ram_streaming_sphere_benchmark::on_run(
        d3d12::device& device,
        const configuration& config,
        power_collector::pointer& power_collector,
        const std::vector<std::string>& changed) {
    sphere_rendering_configuration cfg(config);
    measurement_context mctx(device, 1, this->pipeline_depth());
    auto shader_code = cfg.shader_id();

    // Clear data that cannot be used any more.
    this->clear_stale_data(changed);

    // Load the data if necessary.
    if (!this->_data) {
        log::instance().write_line(log_level::information, "Loading data set \""
            "{}\" into memory ...", cfg.data_set());
        this->_data.load([this](const UINT64 size) {
            this->_buffer.resize(size);
            return this->_buffer.data();
        }, shader_code, cfg);

        log::instance().write_line(log_level::debug, "Reshaping GPU "
            "stream ...");
        this->_stream.reshape(this->_data);
    }

    // Now that we have the data, update the shader code to match it.
    shader_code = this->_data.adjust_shader_code(shader_code);

    // As we have the data, we can also configure the camera now.
    this->configure_camera(config);

    // If necessary, build the GPU heap for streaming the data.
    if (this->_stream.rebuild_required(changed)) {
        log::instance().write_line(log_level::information, "Building GPU "
            "stream ...");
        this->_stream.rebuild(device.d3d_device(), config,
            this->pipeline_depth());
    }






    // Record a command list for each frame for the CPU measurements.
    std::vector<ATL::CComPtr<ID3D12GraphicsCommandList>> cmd_lists(
        this->pipeline_depth());
    for (UINT i = 0; i < this->pipeline_depth(); ++i) {
        auto cmd_list = cmd_lists[i] = this->create_graphics_command_list(i);
        set_debug_object_name(cmd_list, "CPU command list #{}", i);

        for (std::size_t b = 0, t = this->_stream.total_batches(); b < t; ++b) {

        }

    }


    log::instance().write_line(log_level::debug, "Prewarming ...");
    {
        auto prewarms = (std::max)(1u, cfg.min_prewarms());

        do {
            mctx.cpu_timer.start();
            for (mctx.cpu_iterations = 0; mctx.cpu_iterations < prewarms;
                    ++mctx.cpu_iterations) {
                //auto cmd_list = cmd_lists[this->buffer_index()];
                //device.execute_command_list(cmd_list);
                //this->present_target();
            }
            device.wait_for_gpu();
            prewarms = mctx.check_cpu_iterations(cfg.min_wall_time());
        } while (prewarms > 0);
    }


    return trrojan::result();
}
