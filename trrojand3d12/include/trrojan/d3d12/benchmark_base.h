// <copyright file="benchmark_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <Windows.h>
#include <d3d12.h>

#include "trrojan/graphics_benchmark_base.h"

#include "trrojan/d3d12/device.h"
#include "trrojan/d3d12/gpu_timer.h"
#include "trrojan/d3d12/render_target.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Base class for D3D12 benchmarks.
    /// </summary>
    class TRROJAND3D12_API benchmark_base
            : public trrojan::graphics_benchmark_base {

    public:

        typedef trrojan::graphics_benchmark_base::manoeuvre_step_type
            manoeuvre_step_type;

        typedef trrojan::graphics_benchmark_base::manoeuvre_type manoeuvre_type;

        typedef trrojan::graphics_benchmark_base::point_type point_type;

        typedef trrojan::graphics_benchmark_base::viewport_type viewport_type;

        /// <summary>
        /// Boolean factor enabling the debug view.
        /// </summary>
        static const std::string factor_debug_view;

        /// <summary>
        /// Boolean factor enabling output of the frame buffer to disk.
        /// </summary>
        static const std::string factor_save_view;

        virtual ~benchmark_base(void);

        virtual bool can_run(trrojan::environment env,
            trrojan::device device) const noexcept override;

        virtual trrojan::result run(const configuration& c) override;

    protected:

        benchmark_base(const std::string& name);

        inline void clear_target(ID3D12GraphicsCommandList *cmd_list) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            this->_render_target->clear(cmd_list);
        }

        /// <summary>
        /// Performs the actual test on behalf of the <see cref="run" /> method.
        /// </summary>
        /// <param name="device">The device to use. It is guaranteed that the
        /// device is obtained from <paramref name="config" />.</param>
        /// <param name="config">The configuration to run.</param>
        /// <param name="changed">The names of the factors that have been
        /// changed since the last test run.</param>
        /// <returns>The test results.</returns>
        virtual trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            const std::vector<std::string>& changed) = 0;

        inline void present_target(ID3D12GraphicsCommandList *cmd_list) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            this->_render_target->present(cmd_list);
        }

        void save_target(const char *path = nullptr);

#if TODO
        /// <summary>
        /// Disables the render target view and returns an UAV for its back
        /// buffer instead.
        /// </summary>
        virtual ATL::CComPtr<ID3D11UnorderedAccessView> switch_to_uav_target(
            void);
#endif

    private:

        typedef trrojan::benchmark_base base;

        ATL::CComPtr<ID3D12GraphicsCommandList> _cmd_list_transition;
        std::shared_ptr<trrojan::d3d12::device> _debug_device;
        render_target _debug_target;
        render_target _render_target;

    };

} /* end namespace d3d12 */
} /* end namespace trrojan */
