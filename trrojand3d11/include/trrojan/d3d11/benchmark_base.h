/// <copyright file="benchmark_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <vector>

#ifdef _UWP
#include <winrt/windows.ui.core.h>
#endif

#include <Windows.h>
#include <d3d11.h>

#include "trrojan/graphics_benchmark_base.h"

#include "trrojan/d3d11/device.h"
#include "trrojan/d3d11/gpu_timer.h"
#include "trrojan/d3d11/render_target.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Base class for D3D11 benchmarks.
    /// </summary>
    class TRROJAND3D11_API benchmark_base 
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

        virtual ~benchmark_base(void);

        virtual bool can_run(trrojan::environment env,
            trrojan::device device) const noexcept;

        virtual trrojan::result run(const configuration& c);

        /// <summary>
        /// Interface to existing UWP window
        /// </summary>
        /// <param name="window"></param>
        void SetWindow(winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> const& window);

    protected:

        benchmark_base(const std::string& name);

        inline void clear_target(void) {
            assert(this->render_target != nullptr);
            this->render_target->clear();
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
        virtual trrojan::result on_run(d3d11::device& device,
            const configuration& config,
            const std::vector<std::string>& changed) = 0;

        inline void present_target(void) {
            assert(this->render_target != nullptr);
            this->render_target->present();
        }

        void save_target(const char *path = nullptr);

        /// <summary>
        /// Disables the render target view and returns an UAV for its back
        /// buffer instead.
        /// </summary>
        virtual ATL::CComPtr<ID3D11UnorderedAccessView> switch_to_uav_target(
            void);

    //private:
    protected:

        typedef trrojan::benchmark_base base;

        std::shared_ptr<trrojan::d3d11::device> debug_device;
        render_target debug_target;
        render_target render_target;

#ifdef _UWP
        // Cached reference to the Window.
        winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> m_window{ nullptr };
#endif // _UWP

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
